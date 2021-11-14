#ifndef APHOTONBOMBFILEHANDLER_H
#define APHOTONBOMBFILEHANDLER_H

#include "aphotonsimsettings.h"

#include <QString>

class ANodeRecord;
class QFile;
class QTextStream;

// !!!*** reformat to AFileSettingsBase

class APhotonBombFileHandler
{
public:
    APhotonBombFileHandler(ANodeFileSettings & settings);
    virtual ~APhotonBombFileHandler();

    void determineFormat(); // very simplistic, better to make more strict !!!***
    bool checkFile(bool collectStatistics); // !!!*** add statistics!

    bool init();
    bool gotoEvent(int iEvent);

    bool readNextBombOfSameEvent(ANodeRecord & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName);

private:
    ANodeFileSettings & Settings;

    int             CurrentEvent = -1;
    bool            EventEndReached = false;

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         LineText;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    char            Header        = 0x00;

    void clearResources();
    bool processEventHeader();
};

#endif // APHOTONBOMBFILEHANDLER_H
