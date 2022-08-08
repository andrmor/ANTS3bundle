#ifndef ASURFACESETTINGS_H
#define ASURFACESETTINGS_H

class QJsonObject;

class ASurfaceSettings
{
public:
    enum EModel {Model1, Model2, Model3};

    ASurfaceSettings(){}

    EModel Model = Model1;

    //Model1 settings
    // ...

    //Model2 settings
    // ...

    //Model3 settings
    // ...

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // ASURFACESETTINGS_H
