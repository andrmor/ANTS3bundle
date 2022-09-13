#ifndef ASURFACESETTINGS_H
#define ASURFACESETTINGS_H

class QJsonObject;

class ASurfaceSettings
{
public:
    enum EModel {Polished, GaussSimplistic, Glisur, Unified};

    ASurfaceSettings(){}

    bool isPolished()    const {return Model == Polished;}
    bool isNotPolished() const {return Model != Polished;}

    EModel Model = GaussSimplistic;

    //Model1 settings
    // ...

    //Glisur model settings
    double Polish = 0.5;

    //Model3 settings
    // ...

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // ASURFACESETTINGS_H
