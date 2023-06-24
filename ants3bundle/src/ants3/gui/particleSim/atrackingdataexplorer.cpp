#include "atrackingdataexplorer.h"
#include "atrackingdataimporter.h"
#include "ageometryhub.h"
#include "aeventtrackingrecord.h"
#include "TGeoTrack.h"
#include "TGeoManager.h"
#include "aparticletrackvisuals.h"

#include <QApplication>

ATrackingDataExplorer::ATrackingDataExplorer() :
    QObject(),
    Geometry(AGeometryHub::getInstance()),
    EventRecord(AEventTrackingRecord::create()) {}

ATrackingDataExplorer::~ATrackingDataExplorer()
{
    delete EventRecord;
}

void ATrackingDataExplorer::addTrack(const AParticleTrackingRecord * r,
              const QSet<QString> & LimitTo, bool bCheckLimitTo, const QSet<QString> & Exclude, bool bCheckExclude,
              bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
              int & iTrack, int MaxTracks)
{
    if (iTrack >= MaxTracks) return;

    bool DoSkipPrim = SkipPrimaries || (SkipPrimNoInter && r->isNoInteractions());

    if (!DoSkipPrim && bCheckLimitTo)
    {
        if (!LimitTo.contains(r->ParticleName)) DoSkipPrim = true;
    }

    if (!DoSkipPrim && bCheckExclude)
    {
        if (Exclude.contains(r->ParticleName)) DoSkipPrim = true;
    }

    if (!DoSkipPrim)
    {
        TGeoTrack * track = new TGeoTrack(1, 22);

        AParticleTrackVisuals::getInstance().applyToParticleTrack(track, r->ParticleName);

        const std::vector<ATrackingStepData *> & Steps = r->getSteps();
        for (const ATrackingStepData * step : Steps)
        {
            if (step->Process != "T")
                track->AddPoint(step->Position[0], step->Position[1], step->Position[2], step->Time);
        }
        Geometry.GeoManager->AddTrack(track);
        iTrack++;
    }

    if (!SkipSecondaries)
    {
        const std::vector<AParticleTrackingRecord *> & Secondaries = r->getSecondaries();
        for (AParticleTrackingRecord * sec : Secondaries)
            addTrack(sec,
                     LimitTo, bCheckLimitTo, Exclude, bCheckExclude,
                     false, false, SkipSecondaries,  // already no primaries from this level down
                     iTrack, MaxTracks);
    }
}

QString ATrackingDataExplorer::buildTracks(const QString & fileName, const QStringList & LimitToParticles, const QStringList & ExcludeParticles,
                                           bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
                                           const int MaxTracks, int LimitToEvent)
{
    AbortEventProcessingFlag = false;
    Geometry.GeoManager->ClearTracks();

    ATrackingDataImporter tdi(fileName);
    if (!tdi.ErrorString.isEmpty()) return tdi.ErrorString;

    const QSet<QString> LimitTo(LimitToParticles.begin(), LimitToParticles.end());
    const bool bCheckLimitTo = !LimitTo.isEmpty();
    const QSet<QString> Exclude(ExcludeParticles.begin(), ExcludeParticles.end());
    const bool bCheckExclude = !Exclude.isEmpty();

    int iEvent = 0;
    int iTrack = 0;
    while (iTrack < MaxTracks && !AbortEventProcessingFlag)
    {
        //qDebug() << "TB--> Event:" << iEvent << "Track index:" << iTrack;
        emit reportEventsProcessed(iEvent);
        QApplication::processEvents();
        if (LimitToEvent >= 0)
        {
            if (iEvent < LimitToEvent)
            {
                iEvent++;
                continue;
            }

            if (iEvent > LimitToEvent) break;
        }

        //qDebug() << "TB--> Asking extractor for event #" << iEvent;
        bool ok = tdi.extractEvent(iEvent, EventRecord);
        iEvent++;

        if (!ok)
        {
            //qDebug() << tdi.ErrorString << tdi.isEndReached();
            if (tdi.isEndReached()) return "";
            return tdi.ErrorString;
        }

        const std::vector<AParticleTrackingRecord *> Prims = EventRecord->getPrimaryParticleRecords();
        for (const AParticleTrackingRecord * r : Prims)
            addTrack(r,
                     LimitTo, bCheckLimitTo, Exclude, bCheckExclude,
                     SkipPrimaries, SkipPrimNoInter, SkipSecondaries,
                     iTrack, MaxTracks);
    }

    return "";
}

void ATrackingDataExplorer::buildTracksForEventRecord(AEventTrackingRecord * record, bool skipTracksForSecondaries)
{
    Geometry.GeoManager->ClearTracks();

    if (!record) return;

    int iTrack = 0;
    QSet<QString> dummy;
    const std::vector<AParticleTrackingRecord *> Prims = record->getPrimaryParticleRecords();
    for (const AParticleTrackingRecord * r : Prims)
        addTrack(r,
                 dummy, false, dummy, false,
                 false, false, skipTracksForSecondaries,
                 iTrack, 1000);
}
