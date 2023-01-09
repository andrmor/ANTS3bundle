#ifndef AFARMNODERECORD_H
#define AFARMNODERECORD_H

#include <QString>

#include <vector>

class QJsonObject;

class AFarmNodeRecord
{
public:
    AFarmNodeRecord(QString Address, int Port, int Cores) : Address(Address), Port(Port), Processes(Cores) {}
    AFarmNodeRecord(){}

    enum EStatus {Unknown, Connecting, Available, NotResponding, Busy};

    QString Name        = "NotDefined";
    QString Address     = "127.0.0.1";
    int     Port        = 12345;
    int     Processes   = 1;
    double  SpeedFactor = 1.0;
    bool    Enabled     = true;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    //runtime
    EStatus Status      = Unknown;
    bool    Checked     = false;
    std::vector<int> Split;
};

#endif // AFARMNODERECORD_H
