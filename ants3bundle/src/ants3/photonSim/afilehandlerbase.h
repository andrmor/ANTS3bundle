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

    virtual void determineFormat();                 // very simplistic in the generic case, feel free to override for the concrete classes
    virtual bool checkFile(bool collectStatistics); // !!!*** how to handle statistics? --> separate (non-virtual?) method?

    bool init();
    bool isInitialized() const;

    bool gotoEvent(int iEvent);
    bool atEnd() const;

    bool readNextRecordSameEvent(ADataIOBase & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    bool copyToFileBuffered(int fromEvent, int toEvent, const QString & fileName, ADataIOBase & buffer);

    QString preview(ADataIOBase & buffer, int numLines = 100);

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

    void clearResources();
    bool processEventHeader();
};

#endif // AFILEHANDLERBASE_H
