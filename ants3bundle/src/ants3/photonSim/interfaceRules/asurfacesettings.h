#ifndef ASURFACESETTINGS_H
#define ASURFACESETTINGS_H

class QJsonObject;

class ASurfaceSettings
{
public:
    enum EModel {Polished, Glisur, Unified};

    ASurfaceSettings(){}

    bool isPolished()    const {return Model == Polished;}
    bool isNotPolished() const {return Model != Polished;}

    EModel Model = Glisur;

    //Glisur model settings
    double Polish = 0.85;

    //Unified model settings
    double SigmaAlpha = 0.1;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // ASURFACESETTINGS_H
