#ifndef ASENSORMODEL_H
#define ASENSORMODEL_H

#include <QString>
#include <vector>

class QJsonObject;

class ASensorModel
{
public:
    ASensorModel(const QString & Name = "") : Name(Name) {}

    QString Name;
    double  DarkCountRate = 3.0e5; //per ns

    bool    SiPM = false;
    int     PixelsX = 50;
    int     PixelsY = 50;

    double  EffectivePDE = 1.0;
    bool    EnableWavelengthSensitivity = false; // !!!***
    std::vector<std::pair<double,double>> PDE;

    bool    EnableAngularSensitivity = false; // !!***
    std::vector<std::pair<double,double>> AngularFactors;
    double  AngularN1 = 1.0;                        // refractive index of the medium where PM was positioned to measure the angular response

    bool    EnableSpatialSensitivity = false;  // !!!***
    std::vector<std::vector<double>> SpatialFactors;
    double  StepX = 1.0;   // in mm
    double  StepY = 1.0;

    void    clear();

    void    writeToJson(QJsonObject &json) const;  // !!! ***
    void    readFromJson(const QJsonObject &json); // !!!***

    //runtime
    std::vector<double> PDEbinned;
    std::vector<double> AngularSensitivityCosRefracted; // response vs cos of refracted beam, binned from 0 to 1 in CosBins bins

};


#endif // ASENSORMODEL_H
