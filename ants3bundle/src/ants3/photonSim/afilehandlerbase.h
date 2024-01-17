#ifndef AFILEHANDLERBASE_H
#define AFILEHANDLERBASE_H

#include "afilesettingsbase.h"

#include <QString>
#include <QDateTime>

class ANodeRecord;
class QFile;
class QTextStream;
class QJsonObject;
class ADataIOBase;

class AFileHandlerBase
{
public:
    AFileHandlerBase(AFileSettingsBase & settings);
    virtual ~AFileHandlerBase();

    virtual void determineFormat();      // very simplistic in the generic case, feel free to override for the concrete classes
    virtual bool checkFile();

    bool collectStatistics(); // uses virtual fillStatisticsForCurrentEvent()

    bool init();
    bool isInitialized() const;

    bool gotoEvent(int iEvent);
    bool atEnd() const;

    bool readNextRecordSameEvent(ADataIOBase & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}
    void skipToNextEventRecord(); // not needed for ascii; for binary dummy-read event index

    bool copyToFileBuffered(int fromEvent, int toEvent, const QString & fileName, ADataIOBase & buffer);

    QString preview(ADataIOBase & buffer, int numLines = 10000);

protected:
    AFileSettingsBase & BaseSettings;

    int             CurrentEvent    = -1;
    bool            EventEndReached = false;
    bool            ReadingEvent    = false; // set to true when at least one record was read

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         LineText;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    char            Header        = 0x00;

    QString         FileType; // file type description, e.g. "photon bomb", used in error reporting

    bool            bFileEndReachedInGoto = false;

    double          OnStartLimit = 1e99;  // initial value used to fill different fields in statistics

    virtual void dummyReadBinaryDataUntilNewEvent(); // = 0 possible convertion
    virtual void clearStatistics() {}
    virtual void fillStatisticsForCurrentEvent() {}

    void clearResources();
    bool processEventHeader();

};

#endif // AFILEHANDLERBASE_H
