#include "ageowin_si.h"
#include "ageometrywindow.h"
#include "ascripthub.h"
#include "ageomarkerclass.h"
#include "anoderecord.h"

#include <QTimer>
#include <QThread>

#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TGeoTrack.h"

AGeoWin_SI::AGeoWin_SI(AGeometryWindow * geoWin) :
    AWindowInterfaceBase(geoWin), ScrHub(AScriptHub::getInstance()), GeometryWindow(geoWin)
{
    Description = "Access to the window showing detector's geometry";

    Help["redraw"] = "Redraw detector geometry";
    Help["showTracksAndMarkers"] = "Shows currently configured tracks and markers";
    Help["clearTracks"] = "Clear all tracks";
    Help["clearMarkers"] = "Clear all markers";
    Help["saveImage"] = "Save image currently shown on the geometry window to an image file.\nTip: use .png file suffix";
    Help["addMarkers"] = "Add markers (points) to the geometry.\n"
                         "XYZs argument is an array of arrays with marker coordinats [x,y,z],\n"
                         "color, size and style are the properties of the TMarker class of CERN ROOT (google TMarker)";
    Help["addTrack"] = "Add new track to the geometry.\n"
                       "XYZs argument is an array of arrays with the track node coordinats [x,y,z],\n"
                       "color, size and width are the properties of the TAttLine class of CERN ROOT (google TAttLine)";;

}

void AGeoWin_SI::updateGeoWin(AGeometryWindow * newGeoWin)
{
    GeometryWindow = newGeoWin;
    BaseWindow = newGeoWin;
}

void AGeoWin_SI::redraw()
{
    ScrHub.prepareToWait();
    emit ScrHub.requestRedraw();
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::showTracksAndMarkers()
{
    ScrHub.prepareToWait();
    emit ScrHub.requestShowTracks();
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::clearTracks()
{
    ScrHub.prepareToWait();
    emit ScrHub.requestClearTracks();
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::clearMarkers()
{
    ScrHub.prepareToWait();
    emit ScrHub.requestClearMarkers();
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::saveImage(QString fileName)
{
    ScrHub.prepareToWait();
    emit ScrHub.requestSaveImage(fileName);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::addMarkers(QVariantList XYZs, int color, int style, double size)
{
    if (XYZs.isEmpty())
    {
        abort("addMarkers: XYZs should contain non-empty array of coordinates: [[x0,y0,z0], [x1,y1,z1], ... ]");
        return;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(EGeoMarkerType::Undefined, style, size, color);
    for (int i = 0; i < XYZs.size(); i++)
    {
        QVariantList el = XYZs[i].toList();
        if (el.size() < 3)
        {
            abort("addMarkers: bad format for coordinates in XYZs");
            delete markers;
            return;
        }
        markers->SetNextPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble());
    }

    ScrHub.prepareToWait();
    emit ScrHub.requestAddMarkers(markers);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AGeoWin_SI::addTrack(QVariantList XYZs, int color, int style, int width)
{
    TGeoTrack * track = new TGeoTrack(1, 22);

    track->SetLineColor(color);
    track->SetLineStyle(style);
    track->SetLineWidth(width);

    for (int i = 0; i < XYZs.size(); i++)
    {
        QVariantList el = XYZs[i].toList();
        if (el.size() < 3)
        {
            abort("addTrack: bad format for coordinates in XYZs");
            delete track;
            return;
        }
        track->AddPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble(), 0);
    }

    ScrHub.prepareToWait();
    emit ScrHub.requestAddTrack(track);
    ScrHub.waitForGuiCallFinished(Lang);
}

/*
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
*/
