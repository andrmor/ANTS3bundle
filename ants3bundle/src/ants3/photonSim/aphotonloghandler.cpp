#include "aphotonloghandler.h"


#include <QFile>
#include <QTextStream>

APhotonLogHandler::APhotonLogHandler() {}

bool APhotonLogHandler::init(const QString & fileName)
{
    delete Stream; Stream = nullptr;
    delete File;   File = nullptr;
    ErrorString.clear();

    File = new QFile(fileName);
    if(!File->open(QIODevice::ReadOnly | QFile::Text))
    {
        delete File; File = nullptr;
        ErrorString = "Could not open: " + fileName;
        return false;
    }

    if (File->size() == 0)
    {
        delete File; File = nullptr;
        ErrorString = "File is empty!";
        return false;
    }

    Stream = new QTextStream(File);
    QString line = Stream->readLine();
    if (line != "#")
    {
        delete Stream; Stream = nullptr;
        delete File;   File = nullptr;
        ErrorString = "Unexpexted photon log file format: photon start char is missing";
        return false;
    }

    return true;
}

APhotonLogHandler::EReadStatus APhotonLogHandler::readRecordFromFile(APhotonHistoryLog & rec)
{
    const QString line = Stream->readLine();

    if (line == "#") return SEndOfPhoton;

    ErrorString = rec.parseFromString(line);
    if (Stream->atEnd()) return SEndOfFile;

    if (!ErrorString.isEmpty()) return SError;

    return SNormal;
}

bool APhotonLogHandler::readNextPhotonLog()
{
    if (!Stream)
    {
        ErrorString = "PhotonLog file reader was not yet initialised.";
        return false;
    }

    APhotonHistoryLog record;

    PhotonLog.clear();
    PhotonLog.reserve(16);

    while (!Stream->atEnd())
    {
        EReadStatus result = readRecordFromFile(record);
        if (result == SError) return false;
        if (result == SEndOfPhoton) break;

        PhotonLog.push_back(record);
        if (result == SEndOfFile) break;

        if (result == SNormal) continue;
    }
    return true;
}

bool APhotonLogHandler::readNextPhotonLogFiltered(const APhotonLogSettings & PhotonLogSet)
{
    while (!Stream->atEnd())
    {
        bool res = readNextPhotonLog();
        if (!res) return false;

        bool ok = APhotonHistoryLog::checkComplyWithFilters(PhotonLog, PhotonLogSet);
        if (ok) return true;
    }

    PhotonLog.clear(); // else shows the last one checked in the file
    return true;
}

void APhotonLogHandler::logToText(QString & text)
{
    for (const APhotonHistoryLog & rec : PhotonLog)
        text += rec.printToString() + "\n";
}

#include "ageometryhub.h"
#include "TGeoManager.h"
#include "TGeoTrack.h"
void APhotonLogHandler::populateTrack()
{
    if (PhotonLog.size() < 2) return;

    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;

    //const bool bHit = (json.contains("h") ? true : false);
    //const bool bSec = (json.contains("s") ? true : false);

    TGeoTrack * track = new TGeoTrack(1, 22);
    int Color = 7;
    //if (bSec) Color = kMagenta;
    if (PhotonLog.back().Process == APhotonHistoryLog::Detected) Color = 2;
    track->SetLineColor(Color);
    //track->SetLineWidth(th->Width);
    //track->SetLineStyle(th->Style);

    for (const APhotonHistoryLog & rec : PhotonLog)
        track->AddPoint(rec.Position[0], rec.Position[1], rec.Position[2], rec.Time); // skip if the same position? !!!***

    if (track->GetNpoints() > 1) GeoManager->AddTrack(track);
}

#include "aphotonsimsettings.h"
void APhotonLogHandler::populateAllTracks(bool doFiltering, const APhotonLogSettings & PhotonLogSet)
{
    if (!Stream) return;

    int numPhots = 0;
    int maxNumPhots = (doFiltering ? PhotonLogSet.MaxNumber : 10000); // fixed number!

    while (!Stream->atEnd() && numPhots < maxNumPhots)
    {
        bool ok = readNextPhotonLog();
        if (!ok) return;

        if (doFiltering && !APhotonHistoryLog::checkComplyWithFilters(PhotonLog, PhotonLogSet)) continue;

        populateTrack();
        numPhots++;
    }
}


