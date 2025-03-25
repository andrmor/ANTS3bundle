#include "ageometrytester.h"

#include <QDebug>

#include "TGeoManager.h"

#include <cmath>

void AGeometryTester::Test(double * start, double * direction)
{
    Record.clear();

    double Norm = 0;
    for (int i = 0; i < 3; i++) Norm += direction[i] * direction[i];
    Norm = sqrt(Norm);
    if (Norm < 1e-20)
    {
        direction[0] = 0;
        direction[1] = 0;
        direction[2] = 1.0;
    }
    else if ( fabs(Norm) < 0.99999 || fabs(Norm) > 1.00001 )
        for (int i=0; i<3; i++) direction[i] /= Norm;

    TGeoNavigator* navigator = GeoManager->GetCurrentNavigator();
    navigator->SetCurrentPoint(start);
    navigator->SetCurrentDirection(direction);
    navigator->FindNode();

    while (!navigator->IsOutside())
    {
       AGeometryTesterReportRecord r;

       TGeoVolume* vol = navigator->GetCurrentVolume();

       r.volName = vol->GetName();       
       r.nodeIndex = navigator->GetCurrentNode()->GetNumber();
       r.matIndex = vol->GetMaterial()->GetIndex();

       r.startX = navigator->GetCurrentPoint()[0];
       r.startY = navigator->GetCurrentPoint()[1];
       r.startZ = navigator->GetCurrentPoint()[2];

       navigator->FindNextBoundaryAndStep();

       Record.push_back(r);
    }

    escapeX = navigator->GetCurrentPoint()[0];
    escapeY = navigator->GetCurrentPoint()[1];
    escapeZ = navigator->GetCurrentPoint()[2];
}
