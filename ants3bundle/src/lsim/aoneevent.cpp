#include "aoneevent.h"
#include "asensorhub.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"
#include "astatisticshub.h"
#include "aphotonstatistics.h"
#include "aphotonsimsettings.h"

#include <QDebug>

//#include "TMath.h"
//#include "TH1D.h"

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

        bool bDetected = registerSiPMhit(ipm, binX, binY);
        if (!bDetected) return false; // not logging if the pixel was already lit
    }

    if (SimSet.RunSet.SaveStatistics) fillDetectionStatistics(iWave, time, angle, numTransitions);
    return true;
}

bool AOneEvent::registerSiPMhit(int ipm, size_t binX, size_t binY)
{
    const ASensorModel * model = SensorHub.sensorModelFast(ipm);
    const int index = model->getPixelIndex(binX, binY);
    if (SiPMpixels[ipm].testBit(index)) return false; // this pixel is already lit

    //registering hit
    SiPMpixels[ipm].setBit(index, true);
    PMhits[ipm] += 1.0f;
    return true;
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
        PMhits[ipm] = model->convertHitsToSignal(PMhits[ipm]);
    }
}

void AOneEvent::addDarkCounts()
{
    for (int ipm = 0; ipm < numPMs; ipm++)
    {
        const ASensorModel * model = SensorHub.sensorModelFast(ipm);

        if (!model->SiPM)
            PMhits[ipm] += RandomHub.poisson(model->_AverageDarkCounts);
        else
        {
            if (model->_PixelDarkFiringProbability < 0.10) //if it is less than 10% assuming there will be no overlap in triggered pixels
            {
                //quick procedure
                int DarkCounts = RandomHub.poisson(model->_AverageDarkCounts);
                for (int iev = 0; iev < DarkCounts; iev++)
                {
                    size_t iX = model->PixelsX * RandomHub.uniform();
                    size_t iY = model->PixelsY * RandomHub.uniform();

                    if (iX >= model->PixelsX) iX = model->PixelsX - 1;
                    if (iY >= model->PixelsY) iY = model->PixelsY - 1;

                    registerSiPMhit(ipm, iX, iY);
                }
            }
            else
            {
                //slow but accurate procedure
                for (size_t iX = 0; iX < model->PixelsX; iX++)
                    for (size_t iY = 0; iY < model->PixelsY; iY++)
                    {
                        if (RandomHub.uniform() < model->_PixelDarkFiringProbability)
                            registerSiPMhit(ipm, iX, iY);
                    }
            }
        }
    }
}

void AOneEvent::fillDetectionStatistics(int waveIndex, double time, double angle, int numTransitions)
{
    SimStat.registerWave(waveIndex);
    SimStat.registerTime(time);
    SimStat.registerAngle(angle);
    SimStat.registerNumTrans(numTransitions);
}
