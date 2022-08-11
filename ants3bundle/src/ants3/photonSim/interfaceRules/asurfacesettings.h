#ifndef ASURFACESETTINGS_H
#define ASURFACESETTINGS_H

class QJsonObject;

class ASurfaceSettings
{
public:
    enum EModel {Polished, GaussSimplistic};

    ASurfaceSettings(){}

    EModel Model = GaussSimplistic;

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
