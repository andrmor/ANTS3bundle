#ifndef APHOTONSOURCEPLOTTER_H
#define APHOTONSOURCEPLOTTER_H

class AGeoMarkerClass;
class TString;
class TVirtualGeoTrack;

class APhotonSourcePlotter
{
public:
    static AGeoMarkerClass * plotSource();

    static constexpr double pi = 3.14159265358979323846;

private:
    static bool isInsideLimitingMaterial(const double * r, int LimitToMaterial);
    static bool isInsideLimitingVolume(const double * r, const TString & LimitToVolume);
    static TVirtualGeoTrack * createTrack();

};

#endif // APHOTONSOURCEPLOTTER_H
