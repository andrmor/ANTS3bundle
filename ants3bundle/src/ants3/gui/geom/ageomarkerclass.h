#ifndef AGEOMARKERCLASS
#define AGEOMARKERCLASS

#include "TPolyMarker3D.h"

class GeoMarkerClass : public TPolyMarker3D
{
public:
    enum EType {Undefined, True, Recon, Source};

    EType Type = Undefined;
    GeoMarkerClass(EType type, int style, int size, int color) {configure(type, style, size, color);}
    GeoMarkerClass() {}
    void configure(EType type, int style, int size, int color) {Type = type; SetMarkerStyle(style); SetMarkerSize(size); SetMarkerColor(color);}
};

#endif // AGEOMARKERCLASS

