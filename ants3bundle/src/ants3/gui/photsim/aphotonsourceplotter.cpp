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

TVirtualGeoTrack *APhotonSourcePlotter::createTrack()
{
    TGeoManager * gGeoManager = AGeometryHub::getInstance().GeoManager;
    Int_t track_index = gGeoManager->AddTrack(1,22);
    TVirtualGeoTrack * track = gGeoManager->GetTrack(track_index);
    track->SetLineWidth(3);
    track->SetLineColor(51);
    return track;
}

void APhotonSourcePlotter::formCircle(double radius, double x0, double y0, double z)
{
    TVirtualGeoTrack * track = createTrack();
    for (int i = 0; i < 51; i++)
    {
        double x = radius * cos(pi / 25.0 * i);
        double y = radius * sin(pi / 25.0 * i);
        track->AddPoint(x0 + x, y0 + y, z, 0);
    }
}

AGeoMarkerClass * APhotonSourcePlotter::plotSource()
{
    APhotonBombsSettings & bombSet = APhotonSimHub::getInstance().Settings.BombSet;
    switch (bombSet.GenerationMode)
    {
    case EBombGen::Single:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(AGeoMarkerClass::Source, 3, 3, 51);
        marks->SetNextPoint(bombSet.SingleSettings.Position[0], bombSet.SingleSettings.Position[1], bombSet.SingleSettings.Position[2]);
        return marks;
    }
    case EBombGen::Grid:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(AGeoMarkerClass::Source, 3, 3, 51);
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
    {
        const AFloodSettings & fs = bombSet.FloodSettings;
        switch (fs.Shape)
        {
        case (AFloodSettings::Rectangular):
        {
            TVirtualGeoTrack * track = createTrack();
            if (fs.Zmode == AFloodSettings::Fixed)
            {
                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zfixed, 0);
                track->AddPoint(fs.Xfrom, fs.Yto,   fs.Zfixed, 0);
                track->AddPoint(fs.Xto,   fs.Yto,   fs.Zfixed, 0);
                track->AddPoint(fs.Xto,   fs.Yfrom, fs.Zfixed, 0);
                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zfixed, 0);
            }
            else
            {
                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zfrom, 0);
                track->AddPoint(fs.Xfrom, fs.Yto,   fs.Zfrom, 0);
                track->AddPoint(fs.Xto,   fs.Yto,   fs.Zfrom, 0);
                track->AddPoint(fs.Xto,   fs.Yfrom, fs.Zfrom, 0);
                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zfrom, 0);

                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zto, 0);
                track->AddPoint(fs.Xfrom, fs.Yto,   fs.Zto, 0);
                    track->AddPoint(fs.Xfrom, fs.Yto,   fs.Zfrom, 0);
                    track->AddPoint(fs.Xfrom, fs.Yto,   fs.Zto, 0);
                track->AddPoint(fs.Xto,   fs.Yto,   fs.Zto, 0);
                    track->AddPoint(fs.Xto,   fs.Yto,   fs.Zfrom, 0);
                    track->AddPoint(fs.Xto,   fs.Yto,   fs.Zto, 0);
                track->AddPoint(fs.Xto,   fs.Yfrom, fs.Zto, 0);
                    track->AddPoint(fs.Xto,   fs.Yfrom, fs.Zfrom, 0);
                    track->AddPoint(fs.Xto,   fs.Yfrom, fs.Zto, 0);
                track->AddPoint(fs.Xfrom, fs.Yfrom, fs.Zto, 0);
            }
            break;
        }
        case (AFloodSettings::Ring):
        {
            if (fs.Zmode == AFloodSettings::Fixed)
            {
                formCircle(0.5 * fs.OuterDiameter, fs.X0, fs.Y0, fs.Zfixed);
                if (fs.InnerDiameter != 0)
                    formCircle(0.5 * fs.InnerDiameter, fs.X0, fs.Y0, fs.Zfixed);
            }
            else
            {
                formCircle(0.5 * fs.OuterDiameter, fs.X0, fs.Y0, fs.Zfrom);
                formCircle(0.5 * fs.OuterDiameter, fs.X0, fs.Y0, fs.Zto);
                if (fs.InnerDiameter != 0)
                {
                    formCircle(0.5 * fs.InnerDiameter, fs.X0, fs.Y0, fs.Zfrom);
                    formCircle(0.5 * fs.InnerDiameter, fs.X0, fs.Y0, fs.Zto);
                }
            }
            break;
        }
        }

    }
    default: break;
    }
    return nullptr;
}
