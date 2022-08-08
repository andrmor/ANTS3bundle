#include "ageowin_si.h"
#include "ageometrywindow.h"
#include "ageomarkerclass.h"
#include "anoderecord.h"

#include <QTimer>

#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TGeoTrack.h"

AGeoWin_SI::AGeoWin_SI(AGeometryWindow * geoWin) :
    AWindowInterfaceBase(geoWin), GeometryWindow(geoWin)
{
    Description = "Access to the Geometry window of GUI";

    Help["saveImage"] = "Save image currently shown on the geometry window to an image file.\nTip: use .png extension";
}

void AGeoWin_SI::setZoom(int level)
{
    QTimer::singleShot(0, GeometryWindow, [this, level]()
    {
        GeometryWindow->ZoomLevel = level;
        GeometryWindow->Zoom(true);
        GeometryWindow->ShowGeometry(true, false);
        GeometryWindow->readRasterWindowProperties();
    } );
}

void AGeoWin_SI::updateView()
{
    QTimer::singleShot(0, GeometryWindow, [this]()
    {
        GeometryWindow->fRecallWindow = true;
        GeometryWindow->PostDraw();
        GeometryWindow->UpdateRootCanvas();
    } );
}

void AGeoWin_SI::setParallel(bool on)
{
    GeometryWindow->ModePerspective = !on;
}

void AGeoWin_SI::BlockUpdates(bool on)
{
    //DoNotUpdateGeometry = on;
    GeometryWindow->bDisableDraw = on;
}

void AGeoWin_SI::showGeometry()
{
    QTimer::singleShot(0, GeometryWindow, [this]()
    {
        GeometryWindow->readRasterWindowProperties();
        GeometryWindow->ShowGeometry(false);
    } );
}

/*
void AGeoWin_SI::showPMnumbers()
{
    QTimer::singleShot(0, GeometryWindow, [this]()
    {
        GeometryWindow->showPMnumbers();
    } );
}
*/

void AGeoWin_SI::showTracks()
{
    GeometryWindow->ShowTracks();
}

/*
int AGeoWin_SI::AddTrack()
{
    SimManager->Tracks.push_back(new TrackHolderClass());
    return SimManager->Tracks.size() - 1;
}

void AGeoWin_SI::AddNodeToTrack(int trk, float x, float y, float z)
{
    TrackHolderClass* th = SimManager->Tracks.at(trk);
    th->Nodes.push_back(TrackNodeStruct(x, y, z));
    th->Width = 1;
    th->Color = kBlue;
}

void AGeoWin_SI::DeleteAllTracks()
{
    SimManager->Tracks.erase(SimManager->Tracks.begin(), SimManager->Tracks.end());
}
*/

void AGeoWin_SI::addMarkers(QVariantList XYZs, int color, int style, double size)
{
    if (XYZs.isEmpty())
    {
        abort("XYZs should contain non-empty array of coordinates: [[x0,y0,z0], [x1,y1,z1], ... ]");
        return;
    }

    GeoMarkerClass * M = new GeoMarkerClass(GeoMarkerClass::Undefined, style, size, color);
    for (int i = 0; i < XYZs.size(); i++)
    {
        QVariantList el = XYZs[i].toList();
        if (el.size() < 3)
        {
            abort("Bad format for coordinates in XYZs");
            delete M;
            return;
        }
        M->SetNextPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble());
    }
    GeometryWindow->GeoMarkers.push_back(M);
}

void AGeoWin_SI::clearTracks()
{
    GeometryWindow->on_pbClearTracks_clicked();
}

void AGeoWin_SI::clearMarkers()
{
    GeometryWindow->clearGeoMarkers();
}

void AGeoWin_SI::saveImage(QString fileName)
{
    GeometryWindow->SaveAs(fileName);
}
