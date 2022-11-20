#include "aoneevent.h"
#include "asensorhub.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"
#include "astatisticshub.h"
#include "aphotonstatistics.h"
#include "aphotonsimsettings.h"
//#include "acustomrandomsampling.h"

#include <QDebug>
#include <QTextStream>

#include "TMath.h"
#include "TH1D.h"

AOneEvent::AOneEvent() :
    SimSet(APhotonSimHub::getInstance().Settings),
    SensorHub(ASensorHub::getConstInstance()),
    RandomHub(ARandomHub::getInstance()),
    SimStat(AStatisticsHub::getInstance().SimStat)
{}

void AOneEvent::init()
{
    numPMs = SensorHub.countSensors();

    PMhits.resize(numPMs);
    //PMsignals.resize(numPMs);
    SiPMpixels.resize(numPMs);

    for (int iSensor = 0; iSensor < numPMs; iSensor++)
    {
        const ASensorModel * model = SensorHub.sensorModel(iSensor);
        if (model->SiPM)
            SiPMpixels[iSensor] = QBitArray(model->PixelsX * model->PixelsY);
        else
            SiPMpixels[iSensor] = QBitArray();
    }

    clearHits(); //clears and resizes the hits / signals containers
}

void AOneEvent::clearHits()
{
    for (int ipm = 0; ipm < numPMs; ipm++)
    {
        PMhits[ipm] = 0;
        //PMsignals[ipm] = 0;

        SiPMpixels[ipm].fill(false);
    }
}

bool AOneEvent::checkSensorHit(int ipm, double time, int iWave, double x, double y, double angle, int numTransitions, double rnd)
{
    const ASensorModel * model = SensorHub.sensorModelFast(ipm); // already checked
    double detectionProb = model->getPDE(iWave);
    detectionProb *= model->getAngularFactor(angle); // angle is undefined if model has no angular sensitivity data (then always returns 1.0)
    detectionProb *= model->getAreaFactor(x, y);
    if (rnd > detectionProb) return false; //random number is provided by the tracker (accelerator mechanics!)

    if (!model->SiPM)
    {
        PMhits[ipm] += 1.0f;
    }
    else
    {
        size_t binX, binY;
        bool isPixelHit = model->getPixelHit(x, y, binX, binY);
        if (!isPixelHit) return false;

        /*
            if (PMs->isDoMCcrosstalk() && PMs->at(ipm).MCmodel==0)
            {
                int num = PMs->at(ipm).MCsampl->sample(RandGen) + 1;
                registerSiPMhit(ipm, binX, binY, num);
            }
            else registerSiPMhit(ipm, binX, binY);
        */
        const int iXY = model->PixelsX * binY + binX;
        if (SiPMpixels[ipm].testBit(iXY)) return false; // this pixel is already lit

        //registering hit
        SiPMpixels[ipm].setBit(iXY, true);
        PMhits[ipm] += 1.0f;
    }

    if (SimSet.RunSet.SaveStatistics) fillDetectionStatistics(iWave, time, angle, numTransitions);
    return true;
}

void AOneEvent::registerSiPMhit(int ipm, int binX, int binY, float numHits)
//numHits != 1 is used 1) for the simplistic model of microcell cross-talk -> then MCmodel = 0
//                     2) to simulate dark counts in advanced model (MCmodel = 1)
{
    const ASensorModel * model = SensorHub.sensorModelFast(ipm); // safe, already was checked
    const int iXY = model->PixelsX * binY + binX;
    if (SiPMpixels[ipm].testBit(iXY)) return;                    // this pixel is already lit

    //registering hit
    SiPMpixels[ipm].setBit(iXY, true);
    PMhits[ipm] += numHits;

    /*
    if (PMs->isDoMCcrosstalk()  &&  PMs->at(ipm).MCmodel == 1)
    {
        //checking 4 neighbours
        const double& trigProb = PMs->at(ipm).MCtriggerProb;
        if (binX > 0             && RandGen->Rndm() < trigProb) registerSiPMhit(ipm, iTime, binX-1, binY, numHits);//left
        if (binX+1 < tp->PixelsX && RandGen->Rndm() < trigProb) registerSiPMhit(ipm, iTime, binX+1, binY, numHits);//right
        if (binY > 0             && RandGen->Rndm() < trigProb) registerSiPMhit(ipm, iTime, binX, binY-1, numHits);//bottom
        if (binY+1 < tp->PixelsY && RandGen->Rndm() < trigProb) registerSiPMhit(ipm, iTime, binX, binY+1, numHits);//top
    }
    */
}

bool AOneEvent::isHitsEmpty() const
{
    for (int ipm = 0; ipm < numPMs; ipm++)
        if (PMhits[ipm] != 0) return false;  // set to exact zero on init, any non-zero is fine
    return true;
}

void AOneEvent::convertHitsToSignals()
{
    for (int ipm = 0; ipm < numPMs; ipm++)
    {
        const ASensorModel * model = SensorHub.sensorModelFast(ipm);

        if (model->ElectronicNoiseSigma > 0)
        {
            PMhits[ipm] += RandomHub.gauss(0, model->ElectronicNoiseSigma);

            /*
            const double& sigma_const = pm.ElNoiseSigma;
            const double& sigma_stat  = pm.ElNoiseSigma_StatSigma;
            const double& sigma_norm  = pm.ElNoiseSigma_StatNorm;

            const double sigma = (sigma_stat == 0 ? sigma_const : sqrt( sigma_const*sigma_const  +  sigma_stat*sigma_stat * pmSignals.at(ipm) / sigma_norm) );
            PMhits[ipm] += RandGen->Gaus(0, model->ElectronicNoiseSigma);
            */
        }

        PMhits[ipm] = model->convertHitsToSignal(PMhits[ipm]);
    }

    /*
    for (int ipm = 0; ipm < numPMs; ipm++)
    {
        const APm & pm = PMs->at(ipm);

        // ADC simulation
        if (PMs->isDoADC())
        {
            if (pmSignals[ipm] < 0) pmSignals[ipm] = 0;
            else
            {
                if (pmSignals[ipm] > pm.ADCmax) pmSignals[ipm] = pm.ADClevels;
                else pmSignals[ipm] = static_cast<int>( pmSignals.at(ipm) / PMs->at(ipm).ADCstep );
            }
        }
    }
    */
}

void AOneEvent::addDarkCounts() //currently applicable only for SiPMs!
{
/*
    for (int ipm = 0; ipm < numPMs; ipm++)
        if (PMs->isSiPM(ipm))
        {
            const APmType* typ = PMs->getTypeForPM(ipm);
            const int&    pixelsX =  typ->PixelsX;
            const int&    pixelsY =  typ->PixelsY;
            const double& darkRate = typ->DarkCountRate; //in Hz
            //   qDebug() << "SiPM dark rate:" << darkRate << "Hz";

            const int     iTimeBins = SimSet->fTimeResolved ? SimSet->TimeBins : 1;
            const double  TimeInterval = ( iTimeBins == 1 ? PMs->at(ipm).MeasurementTime : (SimSet->TimeTo - SimSet->TimeFrom)/SimSet->TimeBins );
            //   qDebug() << "Time interval:" << TimeInterval << "ns";

            const double  averageDarkCounts = darkRate * TimeInterval * 1.0e-9;
            //   qDebug() << "Average dark counts per time bin:" << averageDarkCounts;

            const double pixelFiringProbability = averageDarkCounts / pixelsX / pixelsY;
            //   qDebug() << "Firing probability of each pixel per time bin:" << pixelFiringProbability;

            if (pixelFiringProbability < 0.05) //if it is less than 5% assuming there will be no overlap in triggered pixels
            {
                //quick procedure
                for (int iTime = 0; iTime < iTimeBins; iTime++)
                {
                    int DarkCounts = RandGen->Poisson(averageDarkCounts);
                    //    qDebug() << "Actual dark counts" << DarkCounts;
                    for (int iev = 0; iev < DarkCounts; iev++)
                    {
                        int iX = pixelsX * RandGen->Rndm();
                        if (iX >= pixelsX) iX = pixelsX-1;//protection
                        int iY = pixelsY * RandGen->Rndm();
                        if (iY >= pixelsY) iY = pixelsY-1;
                        //   qDebug()<<"Pixels:"<<iX<<iY;

                        registerSiPMhit(ipm, iTime, iX, iY, generateDarkHitIncrement(ipm));
                    }
                }
            }
            else
            {
                //slow but accurate procedure
                for (int iTime = 0; iTime<iTimeBins; iTime++)
                {
                    for (int iX = 0; iX<pixelsX; iX++)
                        for (int iY = 0; iY<pixelsY; iY++)
                        {
                            if (RandGen->Rndm() < pixelFiringProbability)
                                registerSiPMhit(ipm, iTime, iX, iY, generateDarkHitIncrement(ipm));
                        }
                }
            }
        }
*/
}

float AOneEvent::generateDarkHitIncrement(int ipm) const
{
/*
    if (PMs->at(ipm).DarkCounts_Model == 0 || PMs->at(ipm).DarkCounts_Distribution.isEmpty()) return 1.0f;

    const int index = RandGen->Uniform(PMs->at(ipm).DarkCounts_Distribution.size());
    return PMs->at(ipm).DarkCounts_Distribution.at(index);
*/
}

void AOneEvent::fillDetectionStatistics(int WaveIndex, double time, double angle, int Transitions)
{
    SimStat.registerWave(WaveIndex);
    SimStat.registerTime(time);
    SimStat.registerAngle(angle);
    SimStat.registerNumTrans(Transitions);
}
