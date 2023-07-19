#include "atrackinghistorycrawler.h"
#include "atrackingdataimporter.h"
#include "athreadpool.h"
#include "vformula.h"
#include "ath.h"

#include <QDebug>
#include <QApplication>
#include <QTimer>

#include "TGeoNode.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH2.h"
#include "TTree.h"

void ATrackingHistoryCrawler::find(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor, int numThreads, int eventsPerThread)
{
    QTimer Timer(this);
    Timer.setInterval(250);
    connect(&Timer, &QTimer::timeout, this, [this](){emit reportProgress(NumEventsProcessed);});
    Timer.start();

    if (numThreads < 1) findSingleThread(criteria, processor);
    else                findMultithread(criteria, processor, numThreads, eventsPerThread);

    Timer.stop();
}

// !!!*** add error control
void ATrackingHistoryCrawler::findSingleThread(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor)
{
    ATrackingDataImporter imp(FileName);

    processor.beforeSearch();

    AEventTrackingRecord * event = AEventTrackingRecord::create();
    NumEventsProcessed = 0;
    while (imp.extractEvent(NumEventsProcessed, event))
    {
        //qDebug() << "-------------Event #" << iEv;
        processor.onNewEvent();

        const std::vector<AParticleTrackingRecord *> & prim = event->getPrimaryParticleRecords();
        for (const AParticleTrackingRecord * p : prim)
        {
            findRecursive(*p, criteria, processor);
            QApplication::processEvents();
            if (bAbortRequested) break;
        }

        processor.onEventEnd();

        NumEventsProcessed++;
        if (bAbortRequested) break;
    }
    processor.afterSearch();
}

void ATrackingHistoryCrawler::findMultithread(const AFindRecordSelector & criteria, AHistorySearchProcessor & processor, int numThreads, int eventsPerThread)
{
    NumEventsProcessed = 0;

    ATrackingDataImporter dataImporter(FileName);
    AThreadPool pool(numThreads);
    const AHistorySearchProcessor * pProcessorForCloning = processor.clone();

    // possible improvement: set binarity in the constructor optional parameter, so there is no need to check the file

    int iEvent = 0;
    while (true)
    {
        //qDebug() << "-------------Event #" << iEvent;
        bool ok = dataImporter.gotoEvent(iEvent);
        if (!ok) break;

        const AImporterEventStart position = dataImporter.getEventStart();

        //while (pool.isFull()) std::this_thread::sleep_for(std::chrono::microseconds(1)); <- makes it slower

        pool.addJob(
                    [&processor, pProcessorForCloning, &criteria, position, iEvent, eventsPerThread, this]()
                    {
                        ATrackingDataImporter localDataImporter(FileName);
                        localDataImporter.setPositionInFile(position);

                        AEventTrackingRecord * event = AEventTrackingRecord::create();

                        AHistorySearchProcessor * localProcessor = pProcessorForCloning->clone(); // acceptable: they are light-weight

                        localProcessor->beforeSearch();

                        for (int iChunk = 0; iChunk < eventsPerThread; iChunk++)
                        {
                            localDataImporter.extractEvent(iEvent + iChunk, event);

                            localProcessor->onNewEvent();

                            const std::vector<AParticleTrackingRecord *> & prim = event->getPrimaryParticleRecords();
                            for (const AParticleTrackingRecord * p : prim)
                            {
                                findRecursive(*p, criteria, *localProcessor);
                                if (bAbortRequested) break;
                            }

                            if (!bAbortRequested)
                                localProcessor->onEventEnd();
                            else
                                break;
                        }

                        localProcessor->afterSearch();

                        if (!bAbortRequested)
                        {
                            std::lock_guard<std::mutex> lock(CrawlerMutex);
                            processor.mergeResuts(*localProcessor);
                            NumEventsProcessed += eventsPerThread;
                        }

                        delete localProcessor;
                        delete event;
                    } );

        QApplication::processEvents();
        if (bAbortRequested) break;
        iEvent += eventsPerThread;
    }

    while (!pool.isIdle() && !bAbortRequested)
    {
        QApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ATrackingHistoryCrawler::findRecursive(const AParticleTrackingRecord & pr, const AFindRecordSelector & opt, AHistorySearchProcessor & processor) const
{
    bool bDoTrack = true;

    if (!processor.isIgnoreParticleSelectors())
    {
        if      (opt.bParticle  && opt.Particle != pr.ParticleName) bDoTrack = false;
        else if (opt.bPrimary   && pr.getSecondaryOf() ) bDoTrack = false;
        else if (opt.bSecondary && !pr.getSecondaryOf() ) bDoTrack = false;
    }

    if (opt.bTime && bDoTrack)
    {
        const std::vector<ATrackingStepData *> & steps = pr.getSteps();
        if (!steps.empty())
        {
            const double & Time = steps.front()->Time;
            if (Time < opt.TimeFrom || Time > opt.TimeTo) bDoTrack = false;
        }
    }

    bool bInlineTrackingOfSecondaries = processor.isInlineSecondaryProcessing();
    bool bSkipTrackingOfSecondaries = false;
    AParticleTrackingRecord * lastSecondaryToTrack = nullptr;

    if (bDoTrack)
    {
        bool bMaster = processor.onNewTrack(pr);
        bInlineTrackingOfSecondaries = processor.isInlineSecondaryProcessing(); // give a possibility to change the mode

        QString curVolume;
        int     curVolIndex;
        int     curMat;

        const std::vector<ATrackingStepData *> & steps = pr.getSteps();
        for (size_t iStep = 0; iStep < steps.size(); iStep++)
        {
            const ATrackingStepData * thisStep = steps[iStep];

            // different handling of Transportation ("T", "O") and all other processes
            // Creation ("C") is checked as both "Transportation" and all other type process

            ProcessType ProcType;
            if      (thisStep->Process == "C")
            {
                ProcType = Creation;

                const ATransportationStepData * trStep = static_cast<const ATransportationStepData*>(thisStep);
                curVolume = trStep->VolName;
                curVolIndex = trStep->VolIndex;
                curMat = trStep->iMaterial;
            }
            else if (thisStep->Process == "T") ProcType = NormalTransportation;
            else if (thisStep->Process == "O") ProcType = ExitingWorld;
            else                               ProcType = Local;

            if (ProcType == Creation || ProcType == NormalTransportation || ProcType == ExitingWorld)
            {
                // two different checks:
                // 1. for specific transitions from - to
                // 2. for enter / exit of the defined volume/mat/index

                bool bExitValidated;
                if (ProcType == Creation) bExitValidated = false;
                else
                {
                    const bool bCheckingExit = (opt.bFromMat || opt.bFromVolume || opt.bFromVolIndex);
                    if (bCheckingExit)
                    {
                        const bool bRejectedByMaterial = (opt.bFromMat      && opt.FromMat      != curMat);
                        const bool bRejectedByVolName  = (opt.bFromVolume   && opt.FromVolume   != curVolume);
                        const bool bRejectedByVolIndex = (opt.bFromVolIndex && opt.FromVolIndex != curVolIndex);
                        bExitValidated = !(bRejectedByMaterial || bRejectedByVolName || bRejectedByVolIndex);
                    }
                    else bExitValidated = true;
                }

                bool bEntranceValidated;
                const bool bCheckingEnter = (opt.bToMat || opt.bToVolume || opt.bToVolIndex);
                if (thisStep->Process == "O")
                {
                    bEntranceValidated = !bCheckingEnter; // if any check selected -> entrance is not valid
                }
                else if (bCheckingEnter)
                {
                    const ATransportationStepData * trStep = static_cast<const ATransportationStepData*>(thisStep); // "O" should not see this line!
                    {
                        const bool bRejectedByMaterial = (opt.bToMat      && opt.ToMat      != trStep->iMaterial);
                        const bool bRejectedByVolName  = (opt.bToVolume   && opt.ToVolume   != trStep->VolName);
                        const bool bRejectedByVolIndex = (opt.bToVolIndex && opt.ToVolIndex != trStep->VolIndex);
                        bEntranceValidated = !(bRejectedByMaterial || bRejectedByVolName || bRejectedByVolIndex);
                    }
                }
                else bEntranceValidated = true;

                if (opt.bEscaping && ProcType != ExitingWorld) bExitValidated = false;
                if (opt.bCreated  && ProcType != Creation)     bEntranceValidated = false;

                // if transition validated, calling onTransition (+paranoic test on existence of the prevStep - for Creation exit is always not validated
                const ATrackingStepData * prevStep = (iStep == 0 ? nullptr : steps[iStep-1]);
                if (bExitValidated && bEntranceValidated && prevStep)
                    processor.onTransition(*prevStep, *thisStep); // not the "next" step here! this is just to extract direction information

                if (opt.bCreated && ProcType == Creation && bEntranceValidated) //special treatment for creation
                {
                    if (iStep == 0 && steps.size() > 1)
                    {
                        ATrackingStepData prevStep = *thisStep;
                        for (int i=0; i<3; i++)
                            prevStep.Position[i] = 2.0 * thisStep->Position[i] - steps[1]->Position[i];
                        processor.onTransition(prevStep, *thisStep);
                    }
                }

                //checking for specific material/volume/index for enter/exit
                //out
                if (ProcType != Creation)
                {
                    const bool bCheckingExit = (opt.bMaterial || opt.bVolume || opt.bVolumeIndex);
                    if (bCheckingExit)
                    {
                        const bool bRejectedByMaterial = (opt.bMaterial    && opt.Material    != curMat);
                        const bool bRejectedByVolName  = (opt.bVolume      && opt.Volume      != curVolume);
                        const bool bRejectedByVolIndex = (opt.bVolumeIndex && opt.VolumeIndex != curVolIndex);
                        bExitValidated = !(bRejectedByMaterial || bRejectedByVolName || bRejectedByVolIndex);
                    }
                    else bExitValidated = true;

                    if (bExitValidated) processor.onTransitionOut(*thisStep);
                }
                //in
                if (ProcType != ExitingWorld)
                {
                    bool bEntranceValidated;
                    const bool bCheckingEnter = (opt.bMaterial || opt.bVolume || opt.bVolumeIndex);
                    if (bCheckingEnter)
                    {
                        const ATransportationStepData * trStep = static_cast<const ATransportationStepData*>(thisStep); // "O" should not see this line!
                        const bool bRejectedByMaterial = (opt.bMaterial    && opt.Material    != trStep->iMaterial);
                        const bool bRejectedByVolName  = (opt.bVolume      && opt.Volume      != trStep->VolName);
                        const bool bRejectedByVolIndex = (opt.bVolumeIndex && opt.VolumeIndex != trStep->VolIndex);
                        bEntranceValidated = !(bRejectedByMaterial || bRejectedByVolName || bRejectedByVolIndex);
                    }
                    else bEntranceValidated = true;

                    if (bEntranceValidated ) processor.onTransitionIn(*thisStep);
                }

                //now can update current volume info for transition step
                if (thisStep->Process == "T")
                {
                    const ATransportationStepData * trStep = static_cast<const ATransportationStepData*>(thisStep);
                    curVolume = trStep->VolName;
                    curVolIndex = trStep->VolIndex;
                    curMat = trStep->iMaterial;
                }
            }

            // Local step or Creation (Creation is treated again -> this time as it would be a local step)
            if (ProcType == Local || ProcType == Creation)
            {
                bool bSkipThisStep = false;
                if (opt.bMaterial || opt.bVolume || opt.bVolumeIndex)
                {
                         if (opt.bMaterial    && opt.Material    != curMat) bSkipThisStep = true;
                    else if (opt.bVolumeIndex && opt.VolumeIndex != curVolIndex) bSkipThisStep = true;
                    else if (opt.bVolume      && opt.Volume      != curVolume) bSkipThisStep = true;
                }
                if (!bSkipThisStep) processor.onLocalStep(*thisStep);

                if (bInlineTrackingOfSecondaries)
                {
                    for (int iSec : thisStep->Secondaries)
                        findRecursive(*pr.getSecondaries().at(iSec), opt, processor);

                    if (!pr.getSecondaryOf() && opt.bLimitToFirstInteractionOfPrimary && ProcType != Creation)
                        break;
                }
                else
                {
                    if (!pr.getSecondaryOf() && opt.bLimitToFirstInteractionOfPrimary && ProcType != Creation)
                    {
                        if (thisStep->Secondaries.empty()) bSkipTrackingOfSecondaries = true;
                        else lastSecondaryToTrack = pr.getSecondaries().at( thisStep->Secondaries.back() );
                        break;
                    }
                }

            }
        }

        processor.onTrackEnd(bMaster);
    }

    if (!bInlineTrackingOfSecondaries)
    {
        if (!bSkipTrackingOfSecondaries)
        {
            const std::vector<AParticleTrackingRecord *> & secondaries = pr.getSecondaries();
            for (AParticleTrackingRecord * sec : secondaries)
            {
                findRecursive(*sec, opt, processor);
                if (sec == lastSecondaryToTrack) break;
            }
        }
    }
}

bool AHistorySearchProcessor_findParticles::onNewTrack(const AParticleTrackingRecord &pr)
{
    Candidate = pr.ParticleName;
    bConfirmed = false;
    return false;
}

void AHistorySearchProcessor_findParticles::onTransitionIn(const ATrackingStepData & )
{
    bConfirmed = true;
}

void AHistorySearchProcessor_findParticles::onLocalStep(const ATrackingStepData & )
{
    bConfirmed = true;
}

void AHistorySearchProcessor_findParticles::onTrackEnd(bool)
{
    if (bConfirmed && !Candidate.isEmpty())
    {
        auto it = FoundParticles.find(Candidate);
        if (it == FoundParticles.end())
            FoundParticles[Candidate] = 1;
        else ++(it->second);
        Candidate.clear();
    }
}

AHistorySearchProcessor * AHistorySearchProcessor_findParticles::clone() const
{
    return new AHistorySearchProcessor_findParticles(*this);
}

bool AHistorySearchProcessor_findParticles::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findParticles * from = dynamic_cast<const AHistorySearchProcessor_findParticles*>(&other);
    if (!from) return false;

    for (const auto & itOther : from->FoundParticles)
    {
        auto itHere = FoundParticles.find(itOther.first);
        if (itHere == FoundParticles.end())
            FoundParticles[itOther.first] = itOther.second;
        else
            itHere->second += itOther.second;
    }
    return true;
}

void AHistorySearchProcessor_findParticles::getResults(std::vector<std::pair<QString, int> > & data) const
{
    data.clear();
    data.reserve(FoundParticles.size());

    for (const auto & pair : FoundParticles)
        data.push_back( {pair.first, pair.second} );

    std::sort(data.begin(), data.end(),
              [](const std::pair<QString,int> & lhs, const std::pair<QString,int> & rhs)
                {return lhs.second > rhs.second;}
             );
}

AHistorySearchProcessor_findDepositedEnergy::AHistorySearchProcessor_findDepositedEnergy(CollectionMode mode, int bins, double from, double to)
{
    Mode = mode;
    Hist = new TH1D("", "Deposited energy", bins, from, to);
    Hist->GetXaxis()->SetTitle("Energy, keV");

    bInlineSecondaryProcessing = false; // on start it is default non-inline mode even in WithSecondaries mode
}

AHistorySearchProcessor_findDepositedEnergy::~AHistorySearchProcessor_findDepositedEnergy()
{
    delete Hist;
}

void AHistorySearchProcessor_findDepositedEnergy::onNewEvent()
{
    if (Mode == OverEvent) clearData();
}

bool AHistorySearchProcessor_findDepositedEnergy::onNewTrack(const AParticleTrackingRecord & )
{
    switch (Mode)
    {
    case Individual:
        clearData();
        return false;
    case WithSecondaries:
        if (bSecondaryTrackingStarted)
            return false;
        else
        {
            clearData();
            bSecondaryTrackingStarted  = true;
            bInlineSecondaryProcessing = true; // change to inline secondaries mode
            bIgnoreParticleSelectors   = true; // to allow secondaries even not allowed by the selection
            return true;
        }
    case OverEvent:
        // no need to do anything - Depo is reset on new event
        return false;
    }
    return false;
}

void AHistorySearchProcessor_findDepositedEnergy::onLocalStep(const ATrackingStepData & tr)
{
    fillDeposition(tr);
}

void AHistorySearchProcessor_findDepositedEnergy::onTransitionOut(const ATrackingStepData & tr)
{
    fillDeposition(tr);
}

void AHistorySearchProcessor_findDepositedEnergy::onTrackEnd(bool bMaster)
{
    switch (Mode)
    {
    case Individual:
        fillHistogram();
        break;
    case WithSecondaries:
        if (bMaster)
        {
            fillHistogram();
            bSecondaryTrackingStarted  = false;
            bInlineSecondaryProcessing = false; // back to default non-inline mode!
            bIgnoreParticleSelectors   = false; // back to normal mode
        }
        break;
    case OverEvent:
        // nothing to do - waiting for the end of the event
        break;
    }
}

void AHistorySearchProcessor_findDepositedEnergy::onEventEnd()
{
    if (Mode == OverEvent) fillHistogram();
}

AHistorySearchProcessor * AHistorySearchProcessor_findDepositedEnergy::clone() const
{
    AHistorySearchProcessor_findDepositedEnergy * pr = new AHistorySearchProcessor_findDepositedEnergy(*this);

    pr->Hist = (TH1D*)Hist->Clone();

    return pr;
}

bool AHistorySearchProcessor_findDepositedEnergy::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findDepositedEnergy * from = dynamic_cast<const AHistorySearchProcessor_findDepositedEnergy*>(&other);
    if (!from) return false;

    ATH1D::merge(Hist, from->Hist);
    return true;
}

void AHistorySearchProcessor_findDepositedEnergy::clearData()
{
    Depo = 0;
}

void AHistorySearchProcessor_findDepositedEnergy::fillDeposition(const ATrackingStepData &tr)
{
    Depo += tr.DepositedEnergy;
}

void AHistorySearchProcessor_findDepositedEnergy::fillHistogram()
{
    if (Depo > 0) Hist->Fill(Depo);
    clearData();
}


AHistorySearchProcessor_findDepositedEnergyTimed::AHistorySearchProcessor_findDepositedEnergyTimed(AHistorySearchProcessor_findDepositedEnergy::CollectionMode mode,
                                                                                                   int binsE, double fromE, double toE,
                                                                                                   int binsT, double fromT, double toT)
{
    Mode = mode;
    Hist2D = new TH2D("", "Deposited energy vs weighted time", binsE, fromE, toE,  binsT, fromT, toT);
    Hist2D->GetXaxis()->SetTitle("Deposited energy, keV");
    Hist2D->GetYaxis()->SetTitle("Mean deposition time (weighted by energy), ns");

    bInlineSecondaryProcessing = false; // on start it is default non-inline mode even in WithSecondaries mode
}

AHistorySearchProcessor_findDepositedEnergyTimed::~AHistorySearchProcessor_findDepositedEnergyTimed()
{
    delete Hist2D;
}

AHistorySearchProcessor * AHistorySearchProcessor_findDepositedEnergyTimed::clone() const
{
    AHistorySearchProcessor_findDepositedEnergyTimed * pr = new AHistorySearchProcessor_findDepositedEnergyTimed(*this);

    pr->Hist2D = (TH2D*)Hist2D->Clone();

    return pr;
}

bool AHistorySearchProcessor_findDepositedEnergyTimed::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findDepositedEnergyTimed * from = dynamic_cast<const AHistorySearchProcessor_findDepositedEnergyTimed*>(&other);
    if (!from) return false;

    ATH2D::merge(Hist2D, from->Hist2D);
    return true;
}

void AHistorySearchProcessor_findDepositedEnergyTimed::clearData()
{
    Depo = 0;
    Time = 0;
}

void AHistorySearchProcessor_findDepositedEnergyTimed::fillDeposition(const ATrackingStepData &tr)
{
    Depo += tr.DepositedEnergy;
    Time += tr.Time * tr.DepositedEnergy;
}

void AHistorySearchProcessor_findDepositedEnergyTimed::fillHistogram()
{
    if (Depo > 0)
    {
        Time /= Depo;
        Hist2D->Fill(Depo, Time);
    }
    clearData();
}



AHistorySearchProcessor_findTravelledDistances::AHistorySearchProcessor_findTravelledDistances(int bins, double from, double to)
{
    Hist = new TH1D("", "Travelled distance", bins, from, to);
    Hist->GetXaxis()->SetTitle("Distance, mm");
}

AHistorySearchProcessor_findTravelledDistances::~AHistorySearchProcessor_findTravelledDistances()
{
    delete Hist;
}

bool AHistorySearchProcessor_findTravelledDistances::onNewTrack(const AParticleTrackingRecord &)
{
    Distance = 0;
    return false;
}

void AHistorySearchProcessor_findTravelledDistances::onLocalStep(const ATrackingStepData &tr)
{
    if (bStarted)
    {
        double d2 = 0;
        for (int i=0; i<3; i++)
        {
            const double delta = LastPosition[i] - tr.Position[i];
            d2 += delta * delta;
            LastPosition[i] = tr.Position[i];
        }
        Distance += sqrt(d2);
    }
}

void AHistorySearchProcessor_findTravelledDistances::onTransitionOut(const ATrackingStepData &tr)
{
    bStarted = false;
    double d2 = 0;
    for (int i=0; i<3; i++)
    {
        const double delta = LastPosition[i] - tr.Position[i];
        d2 += delta * delta;
    }
    Distance += sqrt(d2);
}

void AHistorySearchProcessor_findTravelledDistances::onTransitionIn(const ATrackingStepData &tr)
{
    bStarted = true;
    for (int i=0; i<3; i++)
        LastPosition[i] = tr.Position[i];
}

void AHistorySearchProcessor_findTravelledDistances::onTrackEnd(bool)
{
    if (Distance > 0) Hist->Fill(Distance);
    Distance = 0;
}

AHistorySearchProcessor * AHistorySearchProcessor_findTravelledDistances::clone() const
{
    AHistorySearchProcessor_findTravelledDistances * pr = new AHistorySearchProcessor_findTravelledDistances(*this);

    pr->Hist = (TH1D*)Hist->Clone();

    return pr;
}

bool AHistorySearchProcessor_findTravelledDistances::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findTravelledDistances * from = dynamic_cast<const AHistorySearchProcessor_findTravelledDistances*>(&other);
    if (!from) return false;

    ATH1D::merge(Hist, from->Hist);
    return true;
}

AHistorySearchProcessor_findProcesses::AHistorySearchProcessor_findProcesses(SelectionMode Mode, bool onlyHadronic, const QString & targetIsotopeStartsFrom) :
    Mode(Mode), OnlyHadronic(onlyHadronic), TargetIsotopeStartsFrom(targetIsotopeStartsFrom.simplified()) {}

void AHistorySearchProcessor_findProcesses::onLocalStep(const ATrackingStepData & tr)
{
    if (validateStep(tr))
    {
        const QString & Proc = tr.Process;
        auto it = FoundProcesses.find(Proc);
        if (it == FoundProcesses.end())
            FoundProcesses[Proc] = 1;
        else ++(it->second);
    }
}

void AHistorySearchProcessor_findProcesses::onTransitionOut(const ATrackingStepData & tr)
{
    if (validateStep(tr))
    {
        auto it = FoundProcesses.find("Out");
        if (it == FoundProcesses.end())
            FoundProcesses["Out"] = 1;
        else ++(it->second);
    }
}

void AHistorySearchProcessor_findProcesses::onTransitionIn(const ATrackingStepData & tr)
{
    if (validateStep(tr))
    {
        auto it = FoundProcesses.find("In");
        if (it == FoundProcesses.end())
            FoundProcesses["In"] = 1;
        else ++(it->second);
    }
}

AHistorySearchProcessor * AHistorySearchProcessor_findProcesses::clone() const
{
    return new AHistorySearchProcessor_findProcesses(*this);
}

bool AHistorySearchProcessor_findProcesses::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findProcesses * from = dynamic_cast<const AHistorySearchProcessor_findProcesses*>(&other);
    if (!from) return false;

    for (const auto & itOther : from->FoundProcesses)
    {
        auto itHere = FoundProcesses.find(itOther.first);
        if (itHere == FoundProcesses.end())
            FoundProcesses[itOther.first] = itOther.second;
        else
            itHere->second += itOther.second;
    }
    return true;
}

void AHistorySearchProcessor_findProcesses::getResults(std::vector<std::pair<QString, int> > & data) const
{
    data.clear();
    data.reserve(FoundProcesses.size());

    for (const auto & pair : FoundProcesses)
        data.push_back( {pair.first, pair.second} );

    std::sort(data.begin(), data.end(),
              [](const std::pair<QString,int> & lhs, const std::pair<QString,int> & rhs)
                {return lhs.second > rhs.second;}
             );
}

bool AHistorySearchProcessor_findProcesses::validateStep(const ATrackingStepData & tr) const
{
    if (OnlyHadronic)
    {
        if (tr.TargetIsotope.isEmpty()) return false;
        if (!TargetIsotopeStartsFrom.isEmpty() && !tr.TargetIsotope.startsWith(TargetIsotopeStartsFrom)) return false;
    }

    switch (Mode)
    {
    case All :                  return true;
    case WithEnergyDeposition : return (tr.DepositedEnergy != 0);
    case TrackEnd :             return (tr.Energy == 0 || tr.Process == "O");
    }
    return false; // just to avoid warning
}

AHistorySearchProcessor_findHadronicChannels::AHistorySearchProcessor_findHadronicChannels()
{
    Aliases.push_back( {"gamma",    "g"} );
    Aliases.push_back( {"proton",   "p"} );
    Aliases.push_back( {"neutron",  "n"} );
    Aliases.push_back( {"alpha",    "a"} );
    Aliases.push_back( {"deuteron", "d"} );
    Aliases.push_back( {"triton",   "t"} );
}
const QString & AHistorySearchProcessor_findHadronicChannels::getAlias(const QString & name)
{
    for (const auto & pair : Aliases)
        if (pair.first == name) return pair.second;
    return name;
}

bool AHistorySearchProcessor_findHadronicChannels::onNewTrack(const AParticleTrackingRecord & pr)
{
    Particle = getAlias(pr.ParticleName);
    TrackRecord = &pr;
    return false;
}

void AHistorySearchProcessor_findHadronicChannels::onLocalStep(const ATrackingStepData & tr)
{
    const QString & Proc = tr.Process;
    if (Proc == "hadElastic") return;

    if (tr.TargetIsotope.isEmpty()) return;
    if (tr.Secondaries.empty())
    {
        // unexpected!
        //qWarning() << "No secondaries for the hadronic reacion:" << Particle << "on" << tr.TargetIsotope;
        return;
    }

    std::vector<QString> ProductVec;
    std::vector<QString> IsotopeVec;
    for (int iSec : tr.Secondaries)
    {
        const AParticleTrackingRecord * sec = TrackRecord->getSecondaries()[iSec];
        const QString & pn = sec->ParticleName;
        if (pn[0] == pn[0].toUpper()) IsotopeVec.push_back(pn);
        else                          ProductVec.push_back( getAlias(pn) );
    }

    QString Result;
    if (IsotopeVec.empty())
    {
        auto it = std::find(ProductVec.begin(), ProductVec.end(), "a");
        if (it != ProductVec.end())
        {
            Result = "He4";
            ProductVec.erase(it);
        }
        else
        {
            auto it = std::find(ProductVec.begin(), ProductVec.end(), "t");
            if (it != ProductVec.end())
            {
                Result = "H3";
                ProductVec.erase(it);
            }
            else
            {
                auto it = std::find(ProductVec.begin(), ProductVec.end(), "d");
                if (it != ProductVec.end())
                {
                    Result = "H2";
                    ProductVec.erase(it);
                }
                else
                {
                    auto it = std::find(ProductVec.begin(), ProductVec.end(), "p");
                    if (it != ProductVec.end())
                    {
                        Result = "H1";
                        ProductVec.erase(it);
                    }
                    else
                    {
                        if (ProductVec.empty())
                        {
                            qWarning() << "Unexpected empty list of products of an inelastic hadron reaction";
                            exit(222);
                        }
                        else
                        {
                            Result = ProductVec.front();
                            ProductVec.erase(ProductVec.begin());
                        }
                    }
                }
            }
        }
    }
    else if (IsotopeVec.size() == 1)
    {
        Result = IsotopeVec.front();
    }
    else // size > 1
    {
        //qDebug() << IsotopeVec << ProductVec;
        std::vector<QString>::iterator itMax = IsotopeVec.begin();
        int maxA = -1;
        for (auto it = IsotopeVec.begin(); it < IsotopeVec.end(); ++it)
        {
            QString n = *it;
            while (n[0].isLetter()) n = n.remove(0, 1);
            bool ok;
            int A = n.toInt(&ok);
            if (ok && A > maxA)
            {
                itMax = it;
                maxA = A;
            }
        }

        Result = *itMax;
        IsotopeVec.erase(itMax);
        std::copy(IsotopeVec.begin(), IsotopeVec.end(), std::back_inserter(ProductVec));
        //qDebug() << "-->" << Result << ProductVec;
    }

    //if (Result == tr.TargetIsotope) return;

    std::sort(ProductVec.begin(), ProductVec.end());

    QString products;
    QString previous = "g"; // not saving gammas
    int counter = 0;
    for (size_t iP = 0; iP < ProductVec.size(); iP++)
    {
        const QString & n = ProductVec[iP];

        if (n == previous)
        {
            counter++;
            continue;
        }

        if (previous != "g")
        {
            // save previous product
            if (!products.isEmpty()) products += "+";
            if (counter != 1) products += QString::number(counter);
            products += previous;

            previous.clear();
            counter = 0;
        }

        previous = n;
        counter = 1;
    }

    //saving the last record
    if (previous != "g")
    {
        if (!products.isEmpty()) products += "+";
        if (counter != 1) products += QString::number(counter);
        products += previous;
    }

    if (products.isEmpty()) products = "g";

    const QString channel = QString("%0(%1,%2)%3").arg(tr.TargetIsotope, Particle, products, Result);

    auto it = Channels.find(channel);
    if (it == Channels.end()) Channels[channel] = 1;
    else                      ++(it->second);
}

AHistorySearchProcessor * AHistorySearchProcessor_findHadronicChannels::clone() const
{
    return new AHistorySearchProcessor_findHadronicChannels(*this);
}

bool AHistorySearchProcessor_findHadronicChannels::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_findHadronicChannels * from = dynamic_cast<const AHistorySearchProcessor_findHadronicChannels*>(&other);
    if (!from) return false;

    for (const auto & itOther : from->Channels)
    {
        auto itHere = Channels.find(itOther.first);
        if (itHere == Channels.end())
            Channels[itOther.first] = itOther.second;
        else
            itHere->second += itOther.second;
    }
    return true;
}

void AHistorySearchProcessor_findHadronicChannels::getResults(std::vector<std::pair<QString, int>> & data) const
{
    data.clear();
    data.reserve(Channels.size());

    for (const auto & pair : Channels)
        data.push_back( {pair.first, pair.second} );

    std::sort(data.begin(), data.end(),
              [](const std::pair<QString,int> & lhs, const std::pair<QString,int> & rhs)
                {return lhs.second > rhs.second;}
             );
}

AHistorySearchProcessor_Border::AHistorySearchProcessor_Border(const QString &what,
                                                               const QString &cuts,
                                                               int bins, double from, double to)
{
    QString s = what;
    formulaWhat1 = parse(s);
    if (!formulaWhat1) ErrorString = "Invalid formula for 'what'";
    else
    {
        if (!cuts.isEmpty())
        {
            QString s = cuts;
            formulaCuts = parse(s);
            if (!formulaCuts) ErrorString = "Invalid formula for cuts";
        }

        if (formulaCuts || cuts.isEmpty())
        {
            Hist1D = new TH1D("", "", bins, from, to);
            TString title = what.toLocal8Bit().data();
            Hist1D->GetXaxis()->SetTitle(title);
            title += ", with ";
            title += cuts.toLocal8Bit().data();
            Hist1D->SetTitle(title);
        }
    }
}

AHistorySearchProcessor_Border::AHistorySearchProcessor_Border(const QString &what, const QString &vsWhat,
                                                               const QString &cuts,
                                                               int bins, double from, double to)
{
    QString s = what;
    formulaWhat1 = parse(s);
    if (!formulaWhat1) ErrorString = "Invalid formula for 'what'";
    else
    {
        s = vsWhat;
        formulaWhat2 = parse(s);
        if (!formulaWhat2) ErrorString = "Invalid formula for 'vsWhat'";
        else
        {
            if (!cuts.isEmpty())
            {
                QString s = cuts;
                formulaCuts = parse(s);
                if (!formulaCuts) ErrorString = "Invalid formula for cuts";
            }

            if (formulaCuts || cuts.isEmpty())
            {
                Hist1D = new TH1D("", "", bins, from, to);
                TString titleY = what.toLocal8Bit().data();
                Hist1D->GetYaxis()->SetTitle(titleY);
                TString titleX = vsWhat.toLocal8Bit().data();
                Hist1D->GetXaxis()->SetTitle(titleX);
                TString title = titleY + " vs " + titleX;
                title += ", with ";
                title += cuts.toLocal8Bit().data();
                Hist1D->SetTitle(title);

                Hist1Dnum = new TH1D("", "", bins, from, to);
            }
        }
    }
}

AHistorySearchProcessor_Border::AHistorySearchProcessor_Border(const QString &what, const QString &vsWhat,
                                                               const QString &cuts,
                                                               int bins1, double from1, double to1,
                                                               int bins2, double from2, double to2)
{
    QString s = what;
    formulaWhat1 = parse(s);
    if (!formulaWhat1) ErrorString = "Invalid formula for 'what'";
    else
    {
        s = vsWhat;
        formulaWhat2 = parse(s);
        if (!formulaWhat2) ErrorString = "Invalid formula for 'vsWhat'";
        else
        {
            if (!cuts.isEmpty())
            {
                s = cuts;
                formulaCuts = parse(s);
                if (!formulaCuts) ErrorString = "Invalid formula for cuts";
            }

            if (formulaCuts || cuts.isEmpty())
            {
                Hist2D = new TH2D("", "", bins1, from1, to1, bins2, from2, to2);
                TString titleY = what.toLocal8Bit().data();
                Hist2D->GetYaxis()->SetTitle(titleY);
                TString titleX = vsWhat.toLocal8Bit().data();
                Hist2D->GetXaxis()->SetTitle(titleX);
                TString title = titleY + " vs " + titleX;
                if (!cuts.isEmpty())
                {
                    title += ", with ";
                    title += cuts.toLocal8Bit().data();
                }
                Hist2D->SetTitle(title);
            }
        }
    }
}

AHistorySearchProcessor_Border::AHistorySearchProcessor_Border(const QString &what, const QString &vsWhat, const QString &andVsWhat,
                                                               const QString &cuts,
                                                               int bins1, double from1, double to1,
                                                               int bins2, double from2, double to2)
{
    QString s = what;
    formulaWhat1 = parse(s);
    if (!formulaWhat1) ErrorString = "Invalid formula for 'what'";
    else
    {
        s = vsWhat;
        formulaWhat2 = parse(s);
        if (!formulaWhat2) ErrorString = "Invalid formula for 'vsWhat'";
        else
        {
            s = andVsWhat;
            formulaWhat3 = parse(s);
            if (!formulaWhat3) ErrorString = "Invalid formula for 'andVsWhat'";
            else
            {
                if (!cuts.isEmpty())
                {
                    s = cuts;
                    formulaCuts = parse(s);
                    if (!formulaCuts) ErrorString = "Invalid formula for cuts";
                }

                if (formulaCuts || cuts.isEmpty())
                {
                    Hist2D = new TH2D("", "", bins1, from1, to1, bins2, from2, to2);
                    TString titleZ = what.toLocal8Bit().data();
                    Hist2D->GetZaxis()->SetTitle(titleZ);
                    TString titleX = vsWhat.toLocal8Bit().data();
                    Hist2D->GetXaxis()->SetTitle(titleX);
                    TString titleY = andVsWhat.toLocal8Bit().data();
                    Hist2D->GetYaxis()->SetTitle(titleY);
                    TString title = titleZ + " vs " + titleX + " and " + titleY;
                    if (!cuts.isEmpty())
                    {
                        title += ", with ";
                        title += cuts.toLocal8Bit().data();
                    }
                    Hist2D->SetTitle(title);

                    Hist2Dnum = new TH2D("", "", bins1, from1, to1, bins2, from2, to2);
                }
            }
        }
    }
}

AHistorySearchProcessor_Border::~AHistorySearchProcessor_Border()
{
    delete formulaWhat1;
    delete formulaWhat2;
    delete formulaWhat3;
    delete formulaCuts;
    delete Hist1D;
    delete Hist2D;
}

void AHistorySearchProcessor_Border::afterSearch()
{
    //calculating average of the bins for two cases

    if (Hist1D)
    {
        //1D case
        if (formulaWhat2)
        {
            int numEntr = Hist1D->GetEntries();
            *Hist1D = *Hist1D / *Hist1Dnum;
            Hist1D->SetEntries(numEntr);
        }
    }
    else
    {
        if (formulaWhat3)
        {
            int numEntr = Hist2D->GetEntries();
            *Hist2D = *Hist2D / *Hist2Dnum;
            Hist2D->SetEntries(numEntr);
        }
    }
}

void AHistorySearchProcessor_Border::onTransition(const ATrackingStepData &fromfromTr, const ATrackingStepData &fromTr)
{
    //double  x, y, z, time, energy, vx, vy, vz
    //        0  1  2    3     4      5   6   7

    par[0] = fromTr.Position[0];
    par[1] = fromTr.Position[1];
    par[2] = fromTr.Position[2];
    par[3] = fromTr.Time;
    par[4] = fromTr.Energy;

    if (bRequiresDirections)
    {
        double sum2 = 0;
        for (int i=0; i<3; i++)
        {
            const double delta = fromTr.Position[i] - fromfromTr.Position[i];
            sum2 += delta * delta;
        }
        double length = sqrt(sum2);

        if (length == 0)
        {
            par[5] = 0;
            par[6] = 0;
            par[7] = 0;
        }
        else
        {
            for (int i=0; i<3; i++)
                par[5+i] = (fromTr.Position[i] - fromfromTr.Position[i]) / length;
        }
    }

    if (formulaCuts)
    {
        bool bPass = formulaCuts->eval(par);
        if (!bPass) return;
    }

    if (Hist1D)
    {
        //1D case
        double res = formulaWhat1->eval(par);
        if (formulaWhat2)
        {
            double resX = formulaWhat2->eval(par);
            Hist1D->Fill(resX, res);
            Hist1Dnum->Fill(resX, 1.0);
        }
        else Hist1D->Fill(res);
    }
    else
    {
        if (formulaWhat3)
        {
            //3D case
            double res1 = formulaWhat1->eval(par);
            double res2 = formulaWhat2->eval(par);
            double res3 = formulaWhat3->eval(par);
            Hist2D->Fill(res2, res3, res1);
            Hist2Dnum->Fill(res2, res3, 1.0);
        }
        else
        {
            //2D case
            double res1 = formulaWhat1->eval(par);
            double res2 = formulaWhat2->eval(par);
            Hist2D->Fill(res2, res1);
        }
    }
}

AHistorySearchProcessor * AHistorySearchProcessor_Border::clone() const
{
    AHistorySearchProcessor_Border * p = new AHistorySearchProcessor_Border(*this);

    p->formulaWhat1 = ( formulaWhat1 ? new VFormula(*formulaWhat1) : nullptr);
    p->formulaWhat2 = ( formulaWhat2 ? new VFormula(*formulaWhat2) : nullptr);
    p->formulaWhat3 = ( formulaWhat3 ? new VFormula(*formulaWhat3) : nullptr);
    p->formulaCuts  = ( formulaCuts  ? new VFormula(*formulaCuts)  : nullptr);

    p->Hist1D    = ( Hist1D    ? (TH1D*)Hist1D->Clone()    : nullptr);
    p->Hist1Dnum = ( Hist1Dnum ? (TH1D*)Hist1Dnum->Clone() : nullptr);

    p->Hist2D    = ( Hist2D    ? (TH2D*)Hist2D->Clone()    : nullptr);
    p->Hist2Dnum = ( Hist2Dnum ? (TH2D*)Hist2Dnum->Clone() : nullptr);

    return p;
}

bool AHistorySearchProcessor_Border::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_Border * from = dynamic_cast<const AHistorySearchProcessor_Border*>(&other);
    if (!from) return false;

    //qDebug() << "->";
    //qDebug() << formulaWhat1->IsValid() << from->formulaWhat1->GetExpFormula();

    if (from->Hist1D)
    {
        /*
        for (int i = 1; i <= from->Hist1D->GetNbinsX(); i++)
            Hist1D->Fill(from->Hist1D->GetBinCenter(i), from->Hist1D->GetBinContent(i));
        */
        ATH1D::merge(Hist1D, from->Hist1D);
    }
    if (from->Hist1Dnum)
    {
        /*
        for (int i = 1; i <= from->Hist1Dnum->GetNbinsX(); i++)
            Hist1Dnum->Fill(from->Hist1Dnum->GetBinCenter(i), from->Hist1Dnum->GetBinContent(i));
        */
        ATH1D::merge(Hist1Dnum, from->Hist1Dnum);
    }

    if (from->Hist2D)
    {
        /*
        for (int ix = 1; ix <= from->Hist2D->GetNbinsX(); ix++)
        {
            const double X = from->Hist2D->GetXaxis()->GetBinCenter(ix);
            for (int iy = 1; iy <= from->Hist2D->GetNbinsY(); iy++)
                Hist2D->Fill(X, from->Hist2D->GetYaxis()->GetBinCenter(iy), from->Hist2D->GetBinContent(ix, iy));
        }
        */
        ATH2D::merge(Hist2D, from->Hist2D);
    }
    if (from->Hist2Dnum)
    {
        /*
        for (int ix = 1; ix <= from->Hist2Dnum->GetNbinsX(); ix++)
        {
            const double X = from->Hist2Dnum->GetXaxis()->GetBinCenter(ix);
            for (int iy = 1; iy <= from->Hist2Dnum->GetNbinsY(); iy++)
                Hist2Dnum->Fill(X, from->Hist2Dnum->GetYaxis()->GetBinCenter(iy), from->Hist2Dnum->GetBinContent(ix, iy));
        }
        */
        ATH2D::merge(Hist2Dnum, from->Hist2Dnum);
    }

    return true;
}

VFormula * AHistorySearchProcessor_Border::parse(QString & expr)
{
    //double  x, y, z, time, energy, vx, vy, vz
    //        0  1  2    3     4      5   6   7

    expr.replace("Energy", "p__4"); expr.replace("ENERGY", "p__4"); expr.replace("energy", "p__4");

    expr.replace("Time", "p__3"); expr.replace("TIME", "p__3"); expr.replace("time", "p__3");

    expr.replace("vx", "p__5"); expr.replace("Vx", "p__5"); expr.replace("VX", "p__5");
    expr.replace("vy", "p__6"); expr.replace("Vy", "p__6"); expr.replace("VY", "p__6");
    expr.replace("vz", "p__7"); expr.replace("Vz", "p__7"); expr.replace("VZ", "p__7");

    expr.replace("X", "p__0"); expr.replace("x", "p__0");
    expr.replace("Y", "p__1"); expr.replace("y", "p__1");
    expr.replace("Z", "p__2"); expr.replace("z", "p__2");

    VFormula * f = new VFormula();
    std::vector<std::string> names;
    for (int i = 0; i < 8; i++) names.push_back(std::string{"p__"} + std::to_string(i));
    f->setVariableNames(names);

    bool ok = f->parse(expr.toLatin1().data());
    if (ok) ok = f->validate();

    if (ok)
    {
        bRequiresDirections = false;
        if (expr.contains("p__5") || expr.contains("p__6") || expr.contains("p__7")) bRequiresDirections = true;
        return f;
    }

    delete f;
    return nullptr;
    /*
    expr.replace("X", "[0]");
    expr.replace("Y", "[1]");
    expr.replace("Z", "[2]");

    expr.replace("Time", "[3]");
    expr.replace("time", "[3]");

    expr.replace("Energy", "[4]");
    expr.replace("energy", "[4]");

    bRequiresDirections = false;
    if (expr.contains("VX") || expr.contains("VY") || expr.contains("VZ")) bRequiresDirections = true;
    if (expr.contains("Vx") || expr.contains("Vy") || expr.contains("Vz")) bRequiresDirections = true;
    expr.replace("Vx", "[5]");
    expr.replace("VX", "[5]");
    expr.replace("Vy", "[6]");
    expr.replace("VY", "[6]");
    expr.replace("Vz", "[7]");
    expr.replace("VZ", "[7]");

    TFormula * f = new TFormula("", expr.toLocal8Bit().data());
    if (f && f->IsValid()) return f;

    delete f;
    return nullptr;
    */
}


bool AHistorySearchProcessor_getDepositionStats::onNewTrack(const AParticleTrackingRecord &pr)
{
    if ( ParticleName != pr.ParticleName)  // fast?  want to avoid re-search in map if possible
    {
        ParticleName = pr.ParticleName;
        bAlreadyFound = false;
    }
    else
    {
        // new track with the same type of particle
        // iterator is valid
    }
    return false;
}

void AHistorySearchProcessor_getDepositionStats::onLocalStep(const ATrackingStepData &tr)
{
    if (tr.DepositedEnergy == 0) return;

    const double & depo = tr.DepositedEnergy;

    if (!bAlreadyFound)
    {
        itParticle = DepoData.find(ParticleName);

        if (itParticle == DepoData.end())
        {
            DepoData[ParticleName] = AParticleDepoStat(1, depo, depo*depo);
            return;
        }
        else bAlreadyFound = true;
    }

    itParticle->second.append(depo);
}

void AHistorySearchProcessor_getDepositionStats::onTransitionOut(const ATrackingStepData &tr)
{
    onLocalStep(tr);
}

AHistorySearchProcessor * AHistorySearchProcessor_getDepositionStats::clone() const
{
    return new AHistorySearchProcessor_getDepositionStats(*this);
}

bool AHistorySearchProcessor_getDepositionStats::mergeResuts(const AHistorySearchProcessor & other)
{
    const AHistorySearchProcessor_getDepositionStats * from = dynamic_cast<const AHistorySearchProcessor_getDepositionStats*>(&other);
    if (!from) return false;

    for (const auto & itOther : from->DepoData)
    {
        auto itHere = DepoData.find(itOther.first);
        if (itHere == DepoData.end())
            DepoData[itOther.first] = itOther.second;
        else
            itHere->second.merge(itOther.second);
    }
    return true;
}

double AHistorySearchProcessor_getDepositionStats::getResults(std::vector<std::tuple<QString,double,double,int,double,double> > & data) const
{
    data.clear();
    data.reserve(DepoData.size());

    double sum = 0;
    for (const auto & pair : DepoData)
        sum += pair.second.sum;
    const double sumInv = (sum > 0 ? 100.0/sum : 1.0);

    // Particle SumDepo FractionDepo Number Mean Sigma     --> Mean!=0 if Number>1 ; Sigma !=0 if Number > 5
    for (const auto & pair : DepoData)
    {
        const AParticleDepoStat & rec = pair.second;

        double mean = rec.sum / rec.num;
        double sigma = 0;
        if (rec.num > 5) sigma = sqrt( (rec.sumOfSquares - 2.0*mean*rec.sum)/rec.num + mean*mean );

        data.push_back( {pair.first, rec.sum, rec.sum*sumInv, rec.num, (rec.num > 1 ? mean : 0), sigma} );
    }

    std::sort(data.begin(), data.end(),
              [](const std::tuple<QString,double,double,int,double,double> & lhs, const std::tuple<QString,double,double,int,double,double> & rhs)
                {return std::get<1>(lhs) > std::get<1>(rhs);}
             );

    return sum;
}

AHistorySearchProcessor_getDepositionStatsTimeAware::AHistorySearchProcessor_getDepositionStatsTimeAware(double timeFrom, double timeTo) :
    AHistorySearchProcessor_getDepositionStats(), timeFrom(timeFrom), timeTo(timeTo) {}

void AHistorySearchProcessor_getDepositionStatsTimeAware::onLocalStep(const ATrackingStepData &tr)
{
    if (tr.DepositedEnergy == 0) return;

    if (tr.Time < timeFrom) return;
    if (tr.Time > timeTo)   return;

    AHistorySearchProcessor_getDepositionStats::onLocalStep(tr);
}

void AHistorySearchProcessor_getDepositionStatsTimeAware::onTransitionOut(const ATrackingStepData &tr)
{
    onLocalStep(tr);
}

AHistorySearchProcessor * AHistorySearchProcessor_getDepositionStatsTimeAware::clone() const
{
    return new AHistorySearchProcessor_getDepositionStatsTimeAware(*this);
}
