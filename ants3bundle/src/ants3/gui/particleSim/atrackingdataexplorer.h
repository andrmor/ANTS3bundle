#ifndef ATRACKINGDATAEXPLORER_H
#define ATRACKINGDATAEXPLORER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>

class AGeometryHub;
class AParticleTrackingRecord;
class AEventTrackingRecord;

class ATrackingDataExplorer : public QObject
{
    Q_OBJECT

public:
    ATrackingDataExplorer();

    QString buildTracks(const QString & fileName, const QStringList & LimitToParticles, const QStringList & ExcludeParticles,
                        bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries,
                        const int MaxTracks, int LimitToEvent = -1);

    void buildTracksForEventRecord(AEventTrackingRecord * record, bool skipTracksForSecondaries);

public slots:
    void abortEventProcessing() {AbortEventProcessingFlag = true;}

signals:
    void reportEventsProcessed(int numEvents);

private:
    AGeometryHub & Geometry;
    bool AbortEventProcessingFlag = false;

    void addTrack(const AParticleTrackingRecord * r, const QSet<QString> & LimitTo, bool bCheckLimitTo, const QSet<QString> & Exclude,
                  bool bCheckExclude, bool SkipPrimaries, bool SkipPrimNoInter, bool SkipSecondaries, int & iTrack, int MaxTracks);
};

#endif // ATRACKINGDATAEXPLORER_H
