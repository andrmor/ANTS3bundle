#ifndef AGEOMARKERCLASS
#define AGEOMARKERCLASS

#include "TPolyMarker3D.h"

enum class EGeoMarkerType {Undefined, PrimarySource, PointOfOrigin, PosTrue, PosReconstructed};

class AGeoMarkerClass : public TPolyMarker3D
{
public:
    //enum EType {Undefined, True, Recon, PointOfOrigin, PrimarySource};

    AGeoMarkerClass(EGeoMarkerType type, int style, int size, int color) {configure(type, style, size, color);}
    AGeoMarkerClass() {}

    void configure(EGeoMarkerType type, int style, int size, int color) {Type = type; SetMarkerStyle(style); SetMarkerSize(size); SetMarkerColor(color);}

    EGeoMarkerType Type = EGeoMarkerType::Undefined;
};

#endif // AGEOMARKERCLASS

