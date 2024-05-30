#include "aphotonhistorylog.h"
#include "amaterialhub.h"

#include <QTextStream>
#include <QDebug>

APhotonHistoryLog::APhotonHistoryLog(const double * position, const TString & volumeName, int volumeIndex,
                                     double time, int iWave,
                                     APhotonHistoryLog::EProcessType process,
                                     int iMatIndex, int iMatIndexAfter,
                                     int number) :
    Process(process), VolumeName(volumeName), VolumeIndex(volumeIndex), Time(time),
    MatIndex(iMatIndex), MatIndexAfter(iMatIndexAfter), Number(number), Wave(iWave)
{
    Position[0] = position[0];
    Position[1] = position[1];
    Position[2] = position[2];
}

QString APhotonHistoryLog::printToString() const
{
    QString s;

    s += getProcessName(Process);

    //if (Process == HitSensor || Process == Detected || Process == NotDetected) s += " (#" + QString::number(Number) + ")";
    if (Process == Fresnel_Reflection || Process == Override_Loss || Process == Override_Back || Process == Fresnel_Transmition || Process == Override_Forward)
    {
        const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
        s += " [" + MatHub.getMaterialName(MatIndex) +"->"+ MatHub.getMaterialName(MatIndexAfter)+"]";
    }

    QString inat = "in";
    if (Process == Fresnel_Transmition || Process == Override_Forward) inat = "to";
    else if (Process == Escaped) inat = "from";
    if (VolumeName.Length() != 0) s += " " + inat + " " + QString(VolumeName) + "#" + QString::number(VolumeIndex);

    s += QString(" at (") + QString::number(Position[0])+","+ QString::number(Position[1]) + ","+QString::number(Position[2])+")";

    if (Wave != -1) s += " iWave="+QString::number(Wave);
    s += " at " + QString::number(Time)+" ns";

    return s;
}

void APhotonHistoryLog::sendToStream(QTextStream * s) const
{
    *s << Process << " ";           // 0
    for (size_t i = 0; i < 3; i++)
        *s << Position[i] << " ";          // 1 2 3
    *s << VolumeName.Data() << " "; // 4
    *s << VolumeIndex << " ";       // 5
    *s << Time << " ";              // 6
    *s << Wave << " ";              // 7
    *s << MatIndex << " ";          // 8
    *s << MatIndexAfter << " ";     // 9
    *s << Number << "\n";           // 10
}

QString APhotonHistoryLog::parseFromString(const QString & str)
{
    const QStringList sl = str.split(' ', Qt::SkipEmptyParts);
    if (sl.size() != 11) return "Wrong size of photon log record";

    bool ok;
    int iPr = sl[0].toInt(&ok);
    if (!ok || iPr < 0 || iPr >= __SizeOfNodeTypes__) return "Invalid process index";
    Process = static_cast<EProcessType>(iPr);

    for (int i = 0; i < 3; i++)
    {
        Position[i] = sl[1+i].toDouble(&ok);
        if (!ok) return "Invalid position";
    }

    VolumeName = TString(sl[4].toLatin1().data());

    VolumeIndex = sl[5].toInt(&ok);
    if (!ok) return "Invalid volume index";

    Time = sl[6].toDouble(&ok);
    if (!ok) return "Invalid time";

    Wave = sl[7].toInt(&ok);
    if (!ok) return "Invalid wave index";

    MatIndex = sl[8].toInt(&ok);
    if (!ok) return "Invalid materail index";

    MatIndexAfter = sl[9].toInt(&ok);
    if (!ok) return "Invalid second materail index";

    Number = sl[10].toInt(&ok);
    if (!ok) return "Invalid numeric parameter";

    return "";
}

QString APhotonHistoryLog::getProcessName(int nodeType)
{
    if (nodeType < 0 || nodeType >= __SizeOfNodeTypes__) return "Error: unknown index!";
    return AllProcessNames[nodeType];
/*
    switch (nodeType)
    {
    case Undefined: return "Undefined";
    case Created: return "Created";
    case HitSensor: return "HitPM";
    case Escaped: return "Escaped";
    case Absorbed: return "Absorbed";
    case MaxNumberCyclesReached: return "MaxNumberCyclesReached";
    case Rayleigh: return "Rayleigh";
    case Reemission: return "Reemission";
    case Fresnel_Reflection: return "Fresnel_Reflection";
    case Fresnel_Transmition: return "Fresnel_Transmition";
    case Override_Loss: return "Override_Loss";
    case Override_Forward: return "Override_Forward";
    case Override_Back: return "Override_Back";
    case Detected: return "Detected";
    case NotDetected: return "NotDetected";
    case GeneratedOutsideGeometry: return "GeneratedOutsideGeometry";
    case Grid_Enter: return "Grid_Enter";
    case Grid_Exit: return "Grid_Exit";
    case Grid_ShiftIn: return "Grid_ShiftIn";
    case Grid_ShiftOut: return "Grid_ShiftOut";
    case KilledByMonitor: return "StoppedByMonitor";
    default: return "Error: unknown index!";
    }
*/
}

int APhotonHistoryLog::getProcessIndex(const QString & name)
{
    for (int i = 0; i < AllProcessNames.size(); i++)
        if (name == AllProcessNames[i]) return i;
    return -1;
}

QStringList APhotonHistoryLog::getAllProcessNames()
{
    QStringList sl;
    for (int i= 0; i < __SizeOfNodeTypes__; i++)
        sl << getProcessName(i);
    return sl;
}

#include "aphotonsimsettings.h"
bool APhotonHistoryLog::checkComplyWithFilters(const std::vector<APhotonHistoryLog> &PhLog, const APhotonLogSettings & LogSettings)
{
    if (!LogSettings.MustNotInclude_Processes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSettings.MustNotInclude_Processes.find(log.Process) != LogSettings.MustNotInclude_Processes.end() )
                return false;
    }

    if (!LogSettings.MustNotInclude_Volumes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSettings.MustNotInclude_Volumes.find(log.VolumeName) != LogSettings.MustNotInclude_Volumes.end() )
                return false;
    }

    for (size_t im = 0; im < LogSettings.MustInclude_Processes.size(); im++)
    {
        bool bFoundThis = false;
        for (int i = PhLog.size()-1; i > -1; i--)
            if (LogSettings.MustInclude_Processes[im] == PhLog[i].Process)
            {
                bFoundThis = true;
                break;
            }
        if (!bFoundThis) return false;
    }

    for (size_t im = 0; im < LogSettings.MustInclude_Volumes.size(); im++)
    {
        bool bFoundThis = false;
        for (int i = PhLog.size()-1; i > -1; i--)
            if ( LogSettings.MustInclude_Volumes[im].Volume == PhLog[i].VolumeName)
            {
                if (LogSettings.MustInclude_Volumes[im].Index == -1 ||
                    LogSettings.MustInclude_Volumes[im].Index == PhLog[i].VolumeIndex)
                {
                    bFoundThis = true;
                    break;
                }
            }
        if (!bFoundThis) return false;
    }

    return true;
}


