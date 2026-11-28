#include "aphotonsourceplotter.h"
#include "aphotonsimhub.h"
#include "aphotonsimsettings.h"
#include "ageomarkerclass.h"
#include "ageometryhub.h"
#include "amaterialhub.h"

#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TVector3.h"

#include <QDebug>

bool APhotonSourcePlotter::isInsideLimitingMaterial(const double * r, int LimitToMaterial)
{
    TGeoNode * node = AGeometryHub::getInstance().GeoManager->FindNode(r[0], r[1], r[2]);
    if (!node) return false;
    return (node->GetVolume() && node->GetVolume()->GetMaterial()->GetIndex() == LimitToMaterial);
}

bool APhotonSourcePlotter::isInsideLimitingVolume(const double * r, const TString & LimitToVolume)
{
    TGeoNode * node = AGeometryHub::getInstance().GeoManager->FindNode(r[0], r[1], r[2]);
    if (!node) return false;
    return (node->GetVolume() && node->GetVolume()->GetName() == LimitToVolume);
}

TVirtualGeoTrack *APhotonSourcePlotter::createTrack(int lineWidth)
{
    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    Int_t track_index = GeoManager->AddTrack(1,22);
    TVirtualGeoTrack * track = GeoManager->GetTrack(track_index);
    track->SetLineWidth(lineWidth);
    track->SetLineColor(51);
    return track;
}

// local (u, v, a) -> global, where a is along the axis and (u, v) are the two other axes in cyclic order
static void floodLocalToGlobal(const double * center, int axis, double u, double v, double a, double * r)
{
    const int i1 = (axis + 1) % 3;
    const int i2 = (axis + 2) % 3;
    r[axis] = center[axis] + a;
    r[i1]   = center[i1] + u;
    r[i2]   = center[i2] + v;
}

void APhotonSourcePlotter::formCircle(double radius, const double * center, int axis, double axialOffset, int lineWidth)
{
    TVirtualGeoTrack * track = createTrack(lineWidth);
    double r[3];
    for (int i = 0; i < 51; i++)
    {
        floodLocalToGlobal(center, axis, radius * cos(pi / 25.0 * i), radius * sin(pi / 25.0 * i), axialOffset, r);
        track->AddPoint(r[0], r[1], r[2], 0);
    }
}

void APhotonSourcePlotter::formRectangle(const double * center, const double * halfSizes, int axis, double axialOffset, int lineWidth)
{
    const int i1 = (axis + 1) % 3;
    const int i2 = (axis + 2) % 3;
    const double hu = halfSizes[i1];
    const double hv = halfSizes[i2];
    const double corners[5][2] = { {-hu, -hv}, {hu, -hv}, {hu, hv}, {-hu, hv}, {-hu, -hv} };

    TVirtualGeoTrack * track = createTrack(lineWidth);
    double r[3];
    for (int i = 0; i < 5; i++)
    {
        floodLocalToGlobal(center, axis, corners[i][0], corners[i][1], axialOffset, r);
        track->AddPoint(r[0], r[1], r[2], 0);
    }
}

void APhotonSourcePlotter::plotFlood()
{
    const AFloodSettings & fs = APhotonSimHub::getConstInstance().Settings.BombSet.FloodSettings;
    const double center[3] = {fs.X0, fs.Y0, fs.Z0};
    const int    thick = 6;

    switch (fs.Shape)
    {
    case AFloodSettings::Box :
    {
        const double half[3] = {0.5 * fs.SizeX, 0.5 * fs.SizeY, 0.5 * fs.SizeZ};
        // two faces perpendicular to Z plus the four vertical edges (a single rectangle if SizeZ = 0)
        formRectangle(center, half, 2, -half[2]);
        if (half[2] > 0)
        {
            formRectangle(center, half, 2, half[2]);
            for (int sx = -1; sx <= 1; sx += 2)
                for (int sy = -1; sy <= 1; sy += 2)
                {
                    TVirtualGeoTrack * track = createTrack();
                    track->AddPoint(center[0] + sx * half[0], center[1] + sy * half[1], center[2] - half[2], 0);
                    track->AddPoint(center[0] + sx * half[0], center[1] + sy * half[1], center[2] + half[2], 0);
                }
        }
        // entry faces of exponential profiles
        const AFloodProfile * profiles[3] = {&fs.ProfileX, &fs.ProfileY, &fs.ProfileZ};
        for (int axis = 0; axis < 3; axis++)
        {
            const AFloodProfile & p = *profiles[axis];
            if (p.Type != AFloodProfile::Exponential || half[axis] == 0) continue;
            const double offset = (p.Direction == AFloodProfile::Plus ? -half[axis] : half[axis]);
            formRectangle(center, half, axis, offset, thick);
        }
        break;
    }
    case AFloodSettings::Cylinder :
    {
        const int    axis  = fs.Axis;
        const double halfL = 0.5 * fs.Length;
        const double rOut  = 0.5 * fs.OuterDiameter;
        const double rIn   = 0.5 * fs.InnerDiameter;
        for (double offset : {-halfL, halfL})
        {
            formCircle(rOut, center, axis, offset);
            if (rIn > 0) formCircle(rIn, center, axis, offset);
            if (halfL == 0) break;
        }
        if (fs.ProfileAxial.Type == AFloodProfile::Exponential && halfL > 0)
        {
            const double offset = (fs.ProfileAxial.Direction == AFloodProfile::Plus ? -halfL : halfL);
            formCircle(rOut, center, axis, offset, thick);
            if (rIn > 0) formCircle(rIn, center, axis, offset, thick);
        }
        break;
    }
    case AFloodSettings::Sphere :
    {
        for (int axis = 0; axis < 3; axis++)
        {
            formCircle(0.5 * fs.OuterDiameter, center, axis, 0);
            if (fs.InnerDiameter > 0) formCircle(0.5 * fs.InnerDiameter, center, axis, 0);
        }
        break;
    }
    }
}

AGeoMarkerClass * APhotonSourcePlotter::plotSource()
{
    APhotonBombsSettings & bombSet = APhotonSimHub::getInstance().Settings.BombSet;
    switch (bombSet.GenerationMode)
    {
    case EBombGen::Single:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(EGeoMarkerType::PrimarySource, 3, 3, 51);
        marks->SetNextPoint(bombSet.SingleSettings.Position[0], bombSet.SingleSettings.Position[1], bombSet.SingleSettings.Position[2]);
        return marks;
    }
    case EBombGen::Grid:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(EGeoMarkerType::PrimarySource, 3, 3, 51);
        const AGridSettings & ScanSet = bombSet.GridSettings;
        const APhotonBombAdvancedSettings & AdvSet = bombSet.AdvancedSettings;

        TString LimitToVolume;
        int LimitToMaterial;
        if (AdvSet.bOnlyVolume)   LimitToVolume  = TString(AdvSet.Volume.toLatin1().data());
        if (AdvSet.bOnlyMaterial) LimitToMaterial = AMaterialHub::getConstInstance().findMaterial(AdvSet.Material);

        double RegGridOrigin[3];
        RegGridOrigin[0] = ScanSet.X0;
        RegGridOrigin[1] = ScanSet.Y0;
        RegGridOrigin[2] = ScanSet.Z0;

        double RegGridStep[3][3]; //vector [axis] [step]
        int    RegGridNodes[3];
        bool   RegGridFlagPositive[3];

        for (int ic = 0; ic < 3; ic++)
        {
            const APhScanRecord & rec = ScanSet.ScanRecords[ic];
            if (rec.bEnabled)
            {
                RegGridStep[ic][0] = rec.DX;
                RegGridStep[ic][1] = rec.DY;
                RegGridStep[ic][2] = rec.DZ;
                RegGridNodes[ic]   = rec.Nodes;
                RegGridFlagPositive[ic] = rec.bBiDirect;
            }
            else
            {
                RegGridStep[ic][0] = 0;
                RegGridStep[ic][1] = 0;
                RegGridStep[ic][2] = 0;
                RegGridNodes[ic] = 1; //1 is disabled axis
                RegGridFlagPositive[ic] = true;
            }
        }

        int iAxis[3];
        double R[3];
        for (iAxis[0]=0; iAxis[0]<RegGridNodes[0]; iAxis[0]++)
            for (iAxis[1]=0; iAxis[1]<RegGridNodes[1]; iAxis[1]++)
                for (iAxis[2]=0; iAxis[2]<RegGridNodes[2]; iAxis[2]++)  //iAxis - counters along the axes!!!
                {
                    for (int i = 0; i < 3; i++) R[i] = RegGridOrigin[i];
                    //shift from the origin
                    for (int axis = 0; axis < 3; axis++)
                    {
                        double ioffset = 0;
                        if (!RegGridFlagPositive[axis]) ioffset = -0.5*( RegGridNodes[axis] - 1 );
                        for (int i = 0; i < 3; i++) R[i] += (ioffset + iAxis[axis]) * RegGridStep[axis][i];
                    }

                    if (AdvSet.bOnlyVolume   && !isInsideLimitingVolume(R, LimitToVolume))     continue;
                    if (AdvSet.bOnlyMaterial && !isInsideLimitingMaterial(R, LimitToMaterial)) continue;

                    marks->SetNextPoint(R[0], R[1], R[2]);
                }
        return marks;
    }
    case EBombGen::Flood :
        plotFlood();
        return nullptr;
    default: break;
    }
    return nullptr;
}
