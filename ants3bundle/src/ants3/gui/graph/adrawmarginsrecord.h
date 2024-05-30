#ifndef ADRAWMARGINSRECORD_H
#define ADRAWMARGINSRECORD_H

class QJsonObject;

class ADrawMarginsRecord
{
public:
    bool Override    = false;

    double Top       = 0.05;
    double Bottom    = 0.1;
    double Left      = 0.1;
    double Right     = 0.1;

    double RightForZ = 0.15;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // ADRAWMARGINSRECORD_H
