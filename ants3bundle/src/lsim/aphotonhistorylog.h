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
    enum EProcessType {
            Undefined = 0,//keep the first one zero!!!

            Created, HitSensor, Detected, NotDetected, Escaped, Absorbed, MaxNumberCyclesReached, Rayleigh,
            Reemission, Fresnel_Reflection, Fresnel_Transmition, Override_Loss, Override_Forward, Override_Back, GeneratedOutsideGeometry,
            Grid_Enter, Grid_Exit, Grid_ShiftIn, Grid_ShiftOut, KilledByMonitor, Functional_In, Functional_Out, Functional_Kill,

            __SizeOfNodeTypes__//keep it last!!!
        };
    // MUST be synchronized with the enum above!
    static constexpr std::array<const char *, __SizeOfNodeTypes__> AllProcessNames
        {
            "Undefined",

            "Created", "HitSensor", "Detected", "NotDetected", "Escaped", "Absorbed", "MaxNumberCyclesReached", "Rayleigh",
            "Reemission", "Fresnel_Reflection", "Fresnel_Transmition", "Override_Loss", "Override_Forward", "Override_Back", "GeneratedOutsideGeometry",
            "Grid_Enter", "Grid_Exit", "Grid_ShiftIn", "Grid_ShiftOut", "KilledByMonitor", "Functional_In", "Functional_Out", "Functional_Kill",
        };

public:
    APhotonHistoryLog(const double * position, const TString & volumeName, int volumeIndex,
                      double time, int iWave, EProcessType process, int iMatIndex = -1, int iMatIndexAfter = -1, int number = -1);
    APhotonHistoryLog() : Process(Undefined), Wave(-1) {}

    EProcessType Process;
    double       Position[3];    // xyz [mm]
    TString      VolumeName;
    int          VolumeIndex;
    double       Time;           // ns
    int          MatIndex;       // material index of the medium
    int          MatIndexAfter;  // material index of the medium after interface (if applicable)
    int          Number;         // if node type is HitPM/Detected/NotDetected -> contains PM number  !!!*** obsolete?
    int          Wave;           // photon wave index

    QString printToString() const;

    void    sendToStream(QTextStream * s) const;
    QString parseFromString(const QString & str); // returns empty string if no error, otherwise the error string

    // --- static methods ---
    static QString     getProcessName(int nodeType);
    static int         getProcessIndex(const QString & name); // returns -1 if not found
    static QStringList getAllProcessNames();
    static bool        checkComplyWithFilters(const std::vector<APhotonHistoryLog> & PhLog, const APhotonLogSettings & LogSettings);

};

#endif // APHOTONHISTORYLOG_H
