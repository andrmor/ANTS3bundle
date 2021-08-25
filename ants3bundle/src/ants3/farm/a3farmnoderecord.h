#ifndef A3FARMNODERECORD_H
#define A3FARMNODERECORD_H

#include <QString>

#include <vector>

class A3FarmNodeRecord
{
public:
    A3FarmNodeRecord(QString Address, int Port, int Cores) : Address(Address), Port(Port), Processes(Cores) {}
    A3FarmNodeRecord(){}

    enum EStatus {Unknown, Connecting, Available, NotResponding, Busy};

    QString Name        = "NotDefined";
    QString Address     = "127.0.0.1";
    int     Port        = 12345;
    int     Processes   = 1;
    double  SpeedFactor = 1.0;
    bool    Enabled     = true;
    EStatus Status      = Unknown;

    //runtime
    bool    Checked     = false;
    std::vector<int> Split;
};

#endif // A3FARMNODERECORD_H
