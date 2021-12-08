#ifndef ASENSORMODEL_H
#define ASENSORMODEL_H

#include <QString>
#include <vector>

class QJsonObject;

class ASensorModel
{
public:
    ASensorModel(const QString & Name = "NoName") : Name(Name) {}

    QString Name;

    bool    SiPM = false;
    int     PixelsX = 50;
    int     PixelsY = 50;

    double  PDE_effective = 1.0;
    bool    WaveSensitive = false;
    std::vector<std::pair<double,double>> PDE_spectral;

    bool    AngleSensitive = false;
    std::vector<std::pair<double,double>> AngularFactors;
    double  InterfaceN = 1.0; // refractive index of the interfacing medium where sensor object was positioned to measure the angular response

    bool    XYSensitive = false;
    std::vector<std::vector<double>> XYFactors;
    double  StepX = 1.0;   // in mm
    double  StepY = 1.0;

    double  DarkCountRate = 0; //per ns

    void    clear();

    void    writeToJson(QJsonObject & json) const;  // !!! ***
    void    readFromJson(const QJsonObject & json); // !!!***

    //runtime
    std::vector<double> PDEbinned;
    std::vector<double> AngularSensitivityCosRefracted; // response vs cos of refracted beam, spanning from 0 to 1, in CosBins bins

};

#endif // ASENSORMODEL_H
