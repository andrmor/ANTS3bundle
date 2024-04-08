#include "aphotonhistorylog.h"
#include "amaterialhub.h"

#include <QTextStream>
#include <QDebug>

APhotonHistoryLog::APhotonHistoryLog(const double * Position, const TString & volumeName, int volumeIndex,
                                     double Time,
                                     int iWave,
                                     APhotonHistoryLog::NodeType node,
                                     int MatIndex, int MatIndexAfter,
                                     int number) :
    process(node), volumeName(volumeName), VolumeIndex(volumeIndex), time(Time),
    matIndex(MatIndex), matIndexAfter(MatIndexAfter), number(number), iWave(iWave)
{
    r[0] = Position[0];
    r[1] = Position[1];
    r[2] = Position[2];
}

QString APhotonHistoryLog::print() const
{
    QString s;

    s += GetProcessName(process);

    if      (process == HitSensor || process == Detected || process == NotDetected) s += " PM# " + QString::number(number);
    else if (process == Fresnel_Reflection || process == Override_Loss || process == Override_Back || process == Fresnel_Transmition || process == Override_Forward)
    {
        const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
        s += " [" + MatHub.getMaterialName(matIndex) +"/"+ MatHub.getMaterialName(matIndexAfter)+"]";
    }

    s += QString(" at (") + QString::number(r[0])+","+ QString::number(r[1]) + ","+QString::number(r[2])+")";
    if (volumeName.Length() != 0) s += " in " + QString(volumeName) + "#" + QString::number(VolumeIndex);
    if (iWave != -1) s += " iWave="+QString::number(iWave);
    s += " " + QString::number(time)+" ns";

    return s;
}

void APhotonHistoryLog::sendToStream(QTextStream * s) const
{
    *s << process << " ";           // 0
    for (size_t i = 0; i < 3; i++)
        *s << r[i] << " ";          // 1 2 3
    *s << volumeName.Data() << " "; // 4
    *s << VolumeIndex << " ";       // 5
    *s << time << " ";              // 6
    *s << iWave << " ";             // 7
    *s << matIndex << " ";          // 8
    *s << matIndexAfter << " ";     // 9
    *s << number << "\n";           // 10
}

QString APhotonHistoryLog::parseFromString(const QString & str)
{
    const QStringList sl = str.split(' ', Qt::SkipEmptyParts);
    if (sl.size() != 11) return "Wrong size of photon log record";

    bool ok;
    int iPr = sl[0].toInt(&ok);
    if (!ok || iPr < 0 || iPr >= __SizeOfNodeTypes__) return "Invalid process index";
    process = static_cast<NodeType>(iPr);

    for (int i = 0; i < 3; i++)
    {
        r[i] = sl[1+i].toDouble(&ok);
        if (!ok) return "Invalid position";
    }

    volumeName = TString(sl[4].toLatin1().data());

    VolumeIndex = sl[5].toInt(&ok);
    if (!ok) return "Invalid volume index";

    time = sl[6].toDouble(&ok);
    if (!ok) return "Invalid time";

    iWave = sl[7].toInt(&ok);
    if (!ok) return "Invalid wave index";

    matIndex = sl[8].toInt(&ok);
    if (!ok) return "Invalid materail index";

    matIndexAfter = sl[9].toInt(&ok);
    if (!ok) return "Invalid second materail index";

    number = sl[10].toInt(&ok);
    if (!ok) return "Invalid numeric parameter";

    return "";
}

QString APhotonHistoryLog::GetProcessName(int nodeType)
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

int APhotonHistoryLog::GetProcessIndex(const QString & name)
{
    for (int i = 0; i < AllProcessNames.size(); i++)
        if (name == AllProcessNames[i]) return i;
    return -1;
}

QString APhotonHistoryLog::PrintAllProcessTypes()
{
    QString s = "<br>Defined types:<br>";
    for (int i=0; i<__SizeOfNodeTypes__; i++)
        s += QString::number(i) + " -> " + GetProcessName(i) + "<br>";
    return s;
}

#include "aphotonsimsettings.h"
bool APhotonHistoryLog::CheckComplyWithFilters(const std::vector<APhotonHistoryLog> &PhLog, const APhotonLogSettings & LogSettings)
{
    if (!LogSettings.MustNotInclude_Processes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSettings.MustNotInclude_Processes.find(log.process) != LogSettings.MustNotInclude_Processes.end() )
                return false;
    }

    if (!LogSettings.MustNotInclude_Volumes.empty())
    {
        for (const APhotonHistoryLog & log : PhLog)
            if ( LogSettings.MustNotInclude_Volumes.find(log.volumeName) != LogSettings.MustNotInclude_Volumes.end() )
                return false;
    }

    for (size_t im = 0; im < LogSettings.MustInclude_Processes.size(); im++)
    {
        bool bFoundThis = false;
        for (int i = PhLog.size()-1; i > -1; i--)
            if (LogSettings.MustInclude_Processes[im] == PhLog[i].process)
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
            if ( LogSettings.MustInclude_Volumes[im].Volume == PhLog[i].volumeName)
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


