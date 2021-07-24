#ifndef APHOTSIMSETTINGS_H
#define APHOTSIMSETTINGS_H

enum class APhotSinTypeEnum {PhotonBombs, FromEnergyDepo, IndividualPhotons, FromLRFs};

class AWaveResSettings
{
public:
    bool Enabled = false;

    double From = 200.0;  // in nm
    double To   = 800.0;
    double Step = 5.0;

    int    countNodes() const;
    double getWavelength(int index) const;
    int    getIndex(double wavelength) const;
};

class APhotSimSettings
{
public:
    APhotSimSettings();

    APhotSinTypeEnum SimType = APhotSinTypeEnum::PhotonBombs;

    AWaveResSettings WaveSet;


};



#endif // APHOTSIMSETTINGS_H
