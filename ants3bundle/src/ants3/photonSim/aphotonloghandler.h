#ifndef APHOTONLOGHANDLER_H
#define APHOTONLOGHANDLER_H

#include "aphotonhistorylog.h"

#include <QString>

#include <vector>

class QFile;
class QTextStream;

class APhotonLogHandler
{
public:
    APhotonLogHandler();

    enum EReadStatus {SNormal = 0, SEndOfPhoton, SEndOfFile, SError};
    QString ErrorString;

    bool init(const QString & fileName);

    bool readNextPhotonLog();
    bool readNextPhotonLogFiltered(const APhotonLogSettings & PhotonLogSet);
    void logToText(QString & text);
    void populateTrack();
    void populateAllTracks(bool doFiltering, const APhotonLogSettings & PhotonLogSet);

private:
    QFile       * File = nullptr;
    QTextStream * Stream = nullptr;

    std::vector<APhotonHistoryLog> PhotonLog;

    EReadStatus readRecordFromFile(APhotonHistoryLog & rec);
};

#endif // APHOTONLOGHANDLER_H
