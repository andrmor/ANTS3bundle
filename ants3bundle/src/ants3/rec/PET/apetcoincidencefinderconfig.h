#ifndef APETCOINCIDENCEFINDERCONFIG_H
#define APETCOINCIDENCEFINDERCONFIG_H

class APetCoincidenceFinderConfig
{
public:
    double CoincidenceWindow = 4.0; // [ns]

    // consider events only in this time range [ns]
    double TimeFrom          = 0;
    double TimeTo            = 1e20;

    // consider events only in this energy range [MeV]
    double EnergyFrom     = 0.95 * 511.0;
    double EnergyTo       = 1.05 * 511.0;

};

#endif // APETCOINCIDENCEFINDERCONFIG_H
