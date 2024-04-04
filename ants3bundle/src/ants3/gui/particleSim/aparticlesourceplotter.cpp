#include "aparticlesourceplotter.h"
#include "aparticlesourcerecord.h"
#include "ageometryhub.h"
#include "aparticlerecord.h"

#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TVector3.h"

void AParticleSourcePlotter::plotSource(const AParticleSourceRecord & p)
{
    TGeoManager * gGeoManager = AGeometryHub::getInstance().GeoManager;

    const double X0 = p.X0;
    const double Y0 = p.Y0;
    const double Z0 = p.Z0;

    const double Phi   = p.Phi   * pi / 180.0;
    const double Theta = p.Theta * pi / 180.0;
    const double Psi   = p.Psi   * pi / 180.0;

    const double size1 = p.Size1;
    const double size2 = p.Size2;
    const double size3 = p.Size3;

    const double CollPhi   = p.DirectionPhi   * pi / 180.0;
    const double CollTheta = p.DirectionTheta * pi / 180.0;

    double Spread;
    switch (p.AngularMode)
    {
    case AParticleSourceRecord::Isotropic  : Spread = (p.UseCutOff ? p.CutOff * pi / 180.0 : pi); break;
    case AParticleSourceRecord::FixedDirection  : Spread = 0;                                          break;
    case AParticleSourceRecord::GaussDispersion : Spread = (p.UseCutOff ? p.CutOff * pi / 180.0 : 0);  break;
    case AParticleSourceRecord::CustomAngular   : Spread = (p.UseCutOff ? p.CutOff * pi / 180.0 : pi); break;
    }

    //calculating unit vector along 1D direction
    TVector3 VV(sin(Theta)*sin(Phi), sin(Theta)*cos(Phi), cos(Theta));
    //qDebug()<<VV[0]<<VV[1]<<VV[2];

    TVector3 V[3];
    V[0].SetXYZ(size1, 0,     0);
    V[1].SetXYZ(0,     size2, 0);
    V[2].SetXYZ(0,     0,     size3);
    for (int i=0; i<3; i++)
    {
        V[i].RotateX(Phi);
        V[i].RotateY(Theta);
        V[i].RotateZ(Psi);
    }
    switch (p.Shape)
    {
    case AParticleSourceRecord::Point :
    {
        //gGeoManager->SetCurrentPoint(X0,Y0,Z0);
        //gGeoManager->DrawCurrentPoint(9);
        break;
    }
    case (AParticleSourceRecord::Line):
    {
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        track->AddPoint(X0+VV[0]*size1, Y0+VV[1]*size1, Z0+VV[2]*size1, 0);
        track->AddPoint(X0-VV[0]*size1, Y0-VV[1]*size1, Z0-VV[2]*size1, 0);
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    case (AParticleSourceRecord::Rectangle):
    {
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        track->AddPoint(X0-V[0][0]-V[1][0], Y0-V[0][1]-V[1][1], Z0-V[0][2]-V[1][2], 0);
        track->AddPoint(X0+V[0][0]-V[1][0], Y0+V[0][1]-V[1][1], Z0+V[0][2]-V[1][2], 0);
        track->AddPoint(X0+V[0][0]+V[1][0], Y0+V[0][1]+V[1][1], Z0+V[0][2]+V[1][2], 0);
        track->AddPoint(X0-V[0][0]+V[1][0], Y0-V[0][1]+V[1][1], Z0-V[0][2]+V[1][2], 0);
        track->AddPoint(X0-V[0][0]-V[1][0], Y0-V[0][1]-V[1][1], Z0-V[0][2]-V[1][2], 0);
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    case (AParticleSourceRecord::Round):
    {
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        TVector3 Circ;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,0);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }

    case (AParticleSourceRecord::Box):
    {
        for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
            {
                if (j==i) continue;
                //third k
                int k = 0;
                for (; k<2; k++)
                    if (k!=i && k!=j) break;
                for (int s=-1; s<2; s+=2)
                {
                    //  qDebug()<<"i j k shift"<<i<<j<<k<<s;
                    Int_t track_index = gGeoManager->AddTrack(1,22);
                    TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
                    track->AddPoint(X0-V[i][0]-V[j][0]+V[k][0]*s, Y0-V[i][1]-V[j][1]+V[k][1]*s, Z0-V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0+V[i][0]-V[j][0]+V[k][0]*s, Y0+V[i][1]-V[j][1]+V[k][1]*s, Z0+V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0+V[i][0]+V[j][0]+V[k][0]*s, Y0+V[i][1]+V[j][1]+V[k][1]*s, Z0+V[i][2]+V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0-V[i][0]+V[j][0]+V[k][0]*s, Y0-V[i][1]+V[j][1]+V[k][1]*s, Z0-V[i][2]+V[j][2]+V[k][2]*s, 0);
                    track->AddPoint(X0-V[i][0]-V[j][0]+V[k][0]*s, Y0-V[i][1]-V[j][1]+V[k][1]*s, Z0-V[i][2]-V[j][2]+V[k][2]*s, 0);
                    track->SetLineWidth(3);
                    track->SetLineColor(9);
                }
            }
        break;
    }
    case (AParticleSourceRecord::Cylinder):
    {
        TVector3 Circ;
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
        double z = size3;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,z);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        track_index = gGeoManager->AddTrack(1,22);
        track = gGeoManager->GetTrack(track_index);
        z = -z;
        for (int i=0; i<51; i++)
        {
            double x = size1*cos(3.1415926535/25.0*i);
            double y = size1*sin(3.1415926535/25.0*i);
            Circ.SetXYZ(x,y,z);
            Circ.RotateX(Phi);
            Circ.RotateY(Theta);
            Circ.RotateZ(Psi);
            track->AddPoint(X0+Circ[0], Y0+Circ[1], Z0+Circ[2], 0);
        }
        track->SetLineWidth(3);
        track->SetLineColor(9);
        break;
    }
    }

    // Collimation direction
    TVector3 K;
    if (p.DirectionBySphericalAngles)
        K = TVector3(sin(CollTheta)*sin(CollPhi), sin(CollTheta)*cos(CollPhi), cos(CollTheta));
    else
    {
        K = TVector3(p.DirectionVectorX, p.DirectionVectorY, p.DirectionVectorZ);
        K = K.Unit();
    }

    Int_t track_index = gGeoManager->AddTrack(1,22);
    TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);
    const double WorldSizeXY = AGeometryHub::getInstance().getWorldSizeXY();
    const double WorldSizeZ  = AGeometryHub::getInstance().getWorldSizeZ();
    double Klength = std::max(WorldSizeXY, WorldSizeZ)*0.5;

    track->AddPoint(X0, Y0, Z0, 0);
    track->AddPoint(X0+K[0]*Klength, Y0+K[1]*Klength, Z0+K[2]*Klength, 0);
    track->SetLineWidth(2);
    track->SetLineColor(9);

    TVector3 Knorm = K.Orthogonal();
    TVector3 K1(K);
    K1.Rotate(Spread, Knorm);
    for (int i=0; i<8; i++)  //drawing spread
    {
        Int_t track_index = gGeoManager->AddTrack(1,22);
        TVirtualGeoTrack *track = gGeoManager->GetTrack(track_index);

        track->AddPoint(X0, Y0, Z0, 0);
        track->AddPoint(X0+K1[0]*Klength, Y0+K1[1]*Klength, Z0+K1[2]*Klength, 0);
        K1.Rotate(3.1415926535/4.0, K);

        track->SetLineWidth(1);
        track->SetLineColor(9);
    }
}

void AParticleSourcePlotter::clearTracks()
{
    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;
    GeoManager->ClearTracks();
}
