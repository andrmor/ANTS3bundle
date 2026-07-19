#include "apersistentutils3d.h"

#include <QDebug>

void APersistentPolymarker3D::EnforcePersistence()
{
    ResetBit(kCanDelete);
}

APersistentPolymarker3D::APersistentPolymarker3D() : TPolyMarker3D() { EnforcePersistence();}

APersistentPolymarker3D::APersistentPolymarker3D(Int_t n, Marker_t marker, Option_t* option)
    : TPolyMarker3D(n, marker, option) { EnforcePersistence(); }

APersistentPolymarker3D::APersistentPolymarker3D(Int_t n, Float_t* p, Marker_t marker, Option_t* option)
    : TPolyMarker3D(n, p, marker, option) { EnforcePersistence(); }

APersistentPolymarker3D::APersistentPolymarker3D(Int_t n, Double_t* p, Marker_t marker, Option_t* option)
    : TPolyMarker3D(n, p, marker, option) { EnforcePersistence(); }

APersistentPolymarker3D::APersistentPolymarker3D(const APersistentPolymarker3D & polymarker)
    : TPolyMarker3D(polymarker) { EnforcePersistence(); }

APersistentPolymarker3D::~APersistentPolymarker3D()
{
    qDebug() << "----> destr for APersistentPolymarker3D";
}

TObject * APersistentPolymarker3D::Clone(const char *) const
{
    return new APersistentPolymarker3D(*this);
}

// ---

void APersistentPolyLine3D::EnforcePersistence()
{
    ResetBit(kCanDelete);
}

APersistentPolyLine3D::APersistentPolyLine3D() : TPolyLine3D() { EnforcePersistence(); }

APersistentPolyLine3D::APersistentPolyLine3D(Int_t n, Option_t* option)
    : TPolyLine3D(n, option) { EnforcePersistence(); }

APersistentPolyLine3D::APersistentPolyLine3D(Int_t n, Float_t* p, Option_t* option)
    : TPolyLine3D(n, p, option) { EnforcePersistence(); }

APersistentPolyLine3D::APersistentPolyLine3D(Int_t n, Double_t* p, Option_t* option)
    : TPolyLine3D(n, p, option) { EnforcePersistence(); }

APersistentPolyLine3D::APersistentPolyLine3D(const APersistentPolyLine3D& polyline)
    : TPolyLine3D(polyline) { EnforcePersistence(); }

APersistentPolyLine3D::~APersistentPolyLine3D()
{
    qDebug() << "--++--> destr for APersistentPolyLine3D";
}

TObject * APersistentPolyLine3D::Clone(const char *) const
{
    return new APersistentPolyLine3D(*this);
}
