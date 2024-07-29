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
    AWindowInterfaceBase(geoWin), GeometryWindow(geoWin)
{
    Description = "Access to the Geometry window of GUI";

    Help["redraw"] = "Redraw detector geometry";

    //Help["saveImage"] = "Save image currently shown on the geometry window to an image file.\nTip: use .png extension";

    connect(this, &AGeoWin_SI::requestRedraw,       geoWin, &AGeometryWindow::onRequestRedrawFromScript,       Qt::QueuedConnection);
    connect(this, &AGeoWin_SI::requestShowTracks,   geoWin, &AGeometryWindow::onRequestShowTracksFromScript,   Qt::QueuedConnection);
    connect(this, &AGeoWin_SI::requestClearTracks,  geoWin, &AGeometryWindow::onRequestClearTracksFromScript,  Qt::QueuedConnection);
    connect(this, &AGeoWin_SI::requestClearMarkers, geoWin, &AGeometryWindow::onRequestClearMarkersFromScript, Qt::QueuedConnection);
    connect(this, &AGeoWin_SI::requestAddMarkers,   geoWin, &AGeometryWindow::onRequestAddMarkersFromScript,   Qt::QueuedConnection);
    connect(this, &AGeoWin_SI::requestAddTrack,     geoWin, &AGeometryWindow::onRequestAddTrackFromScript,     Qt::QueuedConnection);

    connect(geoWin, &AGeometryWindow::taskRequestedFromScriptCompleted, this, &AGeoWin_SI::onWindowReportTaskCompleted, Qt::DirectConnection);
}

void AGeoWin_SI::updateGeoWin(AGeometryWindow * newGeoWin)
{
    GeometryWindow = newGeoWin;
    BaseWindow = newGeoWin;
}

void AGeoWin_SI::onWindowReportTaskCompleted()
{
    WaitingForTaskCompleted = false;
}

void AGeoWin_SI::redraw()
{
    WaitingForTaskCompleted = true;
    emit requestRedraw();

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
}

void AGeoWin_SI::showTracksAndMarkers()
{
    WaitingForTaskCompleted = true;
    emit requestShowTracks();

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
}

void AGeoWin_SI::clearTracks()
{
    WaitingForTaskCompleted = true;
    emit requestClearTracks();

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
}

void AGeoWin_SI::clearMarkers()
{
    WaitingForTaskCompleted = true;
    emit requestClearMarkers();

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
}

void AGeoWin_SI::saveImage(QString fileName)
{
    WaitingForTaskCompleted = true;
    emit requestSaveImage(fileName);

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
}

void AGeoWin_SI::addMarkers(QVariantList XYZs, int color, int style, double size)
{
    if (XYZs.isEmpty())
    {
        abort("XYZs should contain non-empty array of coordinates: [[x0,y0,z0], [x1,y1,z1], ... ]");
        return;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(AGeoMarkerClass::Undefined, style, size, color);
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

    WaitingForTaskCompleted = true;
    emit requestAddMarkers(markers);

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
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

    WaitingForTaskCompleted = true;
    emit requestAddTrack(track);

    while (WaitingForTaskCompleted)
    {
        AScriptHub::getInstance().processEvents(Lang);
        QThread::usleep(100);
    }
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
