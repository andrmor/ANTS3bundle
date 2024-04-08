#ifndef APHOTONHISTORYLOG_H
#define APHOTONHISTORYLOG_H

#include <QString>

#include <array>
#include <vector>

#include "TString.h"

class QTextStream;
class APhotonLogSettings;

class APhotonHistoryLog
{
public:
    enum NodeType {
            Undefined = 0,//keep the first one zero!!!

            Created, HitSensor, Detected, NotDetected, Escaped, Absorbed, MaxNumberCyclesReached, Rayleigh,
            Reemission, Fresnel_Reflection, Fresnel_Transmition, Override_Loss, Override_Forward, Override_Back, GeneratedOutsideGeometry,
            Grid_Enter, Grid_Exit, Grid_ShiftIn, Grid_ShiftOut, KilledByMonitor,

            __SizeOfNodeTypes__//keep it last!!!
        };
    // MUST be synchronized with the enum above!
    static constexpr std::array<const char *, __SizeOfNodeTypes__> AllProcessNames
        {
            "Undefined",

            "Created", "HitSensor", "Detected", "NotDetected", "Escaped", "Absorbed", "MaxNumberCyclesReached", "Rayleigh",
            "Reemission", "Fresnel_Reflection", "Fresnel_Transmition", "Override_Loss", "Override_Forward", "Override_Back", "GeneratedOutsideGeometry",
            "Grid_Enter", "Grid_Exit", "Grid_ShiftIn", "Grid_ShiftOut", "KilledByMonitor"
        };

public:
    APhotonHistoryLog(const double * Position, const TString & volumeName, int volumeIndex, double Time, int iWave, NodeType process, int MatIndex = -1, int MatIndexAfter = -1, int number = -1);
    APhotonHistoryLog() : process(Undefined), iWave(-1) {}

    NodeType process;
    double   r[3];           //position xyz
    TString  volumeName;
    int      volumeIndex;
    double   time;
    int      matIndex;       //material index of the medium
    int      matIndexAfter;  //material index of the medium after interface (if applicable)
    int      number;         //if node type is HitPM/Detected/NotDetected -> contains PM number
    int      iWave;          //photon wave index

    QString print() const;

    void    sendToStream(QTextStream * s) const;
    QString parseFromString(const QString & str); // returns empty string if no error, otherwise the error string

    // --- static methods ---
    static QString     getProcessName(int nodeType);
    static int         getProcessIndex(const QString & name); // returns -1 if not found
    static QStringList getAllProcessNames();
    static bool        checkComplyWithFilters(const std::vector<APhotonHistoryLog> & PhLog, const APhotonLogSettings & LogSettings);

};

#endif // APHOTONHISTORYLOG_H
