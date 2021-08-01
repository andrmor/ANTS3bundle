#ifndef ASENSORHUB_H
#define ASENSORHUB_H

#include <QString>
#include <QStringList>

#include <vector>

class AGeoObject;
class QJsonObject;

class ASensorModel
{
public:
    QString Name = "Undefined";
    double  PDE  = 1.0;
};

class ASensorHub
{
public:
    static ASensorHub & getInstance();
    static const ASensorHub & getConstInstance();

    const ASensorModel * getModelFast(int iModel) const;

    const QStringList getListOfModelNames() const;

    void writeToJson(QJsonObject & json) const;
    bool readFromJson(const QJsonObject & json);

private:
    ASensorHub();
    ~ASensorHub(){}

    ASensorHub(const ASensorHub&)            = delete;
    ASensorHub(ASensorHub&&)                 = delete;
    ASensorHub& operator=(const ASensorHub&) = delete;
    ASensorHub& operator=(ASensorHub&&)      = delete;

private:
    std::vector<ASensorModel> Models;

    // runtime - populated together with GeoManager
    std::vector<AGeoObject*> Sensors;

};

#endif // ASENSORHUB_H
