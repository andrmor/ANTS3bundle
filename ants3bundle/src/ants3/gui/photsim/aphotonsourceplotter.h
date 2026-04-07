#ifndef APHOTONSOURCEPLOTTER_H
#define APHOTONSOURCEPLOTTER_H

class AGeoMarkerClass;
class TString;
class TVirtualGeoTrack;

class APhotonSourcePlotter
{
public:
    static AGeoMarkerClass * plotSource(int defaultSizeDelta);

    static constexpr double pi = 3.14159265358979323846;

private:
    static bool isInsideLimitingMaterial(const double * r, int LimitToMaterial);
    static bool isInsideLimitingVolume(const double * r, const TString & LimitToVolume);
    static TVirtualGeoTrack * createTrack();

    static void formCircle(double radius, double x0, double y0, double z);
};

#endif // APHOTONSOURCEPLOTTER_H
