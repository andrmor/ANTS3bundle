#include "afileparticlegenerator.h"
#include "afilegeneratorsettings.h"
#include "aparticlesimsettings.h"
#include "aparticlerecord.h"
#include "ajsontools.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include <iostream>
#include <fstream>
#include <string>
#include <istream>
#include <iostream>
#include <algorithm>

AFileParticleGenerator::AFileParticleGenerator(AFileGeneratorSettings & settings) :
    Settings(settings) {}

AFileParticleGenerator::~AFileParticleGenerator()
{
    releaseResources();
}

bool AFileParticleGenerator::init()
{
    releaseResources();

    switch (Settings.FileFormat)
    {
    case AFileGeneratorSettings::G4Binary:
        Engine = new AFilePGEngineG4antsBin(Settings);
        break;
    case AFileGeneratorSettings::G4Ascii:
        Engine = new AFilePGEngineG4antsTxt(Settings);
        break;
    default:
        ErrorString = "Invalid file format";
        return false;
    }

    return Engine->doInit();
}

bool AFileParticleGenerator::initWithCheck(bool bExpanded)
{
    bool bNeedInspect = true; // !!!***

    releaseResources();

    if (Settings.FileName.empty())
    {
        ErrorString = "File name is not defined";
        return false;
    }

    Settings.clearStatistics();

    bool bOK = determineFileFormat();
    if (!bOK) return false;

    switch (Settings.FileFormat)
    {
    case AFileGeneratorSettings::G4Binary:
        Engine = new AFilePGEngineG4antsBin(Settings);
        break;
    case AFileGeneratorSettings::G4Ascii:
        Engine = new AFilePGEngineG4antsTxt(Settings);
        break;
    default:
        ErrorString = "Invalid file format";
        return false;
    }

    if (bNeedInspect)
        bOK = Engine->doInitAndInspect(bExpanded);
    else
        bOK = Engine->doInit();

    if (bOK && Settings.ParticleStat.size() > 0)
    {
        std::sort(Settings.ParticleStat.begin(), Settings.ParticleStat.end(),
                  [](const AParticleInFileStatRecord & l, const AParticleInFileStatRecord & r)
                  {
                    return (l.Entries > r.Entries);
                  });
    }
    return bOK;
}

bool AFileParticleGenerator::determineFileFormat()
{
    if (isFileG4Binary())
    {
        //qDebug() << "Assuming it is G4 bin format";
        Settings.FileFormat = AFileGeneratorSettings::G4Binary;
        return true;
    }

    if (isFileG4Ascii())
    {
        //qDebug() << "Assuming it is G4 ascii format";
        Settings.FileFormat = AFileGeneratorSettings::G4Ascii;
        return true;
    }

    Settings.FileFormat = AFileGeneratorSettings::BadFormat;
    if (!ErrorString.empty()) return false;

    ErrorString = "Unexpected format of the file " + Settings.FileName;
    return false;
}

bool AFileParticleGenerator::isFileG4Binary()
{
    std::ifstream inB(Settings.FileName, std::ios::in | std::ios::binary);
    if (!inB.is_open())
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }
    char ch;
    inB.get(ch);
    if (ch == (char)0xEE)
    {
        int iEvent = -1;
        inB.read((char*)&iEvent, sizeof(int));
        if (iEvent == 0) return true;
    }
    return false;
}

bool AFileParticleGenerator::isFileG4Ascii()
{
    std::ifstream inT(Settings.FileName);
    if (!inT.is_open())
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }

    std::string str;
    getline(inT, str);
    inT.close();
    if (str.size() > 1 && str[0] == '#')
    {
        QString line = QString(str.data()).mid(1);
        bool bOK;
        int iEvent = line.toInt(&bOK);
        if (bOK && iEvent == 0) return true;
    }
    return false;
}

void AFileParticleGenerator::releaseResources()
{
    delete Engine; Engine = nullptr;
}

bool AFileParticleGenerator::generateEvent(std::function<void(const AParticleRecord&)> handler, int /*iEvent*/)
{
    if (!Engine)
    {
        ErrorString = "Generator is not configured!";
        return false;
    }

    return Engine->doGenerateEvent(handler);
}

void AFileParticleGenerator::setStartEvent(int startEvent)
{
    if (Engine) Engine->doSetStartEvent(startEvent);
}

bool AFileParticleGenerator::generateG4File(int eventBegin, int eventEnd, const QString & FileName)
{
    if (Engine)
        return Engine->doGenerateG4File(eventBegin, eventEnd, FileName);
    else
        return false;
}

// ***************************************************************************



AFilePGEngineG4antsTxt::~AFilePGEngineG4antsTxt()
{
    if (inStream) inStream->close();
    delete inStream; inStream = nullptr;
}

bool AFilePGEngineG4antsTxt::doInit()
{
    inStream = new std::ifstream(Settings.FileName);
    if (!inStream->is_open())
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }
    return true;
}

bool AFilePGEngineG4antsTxt::doInitAndInspect(bool bDetailedInspection)
{
    inStream = new std::ifstream(Settings.FileName);

    if (!inStream->is_open())
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }

        //qDebug() << "Inspecting G4ants-generated txt file:" << FileName;
        std::string str;
        bool bWasParticle = true;
        bool bWasMulty    = false;
        Settings.NumEvents = 0;
        while (!inStream->eof())
        {
            getline( *inStream, str );
            if (str.empty())
            {
                if (inStream->eof())
                {
                    inStream->clear();  // will reuse the stream
                    break;
                }

                ErrorString = "Found empty line!";
                return false;
            }

            if (str[0] == '#')
            {
                //new event
                Settings.NumEvents++;
                if (bWasMulty)     Settings.statNumMultipleEvents++;
                if (!bWasParticle) Settings.statNumEmptyEventsInFile++;
                bWasMulty = false;
                bWasParticle = false;
                continue;
            }

            QStringList f = QString(str.data()).split(rx, Qt::SkipEmptyParts);
            //pname en x y z i j k time

            if (f.size() != 9)
            {
                ErrorString = "Bad format of particle record!";
                return false;
            }

            std::string name = f[0].toLatin1().data();

            if (bDetailedInspection)
            {
                bool bNotFound = true;
                for (AParticleInFileStatRecord & rec : Settings.ParticleStat)
                {
                    if (rec.Name == name)
                    {
                        rec.Entries++;
                        rec.Energy += f.at(1).toDouble();
                        bNotFound = false;
                        break;
                    }
                }

                if (bNotFound)
                    Settings.ParticleStat.push_back(AParticleInFileStatRecord(name, f.at(1).toDouble()));
            }

            if (bWasParticle) bWasMulty = true;
            bWasParticle = true;
        }
        if (bWasMulty)     Settings.statNumMultipleEvents++;
        if (!bWasParticle) Settings.statNumEmptyEventsInFile++;

        return true;
}

//bool AFilePGEngineG4antsTxt::doGenerateEvent(QVector<AParticleRecord *> &GeneratedParticles)
bool AFilePGEngineG4antsTxt::doGenerateEvent(std::function<void (const AParticleRecord &)> handler)
{
    std::string str;
    while ( getline(*inStream, str) )
    {
        qDebug() << str.data();
        if (str.empty())
        {
            if (inStream->eof()) return false;
            ErrorString = "Found empty line!";
            return false;
        }

        if (str[0] == '#') return true; //new event

        QStringList f = QString(str.data()).split(rx, Qt::SkipEmptyParts); //pname en time x y z i j k
        if (f.size() != 9)
        {
            ErrorString = "Bad format of particle record!";
            return false;
        }

        QString name = f.first();
        //kill [***] appearing in ion names
        int iBracket = name.indexOf('[');
        if (iBracket != -1) name = name.left(iBracket);

        AParticleRecord p;
     #ifdef GEANT4
        // p->particle = !!!***
     #else
        p.particle = name.toLatin1().data();
     #endif
        p.energy = f.at(1).toDouble();
        p.r[0]   = f.at(2).toDouble();
        p.r[1]   = f.at(3).toDouble();
        p.r[2]   = f.at(4).toDouble();
        p.v[0]   = f.at(5).toDouble();
        p.v[1]   = f.at(6).toDouble();
        p.v[2]   = f.at(7).toDouble();
        p.time   = f.at(8).toDouble();

        handler(p);
    }

    if (inStream->eof()) return true;
    return false;
}

bool AFilePGEngineG4antsTxt::doSetStartEvent(int startEvent)
{
    if (!inStream) return false;

    inStream->seekg(0);

    std::string str;

    while ( getline(*inStream, str) )
    {
        if (str[0] == '#') //new event
        {
            const int iEvent = std::stoi( str.substr(1) );  // kill the leading '#' - the file was already verified at this stage!
            if (iEvent == startEvent)
                return true;
        }
    }
    return false;
}

bool AFilePGEngineG4antsTxt::doGenerateG4File(int eventBegin, int eventEnd, const QString &FileName)
{
    std::ofstream outStream;
    outStream.open(FileName.toLatin1().data());
    if (!outStream.is_open())
    {
        ErrorString = "Cannot open file to export primaries for G4ants sim";
        return false;
    }

    int currentEvent = -1;
    int eventsToDo = eventEnd - eventBegin;
    bool bSkippingEvents = (eventBegin != 0);
    inStream->seekg(0);
    std::string str;
    while ( getline(*inStream, str) )
    {
        if (inStream->eof())
        {
            if (eventsToDo == 0) return true;
            else
            {
                ErrorString = "Unexpected end of file";
                return false;
            }
        }
        if (str.empty())
        {
            ErrorString = "Found empty line!";
            return false;
        }

        if (str[0] == '#')
        {
            if (eventsToDo == 0) return true;
            currentEvent++;

            if (bSkippingEvents && currentEvent == eventBegin) bSkippingEvents = false;

            if (!bSkippingEvents)
            {
                outStream << str << std::endl;
                eventsToDo--;
            }
            continue;
        }

        if (!bSkippingEvents) outStream << str << std::endl;
    }

    if (eventsToDo == 0) return true;

    ErrorString = "Unexpected end of file";
    return false;
}

AFilePGEngineG4antsBin::~AFilePGEngineG4antsBin()
{
    if (inStream) inStream->close();
    delete inStream; inStream = nullptr;
}

bool AFilePGEngineG4antsBin::doInit()
{
    inStream = new std::ifstream(Settings.FileName, std::ios::in | std::ios::binary);
    if (!inStream->is_open()) //paranoic
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }
    inStream->clear();
    return true;
}

bool AFilePGEngineG4antsBin::doInitAndInspect(bool bDetailedInspection)
{
    inStream = new std::ifstream(Settings.FileName, std::ios::in | std::ios::binary);

    if (!inStream->is_open()) //paranoic
    {
        ErrorString = "Cannot open file " + Settings.FileName;
        return false;
    }

        //qDebug() << "Inspecting G4ants-generated bin file:" << FileName;
        bool bWasParticle = true;
        bool bWasMulty    = false;
        std::string ParticleName;
        double energy, time;
        double PosDir[6];
        char h;
        int eventId;
        Settings.NumEvents = 0;
        while (inStream->get(h))
        {
            if (h == char(0xEE)) //new event
            {
                inStream->read((char*)&eventId, sizeof(int));
                Settings.NumEvents++;
                if (bWasMulty)     Settings.statNumMultipleEvents++;
                if (!bWasParticle) Settings.statNumEmptyEventsInFile++;
                bWasMulty = false;
                bWasParticle = false;
            }
            else if (h == char(0xFF))
            {
                //data line
                ParticleName.clear();
                bool bCopyToName = true;
                while (*inStream >> h)
                {
                    if (h == (char)0x00) break;
                    if (bCopyToName)
                    {
                        if (h == '[') bCopyToName = false;
                        else ParticleName += h;
                    }
                }

                inStream->read((char*)&energy,       sizeof(double));
                inStream->read((char*)&PosDir,     6*sizeof(double));
                inStream->read((char*)&time,         sizeof(double));
                if (inStream->fail())
                {
                    ErrorString = "Unexpected format of a line in the binary file with the input particles";
                    return false;
                }

                if (bDetailedInspection)
                {
                    bool bNotFound = true;
                    for (AParticleInFileStatRecord & rec : Settings.ParticleStat)
                    {
                        if (rec.Name == ParticleName)
                        {
                            rec.Entries++;
                            rec.Energy += energy;
                            bNotFound = false;
                            break;
                        }
                    }

                    if (bNotFound)
                        Settings.ParticleStat.push_back(AParticleInFileStatRecord(ParticleName, energy));
                }

                if (bWasParticle) bWasMulty = true;
                bWasParticle = true;
            }
            else
            {
                ErrorString = "Format error in binary file!";
                return false;
            }
        }
        if (!inStream->eof())
        {
            ErrorString = "Format error in binary file!";
            return false;
        }
        if (bWasMulty)     Settings.statNumMultipleEvents++;
        if (!bWasParticle) Settings.statNumEmptyEventsInFile++;

    inStream->clear();
    return true;
}

//bool AFilePGEngineG4antsBin::doGenerateEvent(QVector<AParticleRecord *> & GeneratedParticles)
bool AFilePGEngineG4antsBin::doGenerateEvent(std::function<void(const AParticleRecord&)> handler)
{
    char h;
    int eventId;
    std::string pn;
    while (inStream->get(h))
    {
        if (h == (char)0xEE)
        {
            //next event starts here!
            inStream->read((char*)&eventId, sizeof(int));
            return true;
        }
        else if (h == (char)0xFF)
        {
            //data line
            pn.clear();
            bool bCopyToName = true;
            while (*inStream >> h)
            {
                if (h == (char)0x00) break;

                if (bCopyToName)
                {
                    if (h == '[') bCopyToName = false;
                    else pn += h;
                }
            }

            AParticleRecord p;
        #ifdef GEANT4
            //p->particle =   ; !!!*** definition from name
        #else
            p.particle = pn;
        #endif
            inStream->read((char*)&p.energy,   sizeof(double));
            inStream->read((char*)&p.r,      3*sizeof(double));
            inStream->read((char*)&p.v,      3*sizeof(double));
            inStream->read((char*)&p.time,     sizeof(double));
            if (inStream->fail())
            {
                ErrorString = "Unexpected format of a line in the binary file with the input particles";
                return false;
            }

            //GeneratedParticles << p;
            handler(p);
        }
        else
        {
            ErrorString = "Unexpected format of a line in the binary file with the input particles";
            return false;
        }
    }

    if (inStream->eof()) return true;
    return false;
}

bool AFilePGEngineG4antsBin::doSetStartEvent(int startEvent)
{
    inStream->seekg(0);

    char h;
    int eventId;
    double buf[8];
    while (inStream->get(h))
    {
        if (h == (char)0xEE)
        {
            //next event starts here
            inStream->read((char*)&eventId, sizeof(int));
            //qDebug() << "Event Id in seaching for" << startEvent << " found:" << eventId;
            if (eventId == startEvent)
                return true;
        }
        else if (h == (char)0xFF)
        {
            //data line
            while (*inStream >> h)
            {
                if (h == (char)0x00) break;
            }
            inStream->read((char*)buf, 8*sizeof(double));
        }
        else
        {
            qWarning() << "Unexpected format of a line in the binary file with the input particles";
            return false;
        }
    }
    return false;
}

bool AFilePGEngineG4antsBin::doGenerateG4File(int eventBegin, int eventEnd, const QString &FileName)
{
    std::ofstream  outStream;
    outStream.open(FileName.toLatin1().data(), std::ios::out | std::ios::binary);
    if (!outStream.is_open())
    {
        ErrorString = "Cannot open file to export primaries for G4ants sim";
        return false;
    }

    int currentEvent = -1;
    int eventsToDo = eventEnd - eventBegin;
    bool bSkippingEvents = (eventBegin != 0);
    int eventId;
    std::string pn;
    double energy, time;
    double posDir[6];
    char ch;
    inStream->seekg(0);
    while (inStream->get(ch))
    {
        if (inStream->eof())
        {
            if (eventsToDo == 0) return true;
            else
            {
                ErrorString = "Unexpected end of file";
                return false;
            }
        }
        if (inStream->fail())
        {
            ErrorString = "Unexpected error during reading of a header char in the G4ants binary file";
            return false;
        }

        if (ch == (char)0xEE)
        {
            inStream->read((char*)&eventId, sizeof(int));
            if (eventsToDo == 0) return true;
            currentEvent++;

            if (bSkippingEvents && currentEvent == eventBegin) bSkippingEvents = false;

            if (!bSkippingEvents)
            {
                outStream << ch;
                outStream.write((char*)&eventId, sizeof(int));
                eventsToDo--;
            }

            continue;
        }
        else if (ch == (char)0xFF)
        {
            //data line
            pn.clear();
            while (*inStream >> ch)
            {
                if (ch == (char)0x00) break;
                pn += ch;
            }
            //qDebug() << pn.data();
            inStream->read((char*)&energy,   sizeof(double));
            inStream->read((char*)&posDir, 6*sizeof(double));
            inStream->read((char*)&time,     sizeof(double));
            if (inStream->fail())
            {
                ErrorString = "Unexpected format of a line in the G4ants binary file";
                return false;
            }
            if (!bSkippingEvents)
            {
                outStream << (char)0xFF;
                outStream << pn << (char)0x00;
                outStream.write((char*)&energy,  sizeof(double));
                outStream.write((char*)posDir, 6*sizeof(double));
                outStream.write((char*)&time,    sizeof(double));
            }
        }
        else
        {
            ErrorString = "Unexpected format of a header char in the binary file with the input particles";
            return false;
        }
    }

    if (eventsToDo == 0) return true;

    ErrorString = "Unexpected end of file";
    return false;
}

AParticleInFileStatRecord::AParticleInFileStatRecord(const std::string & name, double energy)
    : Name(name), Entries(1), Energy(energy) {}
