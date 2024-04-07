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

void APhotonLogHandler::logToText(QString & text)
{
    for (const APhotonHistoryLog & rec : PhotonLog)
        text += rec.print() + "\n";
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
    if (PhotonLog.back().process == APhotonHistoryLog::Detected) Color = 2;
    track->SetLineColor(Color);
    //track->SetLineWidth(th->Width);
    //track->SetLineStyle(th->Style);

    for (const APhotonHistoryLog & rec : PhotonLog)
        track->AddPoint(rec.r[0], rec.r[1], rec.r[2], rec.time); // skip if the same position? !!!***

    if (track->GetNpoints() > 1) GeoManager->AddTrack(track);
}

void APhotonLogHandler::populateAllTracks()
{
    if (!Stream) return;

    while (!Stream->atEnd())
    {
        bool ok = readNextPhotonLog();
        if (!ok) return;

        populateTrack();
    }
}


