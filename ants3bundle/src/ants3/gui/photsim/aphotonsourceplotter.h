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

    static bool isInsideLimitingMaterial(const double * r, int LimitToMaterial);
    static bool isInsideLimitingVolume(const double * r, const TString & LimitToVolume);

private:
    static TVirtualGeoTrack * createTrack(int lineWidth = 3);

    // circle perpendicular to the axis (0=X, 1=Y, 2=Z), centered at center + axialOffset along the axis
    static void formCircle(double radius, const double * center, int axis, double axialOffset, int lineWidth = 3);
    // rectangle perpendicular to the axis at axialOffset; halfSizes are the box half sizes in X, Y, Z
    static void formRectangle(const double * center, const double * halfSizes, int axis, double axialOffset, int lineWidth = 3);

    static void plotFlood();
};

#endif // APHOTONSOURCEPLOTTER_H
