#ifndef AFILEHANDLERBASE_H
#define AFILEHANDLERBASE_H

#include "afilesettingsbase.h"

#include <QString>
#include <QDateTime>

class ANodeRecord;
class QFile;
class QTextStream;
class QJsonObject;

class AFileHandlerBase
{
public:
    AFileHandlerBase(AFileSettingsBase & settings);
    virtual ~AFileHandlerBase();

    virtual void determineFormat();                 // very simplistic in the generic case, feel free to override for the concrete classes
    virtual bool checkFile(bool collectStatistics); // !!!*** how to handle statistics?

    bool init();
    bool gotoEvent(int iEvent);

    //bool readNext(??? & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName);

protected:
    AFileSettingsBase & BaseSettings;

    int             CurrentEvent    = -1;
    bool            EventEndReached = false;

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
