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

AGeoMarkerClass * APhotonSourcePlotter::plotSource()
{
    APhotonBombsSettings & simSet = APhotonSimHub::getInstance().Settings.BombSet;
    switch (simSet.GenerationMode)
    {
    case EBombGen::Single:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(AGeoMarkerClass::Source, 3, 10, kBlue);
        marks->SetNextPoint(simSet.SingleSettings.Position[0], simSet.SingleSettings.Position[1], simSet.SingleSettings.Position[2]);
        return marks;
    }
    case EBombGen::Grid:
    {
        AGeoMarkerClass * marks = new AGeoMarkerClass(AGeoMarkerClass::Source, 3, 10, kBlue);
        const AGridSettings & ScanSet = simSet.GridSettings;
        const APhotonBombAdvancedSettings & AdvSet = simSet.AdvancedSettings;

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
    case EBombGen::Flood: break;
    default: break;
    }
    return nullptr;
}
