#include "apartanalysis_si.h"
#include "aeventtrackingrecord.h"
#include "atrackinghistorycrawler.h"

#include <QDebug>
#include <QFile>

#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"

APartAnalysis_SI::APartAnalysis_SI() :
    AScriptInterface()
{
    Criteria = new AFindRecordSelector();
}

APartAnalysis_SI::~APartAnalysis_SI()
{
    delete Criteria;
    delete Crawler;
}

void APartAnalysis_SI::configure(QString fileName, bool binary, int numThreads)
{
    FileName    = fileName;
    bBinaryFile = binary;
    NumThreads  = numThreads;
}

void APartAnalysis_SI::clearCriteria()
{
    delete Criteria; Criteria = new AFindRecordSelector();
}

void APartAnalysis_SI::setParticle(QString particleName)
{
    Criteria->bParticle = true;
    Criteria->Particle = particleName;
}

void APartAnalysis_SI::setOnlyPrimary()
{
    Criteria->bPrimary = true;
}

void APartAnalysis_SI::setOnlySecondary()
{
    Criteria->bSecondary = true;
}

void APartAnalysis_SI::setLimitToFirstInteractionOfPrimary()
{
    Criteria->bLimitToFirstInteractionOfPrimary = true;
}

void APartAnalysis_SI::setMaterial(int matIndex)
{
    Criteria->bMaterial = true;
    Criteria->Material = matIndex;
}

void APartAnalysis_SI::setVolume(QString volumeName)
{
    Criteria->bVolume = true;
    Criteria->Volume = volumeName.toLocal8Bit().data();
}

void APartAnalysis_SI::setVolumeIndex(int volumeIndex)
{
    Criteria->bVolumeIndex = true;
    Criteria->VolumeIndex = volumeIndex;
}

void APartAnalysis_SI::setFromMaterial(int matIndex)
{
    Criteria->bFromMat = true;
    Criteria->FromMat = matIndex;
}

void APartAnalysis_SI::setToMaterial(int matIndex)
{
    Criteria->bToMat = true;
    Criteria->ToMat = matIndex;
}

void APartAnalysis_SI::setFromVolume(QString volumeName)
{
    Criteria->bFromVolume = true;
    Criteria->FromVolume = volumeName.toLocal8Bit().data();
}

void APartAnalysis_SI::setToVolume(QString volumeName)
{
    Criteria->bToVolume = true;
    Criteria->ToVolume = volumeName.toLocal8Bit().data();
}

void APartAnalysis_SI::setFromIndex(int volumeIndex)
{
    Criteria->bFromVolIndex = true;
    Criteria->FromVolIndex = volumeIndex;
}

void APartAnalysis_SI::setToIndex(int volumeIndex)
{
    Criteria->bToVolIndex = true;
    Criteria->ToVolIndex = volumeIndex;
}

void APartAnalysis_SI::setOnlyCreated()
{
    Criteria->bCreated = true;
    Criteria->bEscaping = false;
}

void APartAnalysis_SI::setOnlyEscaping()
{
    Criteria->bEscaping = true;
    Criteria->bCreated = false;
}

#include "athreadpool.h"
#include "arandomhub.h"
void APartAnalysis_SI::test(int numThreads)
{
    AThreadPool pool(numThreads);

    for(int i = 0; i < 8; ++i)
    {
        //qDebug() << "preparing job #" << i;
        while (pool.isFull()) {std::this_thread::sleep_for(std::chrono::microseconds(10));}
        qDebug() << "Pooling job #" << i;

        pool.addJob([i]()
        {
            qDebug() << "  -->" << i;
            std::this_thread::sleep_for(std::chrono::seconds(2) + ARandomHub::getInstance().uniform()*std::chrono::seconds(3));
            qDebug() << "     <--" << i;
        });
    }

    qDebug() << "Waiting for jobs to finish...";
    while (!pool.isIdle()) {std::this_thread::sleep_for(std::chrono::microseconds(1000));}

    qDebug() << "Done!";
}

// ---

bool APartAnalysis_SI::initCrawler()
{
    if (FileName.isEmpty())
    {
        abort("File name with tracking history not configured. Use configure(name, binary) method");
        return false;
    }

    if (!QFile::exists(FileName))
    {
        abort("File with tracking history does not exist:\n" + FileName);
        return false;
    }

    delete Crawler;
    Crawler = new ATrackingHistoryCrawler(FileName, bBinaryFile);
    return true;
}

QVariantList APartAnalysis_SI::findParticles()
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_findParticles p;
    Crawler->find(*Criteria, p, NumThreads);

    for (const auto & pair : p.FoundParticles)
    {
        QVariantList el;
        el << pair.first << pair.second;
        vl.push_back(el);
    }

    return vl;
}

QVariantList APartAnalysis_SI::findProcesses(int All0_WithDepo1_TrackEnd2, bool onlyHadronic, QString targetIsotopeStartsFrom)
{
    QVariantList vl;

    if (All0_WithDepo1_TrackEnd2 < 0 || All0_WithDepo1_TrackEnd2 > 2)
    {
        abort("Bad selector for findProcess method");
        return vl;
    }

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_findProcesses::SelectionMode mode = static_cast<AHistorySearchProcessor_findProcesses::SelectionMode>(All0_WithDepo1_TrackEnd2);

    AHistorySearchProcessor_findProcesses p(mode, onlyHadronic, targetIsotopeStartsFrom);
    Crawler->find(*Criteria, p, NumThreads);

    for (const auto & pair : p.FoundProcesses)
    {
        QVariantList el;
        el << pair.first << pair.second;
        vl.push_back(el);
    }
    return vl;
}

QVariantList APartAnalysis_SI::findDepE(AHistorySearchProcessor_findDepositedEnergy & p)
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    Crawler->find(*Criteria, p, NumThreads);

    int numBins = p.Hist->GetXaxis()->GetNbins();
    for (int iBin=1; iBin<numBins+1; iBin++)
    {
        QVariantList el;
        el << p.Hist->GetBinCenter(iBin) << p.Hist->GetBinContent(iBin);
        vl.push_back(el);
    }
    return vl;
}

QVariantList APartAnalysis_SI::findDepositedEnergies(int bins, double from, double to)
{
    AHistorySearchProcessor_findDepositedEnergy p(AHistorySearchProcessor_findDepositedEnergy::Individual, bins, from, to);
    return findDepE(p);
}

QVariantList APartAnalysis_SI::findDepositedEnergiesWithSecondaries(int bins, double from, double to)
{
    AHistorySearchProcessor_findDepositedEnergy p(AHistorySearchProcessor_findDepositedEnergy::WithSecondaries, bins, from, to);
    return findDepE(p);
}

QVariantList APartAnalysis_SI::findDepositedEnergiesOverEvent(int bins, double from, double to)
{
    AHistorySearchProcessor_findDepositedEnergy p(AHistorySearchProcessor_findDepositedEnergy::OverEvent, bins, from, to);
    return findDepE(p);
}

QVariantList APartAnalysis_SI::findDepositedEnergyStats()
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_getDepositionStats p;
    Crawler->find(*Criteria, p, NumThreads);

    QMap<QString, AParticleDepoStat>::const_iterator it = p.DepoData.constBegin();
    while (it != p.DepoData.constEnd())
    {
        QVariantList el;
        const AParticleDepoStat & rec = it.value();
        const double mean = rec.sum / rec.num;
        const double sigma = sqrt( (rec.sumOfSquares - 2.0*mean*rec.sum)/rec.num + mean*mean );
        el << it.key() << rec.num << mean << sigma;
        vl.push_back(el);
        ++it;
    }
    return vl;
}

QVariantList APartAnalysis_SI::findDepositedEnergyStats(double timeFrom, double timeTo)
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_getDepositionStatsTimeAware p(timeFrom, timeTo);
    Crawler->find(*Criteria, p, NumThreads);

    QMap<QString, AParticleDepoStat>::const_iterator it = p.DepoData.constBegin();
    while (it != p.DepoData.constEnd())
    {
        QVariantList el;
        const AParticleDepoStat & rec = it.value();
        const double mean = rec.sum / rec.num;
        const double sigma = sqrt( (rec.sumOfSquares - 2.0*mean*rec.sum)/rec.num + mean*mean );
        el << it.key() << rec.num << mean << sigma;
        vl.push_back(el);
        ++it;
    }
    return vl;
}

QVariantList APartAnalysis_SI::findTravelledDistances(int bins, double from, double to)
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_findTravelledDistances p(bins, from, to);
    Crawler->find(*Criteria, p, NumThreads);

    int numBins = p.Hist->GetXaxis()->GetNbins();
    for (int iBin=1; iBin<numBins+1; iBin++)
    {
        QVariantList el;
        el << p.Hist->GetBinCenter(iBin) << p.Hist->GetBinContent(iBin);
        vl.push_back(el);
    }
    return vl;
}

QVariantList APartAnalysis_SI::findhadronicChannels()
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    AHistorySearchProcessor_findHadronicChannels p;
    Crawler->find(*Criteria, p, NumThreads);

    std::vector<std::pair<QString,int>> vec;
    p.getResults(vec);

    for (const auto & p : vec)
    {
        QVariantList el;
        el << p.first << p.second;
        vl.push_back(el);
    }

    return vl;
}

const QVariantList APartAnalysis_SI::findOB_1D(AHistorySearchProcessor_Border & p)
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    Crawler->find(*Criteria, p, NumThreads);

    int numBins = p.Hist1D->GetXaxis()->GetNbins();
    for (int iBin=1; iBin<numBins+1; iBin++)
    {
        QVariantList el;
        el << p.Hist1D->GetBinCenter(iBin) << p.Hist1D->GetBinContent(iBin);
        vl.push_back(el);
    }
    return vl;
}

QVariantList APartAnalysis_SI::findOnBorder(QString what, QString cuts, int bins, double from, double to)
{
    AHistorySearchProcessor_Border p(what, cuts, bins, from, to);
    if (!p.ErrorString.isEmpty())
    {
        abort(p.ErrorString);
        return QVariantList();
    }
    else
        return findOB_1D(p);
}

QVariantList APartAnalysis_SI::findOnBorder(QString what, QString vsWhat, QString cuts, int bins, double from, double to)
{
    AHistorySearchProcessor_Border p(what, vsWhat, cuts, bins, from, to);
    if (!p.ErrorString.isEmpty())
    {
        abort(p.ErrorString);
        return QVariantList();
    }
    else
        return findOB_1D(p);
}

const QVariantList APartAnalysis_SI::findOB_2D(AHistorySearchProcessor_Border & p)
{
    QVariantList vl;

    bool ok = initCrawler();
    if (!ok) return vl;

    Crawler->find(*Criteria, p, NumThreads);

    int numX = p.Hist2D->GetXaxis()->GetNbins();
    int numY = p.Hist2D->GetYaxis()->GetNbins();
    for (int iX=1; iX<numX+1; iX++)
    {
        double x = p.Hist2D->GetXaxis()->GetBinCenter(iX);
        for (int iY=1; iY<numY+1; iY++)
        {
            QVariantList el;
            el << x
               << p.Hist2D->GetYaxis()->GetBinCenter(iY)
               << p.Hist2D->GetBinContent(iX, iY);
            vl.push_back(el);
        }
    }
    return vl;
}

QVariantList APartAnalysis_SI::findOnBorder(QString what, QString vsWhat, QString cuts, int bins1, double from1, double to1, int bins2, double from2, double to2)
{
    AHistorySearchProcessor_Border p(what, vsWhat, cuts, bins1, from1, to1, bins2, from2, to2);
    if (!p.ErrorString.isEmpty())
    {
        abort(p.ErrorString);
        return QVariantList();
    }
    else
        return findOB_2D(p);
}

QVariantList APartAnalysis_SI::findOnBorder(QString what, QString vsWhat, QString andVsWhat, QString cuts, int bins1, double from1, double to1, int bins2, double from2, double to2)
{
    AHistorySearchProcessor_Border p(what, vsWhat, andVsWhat, cuts, bins1, from1, to1, bins2, from2, to2);
    if (!p.ErrorString.isEmpty())
    {
        abort(p.ErrorString);
        return QVariantList();
    }
    else
        return findOB_2D(p);
}
