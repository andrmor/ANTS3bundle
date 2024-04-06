#ifndef APHOTONHISTORYLOG_H
#define APHOTONHISTORYLOG_H

#include <QString>
#include "TString.h"

class QTextStream;

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

public:
    APhotonHistoryLog(const double * Position, const TString & volumeName, int volumeIndex, double Time, int iWave, NodeType process, int MatIndex = -1, int MatIndexAfter = -1, int number = -1);
    APhotonHistoryLog() : process(Undefined), iWave(-1) {}

    NodeType process;
    double   r[3];        //position xyz
    TString  volumeName;
    int      VolumeIndex;
    double   time;
    int      matIndex;       //material index of the medium
    int      matIndexAfter;  //material index of the medium after interface (if applicable)
    int      number;         //if node type is HitPM/Detected/NotDetected -> contains PM number
    int      iWave;          //photon wave index

    QString print() const;

    void sendToStream(QTextStream * s) const;
    QString parseFromString(const QString & str); // returns empty string if no error, otherwise the error string

    static QString GetProcessName(int nodeType);
    static QString PrintAllProcessTypes();
};

#endif // APHOTONHISTORYLOG_H
