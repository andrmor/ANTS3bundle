#ifndef A3FARMNODERECORD_H
#define A3FARMNODERECORD_H

#include <QString>

#include <vector>

struct A3FarmNodeRecord
{
    A3FarmNodeRecord(QString Address, int Port, int Cores) : Address(Address), Port(Port), Cores(Cores) {}
    A3FarmNodeRecord(){}

    QString Name        = "Undefined";
    QString Address     = "127.0.0.1";
    int     Port        = 12345;
    int     Cores       = 1;
    double  SpeedFactor = 1.0;
    bool    Enabled     = true;

    //runtime
    bool    Checked     = false;
    std::vector<int> Split;
};

#endif // A3FARMNODERECORD_H
