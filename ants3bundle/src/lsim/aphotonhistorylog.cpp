#include "aphotonhistorylog.h"
#include "amaterialhub.h"

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

    s += QString(" at ( ") + QString::number(r[0])+", "+ QString::number(r[1]) + ", "+QString::number(r[2])+" )";
    if (volumeName.Length() != 0) s += " in " + QString(volumeName) + " with index " + QString::number(VolumeIndex);
    if (iWave != -1) s += " iWave="+QString::number(iWave);
    s += ", " + QString::number(time)+" ns";

    return s;
}

#include <QTextStream>
void APhotonHistoryLog::sendToStream(QTextStream * s) const
{
    *s << process << " ";
    for (size_t i = 0; i < 3; i++)
        *s << r[i] << " ";
    *s << volumeName.Data() << " ";
    *s << VolumeIndex << " ";
    *s << time << " ";
    *s << matIndex << " ";
    *s << matIndexAfter << " ";
    *s << number << " ";
    *s << iWave << "\n";
}

QString APhotonHistoryLog::GetProcessName(int nodeType)
{
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
}

QString APhotonHistoryLog::PrintAllProcessTypes()
{
    QString s = "<br>Defined types:<br>";
    for (int i=0; i<__SizeOfNodeTypes__; i++)
        s += QString::number(i) + " -> " + GetProcessName(i) + "<br>";
    return s;
}
