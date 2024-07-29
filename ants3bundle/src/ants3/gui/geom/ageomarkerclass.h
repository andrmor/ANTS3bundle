#ifndef AGEOMARKERCLASS
#define AGEOMARKERCLASS

#include "TPolyMarker3D.h"

class AGeoMarkerClass : public TPolyMarker3D
{
public:
    enum EType {Undefined, True, Recon, Source};

    AGeoMarkerClass(EType type, int style, int size, int color) {configure(type, style, size, color);}
    AGeoMarkerClass() {}

    void configure(EType type, int style, int size, int color) {Type = type; SetMarkerStyle(style); SetMarkerSize(size); SetMarkerColor(color);}

    EType Type = Undefined;
};

#endif // AGEOMARKERCLASS

