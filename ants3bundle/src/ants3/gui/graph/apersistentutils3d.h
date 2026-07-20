#ifndef APERSISTENTUTILS3D_H
#define APERSISTENTUTILS3D_H

#include "TPolyMarker3D.h"
#include "TPolyLine3D.h"

class APersistentPolymarker3D : public TPolyMarker3D
{
public:
    APersistentPolymarker3D();
    APersistentPolymarker3D(Int_t n, Marker_t marker=1, Option_t* option = "");
    APersistentPolymarker3D(Int_t n, Float_t* p, Marker_t marker = 1, Option_t* option = "");
    APersistentPolymarker3D(Int_t n, Double_t* p, Marker_t marker = 1, Option_t* option = "");

    APersistentPolymarker3D(const APersistentPolymarker3D& polymarker);

    virtual ~APersistentPolymarker3D() = default;
    //virtual ~APersistentPolymarker3D();

    virtual TObject* Clone(const char * newname = "") const override;

private:
    void EnforcePersistence();

};

class APersistentPolyLine3D : public TPolyLine3D {
public:
    // Constructors mirroring TPolyLine3D
    APersistentPolyLine3D();
    APersistentPolyLine3D(Int_t n, Option_t* option = "");
    APersistentPolyLine3D(Int_t n, Float_t* p, Option_t* option = "");
    APersistentPolyLine3D(Int_t n, Double_t* p, Option_t* option = "");

    APersistentPolyLine3D(const APersistentPolyLine3D& polyline);

    virtual ~APersistentPolyLine3D() = default;
    //virtual ~APersistentPolyLine3D();

    virtual TObject* Clone(const char * newname = "") const override;

private:
    void EnforcePersistence();

};

#endif // APERSISTENTUTILS3D_H
