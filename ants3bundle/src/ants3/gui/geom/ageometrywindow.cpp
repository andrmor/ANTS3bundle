#include "TCanvas.h"
#include "ageometrywindow.h"
#include "ui_ageometrywindow.h"
#include "ageometryhub.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "rasterwindowbaseclass.h"
#include "a3global.h"
#include "ajsontools.h"
#include "ageomarkerclass.h"
#include "ageoshape.h"
#include "ageoobject.h"
#include "acameracontroldialog.h"
#include "guitools.h"
#include "ascripthub.h"
#include "ageowin_si.h"

#include <vector>

#include <QStringList>
#include <QDebug>
#include <QFileDialog>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QActionGroup>

#ifdef __USE_ANTS_JSROOT__
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
//#include <QWebEngineDownloadItem>
#endif
#ifdef USE_ROOT_HTML
#include "aroothttpserver.h"
#endif

#include "TView3D.h"
#include "TView.h"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"

AGeometryWindow::AGeometryWindow(bool jsrootViewer, QWidget * parent) :
    AGuiWindow("Geo", parent),
    UseJSRoot(jsrootViewer),
    Geometry(AGeometryHub::getInstance()),
    ui(new Ui::AGeometryWindow)
{
    ui->setupUi(this);

    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    windowFlags |= Qt::WindowMinimizeButtonHint;
    windowFlags |= Qt::WindowMaximizeButtonHint;
    this->setWindowFlags( windowFlags );

    this->setMinimumWidth(200);

#ifndef __USE_ANTS_JSROOT__
    if (UseJSRoot)
    {
        qWarning() << "Cannot use JSROOT viewer: ANTS3 was compiled without flag ants3_jsroot";
        UseJSRoot = false;
    }
    ui->cobViewer->setEnabled(false);
#endif

    if (!UseJSRoot)
    {
        RasterWindow = new RasterWindowBaseClass(this);
        ui->hlMain->addWidget(RasterWindow);
        connect(RasterWindow, &RasterWindowBaseClass::userChangedWindow, this, &AGeometryWindow::onRasterWindowChange);

        ui->cbWireFrame->setVisible(false);
        CameraControl = new ACameraControlDialog(RasterWindow, this);
        CameraControl->setModal(false);
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        WebView = new QWebEngineView(this);
        ui->hlMain->addWidget(WebView);
        //WebView->load(QUrl("http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks;transp50"));

        // !!!*** to fix:
        //QWebEngineProfile::defaultProfile()->connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
        //                                                 this, &AGeometryWindow::onDownloadPngRequested);

        ui->pbCameraDialog->setVisible(false);

        showWebView();
#endif
    }

    TMPignore = true;
    ui->cobViewer->setCurrentIndex(UseJSRoot ? 1 : 0);
    TMPignore = false;

    QActionGroup * group = new QActionGroup( this );
    ui->actionSmall_dot->setActionGroup(group);
    ui->actionLarge_dot->setActionGroup(group);
    ui->actionSmall_cross->setActionGroup(group);
    ui->actionLarge_cross->setActionGroup(group);

    AScriptHub::getInstance().addCommonInterface(new AGeoWin_SI(this), "geowin");
}

AGeometryWindow::~AGeometryWindow()
{
    delete ui;
    clearGeoMarkers(0);
}

void AGeometryWindow::on_cobViewer_currentIndexChanged(int index)
{
    if (TMPignore) return;

    if (index == 1)
    {
#ifdef __USE_ANTS_JSROOT__
        ARootHttpServer & rs = ARootHttpServer::getInstance();
        if (!rs.isRunning())
        {
            qDebug() << "Root server is not running, starting...";
            bool bOK = rs.start();
            if (!bOK)
            {
                ui->cobViewer->setCurrentIndex(0);
                guitools::message("Failed to start root http server. Check if another server is running at the same port", this);
                emit requestShowNetSettings();
                return;
            }
        }
#else
        ui->cobViewer->setCurrentIndex(0);
        guitools::message("Enable ants3_jsroot in ants3.pro and rebuild ants3", this);
        return;
#endif
    }

    storeGeomStatus();

    bool requestJSRoot = (index == 1);
    emit requestChangeGeoViewer(requestJSRoot);
}

void AGeometryWindow::adjustGeoAttributes(TGeoVolume * vol, int Mode, int transp, bool adjustVis, int visLevel, int currentLevel)
{
    const int totNodes = vol->GetNdaughters();
    for (int i=0; i<totNodes; i++)
    {
        TGeoNode* thisNode = (TGeoNode*)vol->GetNodes()->At(i);
        TGeoVolume * v = thisNode->GetVolume();
        v->SetTransparency(Mode == 0 ? 0 : transp);
        if (Mode != 0)
        {
            if (adjustVis)
                v->SetAttBit(TGeoAtt::kVisOnScreen, (currentLevel < visLevel) );
            else
                v->SetAttBit(TGeoAtt::kVisOnScreen, true );
        }

        adjustGeoAttributes(v, Mode, transp, adjustVis, visLevel, currentLevel+1);
    }
}

void AGeometryWindow::prepareGeoManager(bool ColorUpdateAllowed)
{
    if (!Geometry.Top) return;

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot

    //root segments for roundish objects
    Geometry.GeoManager->SetNsegments(A3Global::getInstance().NumSegmentsTGeo);

    //control of visibility of inner volumes
    int level = ui->sbLimitVisibility->value();
    if (!ui->cbLimitVisibility->isChecked()) level = -1;
    Geometry.GeoManager->SetVisLevel(level);

    if (ColorUpdateAllowed)
    {
        if (ColorByMaterial) Geometry.colorVolumes(1);
        else Geometry.colorVolumes(0);
    }

    Geometry.GeoManager->SetTopVisible(ui->cbShowTop->isChecked());
    Geometry.Top->SetAttBit(TGeoAtt::kVisOnScreen, ui->cbShowTop->isChecked());

    int transp = ui->sbTransparency->value();
    Geometry.Top->SetTransparency(UseJSRoot ? transp : 0);
    adjustGeoAttributes(Geometry.Top, Mode, transp, ui->cbLimitVisibility->isChecked(), level, 0);

    //making contaners visible
    Geometry.Top->SetVisContainers(true);
}

void AGeometryWindow::on_pbShowGeometry_clicked()
{
    //qDebug() << "Redraw triggered!";
    ShowAndFocus();
    if (!UseJSRoot) fRecallWindow = false;
    ShowGeometry(true, false); //not doing "same" option!
}

void AGeometryWindow::ShowGeometry(bool activateWindow, bool same, bool colorUpdateAllowed)
{
    if (bDisableDraw) return;

    prepareGeoManager(colorUpdateAllowed);

    if (activateWindow) ShowAndFocus();

    if (UseJSRoot) showGeometryJSRootWindow();
    else showGeometryRasterWindow(same);
}

void AGeometryWindow::showGeometryRasterWindow(bool same)
{
    SetAsActiveRootWindow();

    setHideUpdate(true);
    ClearRootCanvas();
    if (same) Geometry.Top->Draw("SAME");  // is it still needed?
    else      Geometry.Top->Draw("");
    PostDraw();

    showGeoMarkers();

    UpdateRootCanvas();
    CameraControl->updateGui();
}

void AGeometryWindow::showGeometryJSRootWindow()
{
    copyGeoMarksToGeoManager();
    Geometry.notifyRootServerGeometryChanged();

//    showWebView();
//    return;

#ifdef __USE_ANTS_JSROOT__

    bool showAxes = ui->cbShowAxes->isChecked();
    bool wireFrame = ui->cbWireFrame->isChecked();
    int  numSegments = (ui->cbWireFrame->isChecked() ? 360 / A3Global::getInstance().NumSegmentsTGeo : 6);
    bool showTop = ui->cbShowTop->isChecked();

    QString sShowAxes  = (showAxes  ? "true" : "false");
    QString sWireFrame = (wireFrame ? "true" : "false");
    QString sShowTop   = (showTop   ? "true" : "false");
    QString sNumSeg    = QString::number(numSegments);

    QWebEnginePage * page = WebView->page();
    //QString js = "doAnts3Redraw()";
    QString js = QString("doAnts3Redraw(%1, %2, %3, %4)").arg(sShowAxes, sWireFrame, sShowTop, sNumSeg);
    //page->runJavaScript(js);
    page->runJavaScript(js, [](const QVariant &v) { qDebug() << v.toString(); });
#endif
}

void AGeometryWindow::copyGeoMarksToGeoManager()
{
    //deleting old markers
      //qDebug() << "Before:" << Detector.GeoManager->GetListOfTracks()->GetEntriesFast() << "markers: "<< MW->GeoMarkers.size();
    TObjArray * Arr = Geometry.GeoManager->GetListOfTracks();
    const int numObj = Arr->GetEntriesFast();
    int iObj = 0;
    for (; iObj<numObj; iObj++)
        if (!dynamic_cast<TVirtualGeoTrack*>(Arr->At(iObj))) break;
    if (iObj < numObj)
    {
        //qDebug() << "First non-track object:"<<iObj;
        for (int iMarker=iObj; iMarker<numObj; iMarker++)
        {
            delete Arr->At(iMarker);
            (*Arr)[iMarker] = nullptr;
        }
        Arr->Compress();
    }
    //qDebug() << "After filtering markers:"<<Detector.GeoManager->GetListOfTracks()->GetEntriesFast();

    if (!GeoMarkers.empty())
    {
        for (size_t i = 0; i < GeoMarkers.size(); i++)
        {
            GeoMarkerClass * gm = GeoMarkers[i];
            //overrides
            if (gm->Type == GeoMarkerClass::Recon || gm->Type == GeoMarkerClass::Source)
            {
                gm->SetMarkerStyle(GeoMarkerStyle);
                gm->SetMarkerSize(GeoMarkerSize);
            }

            TPolyMarker3D * mark = new TPolyMarker3D(*gm);
            Geometry.GeoManager->GetListOfTracks()->Add(mark);
        }
    }
    //qDebug() << "After:" << Detector.GeoManager->GetListOfTracks()->GetEntriesFast();
}

bool drawIfFound(TGeoNode * node, const TString & name, bool same = false)
{
    if (node->GetName() == name)
    {
        TGeoVolume * vol = node->GetVolume();
        vol->SetLineColor(2);
        AGeometryHub::getInstance().GeoManager->SetTopVisible(true);
        vol->Draw(same ? "same" : "");

        const int totNodes = node->GetNdaughters();
        for (int i = 0; i < totNodes; i++)
        {
            TGeoNode * daugtherNode = node->GetDaughter(i);
            if ( drawIfFound(daugtherNode, daugtherNode->GetName(), true) ) return true;
        }
        return true;
    }

    const int totNodes = node->GetNdaughters();
    for (int i = 0; i < totNodes; i++)
    {
        TGeoNode * daugtherNode = node->GetDaughter(i);
        if ( drawIfFound(daugtherNode, name) ) return true;
    }
    return false;
}

void AGeometryWindow::showRecursive(QString objectName)
{
    ShowAndFocus();

    TString tname = objectName.toLatin1().data();
    tname += "_0";
    bool found = drawIfFound(Geometry.GeoManager->GetTopNode(), tname);
    if (!found)
    {
        tname = objectName.toLatin1().data();
        tname += "_1";
        drawIfFound(Geometry.GeoManager->GetTopNode(), tname);
    }

    UpdateRootCanvas();
}

void AGeometryWindow::PostDraw()
{
    TView3D *v = dynamic_cast<TView3D*>(RasterWindow->fCanvas->GetView());
    if (!v)
    {
        qWarning() << "There is no TView3D!";
        return;
    }

    if (!fRecallWindow) Zoom();

    if (ModePerspective)
    {
        if (!v->IsPerspective()) v->SetPerspective();
    }
    else
    {
        if (v->IsPerspective()) v->SetParallel();
    }

    if (fRecallWindow) RasterWindow->setWindowProperties();

    if (ui->cbShowAxes->isChecked()) v->ShowAxis();
    setHideUpdate(false);

    //canvas is updated in the caller
}

/*
page->runJavaScript("JSROOT.GetMainPainter(\"onlineGUI_drawing\").produceCameraUrl(6)", [page](const QVariant &v)
{
    QString reply = v.toString();
    qDebug() << reply;
    QStringList sl = reply.split(',', QString::SkipEmptyParts); //quick parse just for now
    if (sl.size() > 2)
    {
        QString s;
        //s += "roty" + ui->leY->text() + ",";
        s += sl.at(0) + ",";
        //s += "rotz" + ui->leZ->text() + ",";
        s += sl.at(1) + ",";
        //s += "zoom" + ui->leZoom->text() + ",";
        s += sl.at(2) + ",";
        s += "dray,nohighlight,all,tracks,transp50";
        qDebug() << s;

        page->runJavaScript("JSROOT.redraw(\"onlineGUI_drawing\", JSROOT.GetMainPainter(\"onlineGUI_drawing\").GetObject(), \"" + s + "\");");
    }
});
*/


void AGeometryWindow::ShowAndFocus()
{
    if (!UseJSRoot) RasterWindow->fCanvas->cd();
    show();
    activateWindow();
    raise();
}

void AGeometryWindow::SetAsActiveRootWindow()
{
    if (UseJSRoot) qDebug() << "SetAsActiveRootWindow called in JSRoot mode!!!";
    else RasterWindow->fCanvas->cd();
}

void AGeometryWindow::ClearRootCanvas()
{
    if (UseJSRoot) qDebug() << "ClearRootCanvas called in JSRoot mode!!!";
    else
    {
        RasterWindow->fCanvas->Clear();
        RasterWindow->fCanvas->cd();
    }
}

void AGeometryWindow::UpdateRootCanvas()
{
    if (UseJSRoot) qDebug() << "UpdateRootCanvas called in JSRoot mode!!!";
    else RasterWindow->UpdateRootCanvas();
}

void AGeometryWindow::SaveAs(const QString & filename)
{
    RasterWindow->SaveAs(filename);
}

void AGeometryWindow::ResetView()
{
    if (UseJSRoot) qDebug() << "ResetView called in JSRoot mode!!!";
    else
    {
        if (ui->cobViewer->currentIndex() == 0)
        {
            TView3D *v = dynamic_cast<TView3D*>(RasterWindow->fCanvas->GetView());
            if (!v) return;

            TMPignore = true;
            ui->cobViewType->setCurrentIndex(0);
            TMPignore = false;

            //CameraControl->resetView();  //does not work: Draw() method resets canvas orientation to the last draw
            //RasterWindow->UpdateRootCanvas();
        }
    }
}

void AGeometryWindow::setHideUpdate(bool flag)
{
    if (UseJSRoot) qDebug() << "setHideUpdate called in JSRoot mode!!!";
    else RasterWindow->setVisible(!flag);
}

void AGeometryWindow::onBusyOn()
{
    this->setEnabled(false);
    if (!UseJSRoot) RasterWindow->setBlockEvents(true);
}

void AGeometryWindow::onBusyOff()
{
    this->setEnabled(true);
    if (!UseJSRoot) RasterWindow->setBlockEvents(false);
}

void AGeometryWindow::writeToJson(QJsonObject & json) const
{
    json["ZoomLevel"] = ZoomLevel;

    QJsonObject js;
    GeoWriter.writeToJson(js);
    json["GeoWriter"] = js;
}

void AGeometryWindow::readFromJson(const QJsonObject & json)
{
    fRecallWindow = false;
    bool ok = jstools::parseJson(json, "ZoomLevel", ZoomLevel);
    if (ok) Zoom(true);

    QJsonObject js;
    ok = jstools::parseJson(json, "GeoWriter", js);
    if (ok) GeoWriter.readFromJson(js);

    if (!UseJSRoot) RasterWindow->ForceResize();
}

bool AGeometryWindow::IsWorldVisible()
{
    return ui->cbShowTop->isChecked();
}

bool AGeometryWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
    {
        if (UseJSRoot) ; // !!!***
        else RasterWindow->UpdateRootCanvas();
    }

    //return AGuiWindow::event(event);
    return QMainWindow::event(event);
}

#include <QCloseEvent>
void AGeometryWindow::closeEvent(QCloseEvent * event)
{
    //qDebug() << "Geometry window close event";

    //fix for bug with root reported for Qt 5.14: close then restore results in resize of the canvas to huge size, and nothing is shown on the widow
    event->ignore();
    hide();
}

//#include "anetworkmodule.h"
void AGeometryWindow::showSensorIndexes()
{
    std::vector<QString> tmp;
    for (int i = 0; i < ASensorHub::getConstInstance().countSensors(); i++)
        tmp.push_back( QString::number(i) );
    showText(tmp, kBlack, AGeoWriter::Sensors);

    /*
    emit requestUpdateRegisteredGeoManager();
    */
}

#include "acalorimeterhub.h"
void AGeometryWindow::showCalorimeterIndexes()
{
    const size_t numCal = ACalorimeterHub::getConstInstance().countCalorimeters();
    std::vector<QString> tmp;
    for (size_t i = 0; i < numCal; i++)
        tmp.push_back( QString::number(i) );
    showText(tmp, kRed, AGeoWriter::Calorimeters);
}

void AGeometryWindow::showText(const std::vector<QString> & textVec, int color, AGeoWriter::EDraw onWhat, bool bFullCycle)
{
    if (bFullCycle) Geometry.GeoManager->ClearTracks();
    if (!isVisible()) showNormal();

    if (!RasterWindow->fCanvas->HasViewer3D()) return;

    AGeoViewParameters & p = RasterWindow->ViewParameters;
    p.read(RasterWindow->fCanvas);

    GeoWriter.setOrientationRoot(p.Lat, p.Long/*, p.Psi*/);

    GeoWriter.drawText(textVec, color, onWhat);

    if (bFullCycle)
    {
        ShowGeometry(false);
        Geometry.GeoManager->DrawTracks();
        UpdateRootCanvas();
    }
}

void AGeometryWindow::AddLineToGeometry(QPointF& start, QPointF& end, Color_t color, int width)
{
    Int_t track_index = Geometry.GeoManager->AddTrack(2,22); //  Here track_index is the index of the newly created track in the array of primaries. One can get the pointer of this track and make it known as current track by the manager class:
    TVirtualGeoTrack *track = Geometry.GeoManager->GetTrack(track_index);
    track->SetLineColor(color);
    track->SetLineWidth(width);

    track->AddPoint(start.x(), start.y(), 0, 0);
    track->AddPoint(end.x(), end.y(), 0, 0);
}

void AGeometryWindow::AddPolygonfToGeometry(QPolygonF& poly, Color_t color, int width)
{
    if (poly.size()<2) return;
    for (int i=0; i<poly.size()-1; i++)
        AddLineToGeometry(poly[i], poly[i+1], color, width);
}

void AGeometryWindow::ShowPMsignals(const QVector<float> & Event, bool bFullCycle)
{
    std::vector<QString> tmp;
    for (const float & f : Event)
        tmp.push_back( QString::number(f) );
    showText(tmp, kBlack, AGeoWriter::Sensors, bFullCycle);
}

void AGeometryWindow::showGeoMarkers()
{
    if (GeoMarkers.empty()) return;

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        SetAsActiveRootWindow();
        for (GeoMarkerClass * gm : GeoMarkers)
        {
            if (gm->Type == GeoMarkerClass::Recon || gm->Type == GeoMarkerClass::True) // Source has its own styling
            {
                gm->SetMarkerStyle(GeoMarkerStyle);
                gm->SetMarkerSize(GeoMarkerSize);
            }
            gm->Draw("same");
        }
        UpdateRootCanvas();
    }
    else
        ShowGeometry(false);
}

#include "anoderecord.h"
void AGeometryWindow::addPhotonNodeGeoMarker(const ANodeRecord & record)
{
    if (GeoMarkers.empty() || GeoMarkers.back()->Type != GeoMarkerClass::True)
    {
        GeoMarkerClass * gm = new GeoMarkerClass(GeoMarkerClass::True, 21, 10, kBlue);
        GeoMarkers.push_back(gm);
    }
    GeoMarkers.back()->SetNextPoint(record.R[0], record.R[1], record.R[2]);
}

void AGeometryWindow::addGeoMarkers(const std::vector<std::array<double, 3>> & XYZs, int color, int style, double size)
{
    GeoMarkerClass * M = new GeoMarkerClass(GeoMarkerClass::Undefined, style, size, color);
    for (const auto & pos : XYZs)
        M->SetNextPoint(pos[0], pos[1], pos[2]);
    GeoMarkers.push_back(M);
}

void AGeometryWindow::ShowTracksAndMarkers()
{
    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        ShowTracks();
        showGeoMarkers();
    }
    else
    {
        ShowGeometry(true, false);
    }
}

void AGeometryWindow::ClearTracks(bool bRefreshWindow)
{
    Geometry.GeoManager->ClearTracks();
    if (bRefreshWindow)
    {
        int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
        if (Mode == 0)
        {
            SetAsActiveRootWindow();
            UpdateRootCanvas();
        }
        else ShowGeometry(false);
    }
}

void AGeometryWindow::clearGeoMarkers(int All_Rec_True)
{
    for (int i = GeoMarkers.size()-1; i>-1; i--)
    {
        switch (All_Rec_True)
        {
        case 1:
            if (GeoMarkers[i]->Type == GeoMarkerClass::Recon)
            {
                delete GeoMarkers[i];
                GeoMarkers.erase(GeoMarkers.begin() + i);
            }
            break;
        case 2:
            if (GeoMarkers[i]->Type == GeoMarkerClass::True)
            {
                delete GeoMarkers[i];
                GeoMarkers.erase(GeoMarkers.begin() + i);
            }
            break;
        case 0:
        default:
            delete GeoMarkers[i];
        }
    }

    if (All_Rec_True == 0) GeoMarkers.clear();
}

void AGeometryWindow::on_cbColor_toggled(bool checked)
{
    ColorByMaterial = checked;
    emit requestUpdateMaterialListWidget();
    ShowGeometry(true, false);
}

void AGeometryWindow::on_pbShowTracks_clicked()
{
    ShowTracks();
}

void AGeometryWindow::ShowTracks()
{
    if (bDisableDraw) return;

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        SetAsActiveRootWindow();
        Geometry.GeoManager->DrawTracks();
        UpdateRootCanvas();
    }
    else ShowGeometry(false);
}

void AGeometryWindow::ShowPoint(double * r, bool keepTracks)
{
    clearGeoMarkers();

    GeoMarkerClass * marks = new GeoMarkerClass(GeoMarkerClass::Source, 3, 10, kBlack);
    marks->SetNextPoint(r[0], r[1], r[2]);
    GeoMarkers.push_back(marks);
    GeoMarkerClass* marks1 = new GeoMarkerClass(GeoMarkerClass::Source, 4, 3, kRed);
    marks1->SetNextPoint(r[0], r[1], r[2]);
    GeoMarkers.push_back(marks1);

    ShowGeometry(false);
    if (keepTracks) ShowTracks();
}

void AGeometryWindow::addGenerationMarker(const double * Pos)
{
    GeoMarkerClass * marks = nullptr;
    if (!GeoMarkers.empty() && GeoMarkers.back()->Type == GeoMarkerClass::Source) marks = GeoMarkers.back();
    else
    {
        marks = new GeoMarkerClass(GeoMarkerClass::Source, 7, 1, 1);
        GeoMarkers.push_back(marks);
    }

    marks->SetNextPoint(Pos[0], Pos[1], Pos[2]);
}

void AGeometryWindow::CenterView(double *r)
{
    if (!RasterWindow->fCanvas->HasViewer3D()) return;

    AGeoViewParameters & p = RasterWindow->ViewParameters;
    p.read(RasterWindow->fCanvas);

    const double size = 100.0;

    for (int i=0; i<3; i++)
    {
        p.RangeLL[i] = r[i] - size;
        p.RangeUR[i] = r[i] + size;
    }
    p.WinX = 0;
    p.WinY = 0;

    RasterWindow->setVisible(false);
    p.apply(RasterWindow->fCanvas);
    RasterWindow->setVisible(true);
    TView3D * v = static_cast<TView3D*>(RasterWindow->fCanvas->GetView());
    v->Zoom();
}

void AGeometryWindow::on_pbClearTracks_clicked()
{
    Geometry.GeoManager->ClearTracks();
    ShowGeometry(true, false);
}

void AGeometryWindow::on_pbTop_clicked()
{
    if (ui->cobViewer->currentIndex() == 0)
    {
        SetAsActiveRootWindow();
        TView * v = RasterWindow->fCanvas->GetView();
        v->Top();
        RasterWindow->fCanvas->Modified();
        RasterWindow->fCanvas->Update();
        readRasterWindowProperties();
        CameraControl->updateGui();
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        QWebEnginePage * page = WebView->page();
        QString js = ""
        "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");"
        "painter.setCameraPosition(90,0);";
        page->runJavaScript(js);

        /*
        page->runJavaScript("JSROOT.GetMainPainter(\"onlineGUI_drawing\").produceCameraUrl()", [page](const QVariant &v)
        {
            QString reply = v.toString();
            qDebug() << reply; // let's ask Sergey to make JSON with this data
            QStringList sl = reply.split(',', QString::SkipEmptyParts); //quick parse just for now
            if (sl.size() > 2)
            {
                QString s;
                //s += "roty" + ui->leY->text() + ",";
                s += "roty90,";
                //s += "rotz" + ui->leZ->text() + ",";
                s += "rotz0,";
                //s += "zoom" + ui->leZoom->text() + ",";
                s += sl.at(2) + ",";
                s += "dray,nohighlight,all,tracks,transp50";
                page->runJavaScript("JSROOT.redraw(\"onlineGUI_drawing\", JSROOT.GetMainPainter(\"onlineGUI_drawing\").GetObject(), \"" + s + "\");");
            }
        });
        */
#endif
    }
}

void AGeometryWindow::on_pbFront_clicked()
{
    if (ui->cobViewer->currentIndex() == 0)
    {
        SetAsActiveRootWindow();
        TView *v = RasterWindow->fCanvas->GetView();
        v->Front();
        RasterWindow->fCanvas->Modified();
        RasterWindow->fCanvas->Update();
        readRasterWindowProperties();
        CameraControl->updateGui();
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        QWebEnginePage * page = WebView->page();
        QString js = ""
        "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");"
        "painter.setCameraPosition(90,90);";
        page->runJavaScript(js);
#endif
    }
}

void AGeometryWindow::onRasterWindowChange()
{
    fRecallWindow = true;
    CameraControl->updateGui();
}

void AGeometryWindow::readRasterWindowProperties()
{
    fRecallWindow = true;
    RasterWindow->ViewParameters.read(RasterWindow->fCanvas);   // !*! method
}

void AGeometryWindow::on_pbSide_clicked()
{
    if (ui->cobViewer->currentIndex() == 0)
    {
        SetAsActiveRootWindow();
        TView *v = RasterWindow->fCanvas->GetView();
        v->Side();
        RasterWindow->fCanvas->Modified();
        RasterWindow->fCanvas->Update();
        readRasterWindowProperties();
        CameraControl->updateGui();
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        QWebEnginePage * page = WebView->page();
        QString js = ""
                     "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");"
                     "painter.setCameraPosition(0.001,0.01);";
        page->runJavaScript(js);
#endif
    }
}

void AGeometryWindow::on_cbShowAxes_toggled(bool /*checked*/)
{
    if (ui->cobViewer->currentIndex() == 0)
    {
        TView *v = RasterWindow->fCanvas->GetView();
        v->ShowAxis(); //it actually toggles show<->hide
    }
    else ShowGeometry(true, false);
}


void AGeometryWindow::on_actionSmall_dot_toggled(bool arg1)
{
    if (arg1)
    {
        GeoMarkerStyle = 1;
        ShowGeometry();
    }

    ui->actionSize_1->setEnabled(false);
    ui->actionSize_2->setEnabled(false);
}

void AGeometryWindow::on_actionLarge_dot_triggered(bool arg1)
{
    if (arg1)
    {
        GeoMarkerStyle = 8;
        ShowGeometry();
    }

    ui->actionSize_1->setEnabled(true);
    ui->actionSize_2->setEnabled(true);
}

void AGeometryWindow::on_actionSmall_cross_toggled(bool arg1)
{
    if (arg1)
    {
        GeoMarkerStyle = 6;
        ShowGeometry();
    }

    ui->actionSize_1->setEnabled(false);
    ui->actionSize_2->setEnabled(false);
}

void AGeometryWindow::on_actionLarge_cross_toggled(bool arg1)
{
    if (arg1)
    {
        GeoMarkerStyle = 2;
        ShowGeometry();
    }

    ui->actionSize_1->setEnabled(true);
    ui->actionSize_2->setEnabled(true);
}

void AGeometryWindow::on_actionSize_1_triggered()
{
    GeoMarkerSize++;
    ShowGeometry();

    ui->actionSize_2->setEnabled(true);
}

void AGeometryWindow::on_actionSize_2_triggered()
{
    if (GeoMarkerSize>0) GeoMarkerSize--;

    if (GeoMarkerSize==0) ui->actionSize_2->setEnabled(false);
    else ui->actionSize_2->setEnabled(true);

    ShowGeometry();
}

void AGeometryWindow::Zoom(bool update)
{
    TView3D *v = dynamic_cast<TView3D*>(RasterWindow->fCanvas->GetView());
    if (!v) return;

    double zoomFactor = 1.0;
    if (ZoomLevel > 0) zoomFactor = pow(1.25, ZoomLevel);
    else if (ZoomLevel < 0) zoomFactor = pow(0.8, -ZoomLevel);
    if (ZoomLevel != 0) v->ZoomView(0, zoomFactor);
    if (update)
    {
        RasterWindow->ForceResize();
        //fRecallWindow = false;
        UpdateRootCanvas();
    }
}

void AGeometryWindow::FocusVolume(QString name)
{
    CameraControl->setFocus(name);
}

void AGeometryWindow::on_actionDefault_zoom_1_triggered()
{
    ZoomLevel++;
    on_pbShowGeometry_clicked();
}

void AGeometryWindow::on_actionDefault_zoom_2_triggered()
{
    ZoomLevel--;
    on_pbShowGeometry_clicked();
}

void AGeometryWindow::on_actionDefault_zoom_to_0_triggered()
{
    ZoomLevel = 0;
    on_pbShowGeometry_clicked();
}

void AGeometryWindow::on_actionSet_line_width_for_objects_triggered()
{
    doChangeLineWidth(1);
}

void AGeometryWindow::on_actionDecrease_line_width_triggered()
{
    doChangeLineWidth(-1);
}

void AGeometryWindow::doChangeLineWidth(int deltaWidth)
{
    TObjArray * list = Geometry.GeoManager->GetListOfVolumes();
    const int numVolumes = list->GetEntries();
    for (int i = 0; i < numVolumes; i++)
    {
        TGeoVolume * tv = (TGeoVolume*)list->At(i);
        int LWidth = tv->GetLineWidth() + deltaWidth;
        if (LWidth < 1) LWidth = 1;
        tv->SetLineWidth(LWidth);
    }
    Geometry.changeLineWidthOfVolumes(deltaWidth);
    ShowGeometry(true, false);
}

void AGeometryWindow::showWebView()
{
    //qDebug() << "------------------showWebView------------------";
#ifdef __USE_ANTS_JSROOT__
        //QString sss = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks;transp50";
        //QString sss = "https://webapps.frm2.tum.de/neutroncalc/";
    /*
    QString sss = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=all;tracks;transp50";
    WebView->load(QUrl(sss));
    WebView->show();
    return;
    */

    QString s = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=all;tracks";
    //QString s = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks";
    //QString s = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=nohighlight;dray;all;tracks";
    //QString s = "http://localhost:8080/?item=Objects/GeoWorld/world&opt=nohighlight;dray;all;tracks";
    if (ui->cbShowTop->isChecked())           s += ";showtop";
    if (ui->cobViewType->currentIndex() == 1) s += ";ortho_camera_rotate";
    if (ui->cbWireFrame->isChecked())         s += ";wireframe";
    s += QString(";transp%1").arg(ui->sbTransparency->value());

    prepareGeoManager(true);
    WebView->load(QUrl(s));
    WebView->show();
#endif
}

void AGeometryWindow::on_actionOpen_GL_viewer_triggered()
{
    int tran = ui->sbTransparency->value();
    TObjArray * list = Geometry.GeoManager->GetListOfVolumes();
    int numVolumes = list->GetEntries();

    for (int i = 0; i < numVolumes; i++)
    {
        TGeoVolume * tv = (TGeoVolume*)list->At(i);
        tv->SetTransparency(tran);
    }
    OpenGLview();
}

void AGeometryWindow::OpenGLview()
{
    RasterWindow->fCanvas->GetViewer3D("ogl");
}

void AGeometryWindow::on_actionJSROOT_in_browser_triggered()
{
#ifdef USE_ROOT_HTML
    ARootHttpServer & RootServer = ARootHttpServer::getInstance();
    if (RootServer.isRunning())
    {
        //QString t = "http://localhost:8080/?nobrowser&item=[Objects/GeoWorld/WorldBox_1,Objects/GeoTracks/TObjArray]&opt=dray;all;tracks";
        //QString t = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks";
        QString t = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=all;tracks";
        t += ";transp"+QString::number(ui->sbTransparency->value());
        if (ui->cbShowAxes->isChecked()) t += ";axis";

        QDesktopServices::openUrl(QUrl(t));
    }
    else
    {
        guitools::message("Start ROOT http server to use this feature"
                          "\nUse MainWindow->Menu->Settings->Net->Run_ROOT_HTTP_server", this);
    }
#else
    guitools::message("ANTS3 has to be compiled with the activated option in ants3.pro:"
            "\nCONFIG += ants3_RootServer\n", this);
#endif
}

/*
    QWebEnginePage * page = WebView->page();
    page->runJavaScript(

//    "JSROOT.NewHttpRequest(\"http://localhost:8080/Objects/GeoWorld/WorldBox_1/root.json\", \"object\","
//    //"JSROOT.NewHttpRequest(\"http://localhost:8080/Objects/GeoTracks/TObjArray/root.json\", \"object\","
//    "function(res) {"
//        //"if (res) console.log('Retrieve object', res._typename);"
//        //    "else console.error('Fail to get object');"
//        //  "JSROOT.cleanup();"

//          //"JSROOT.redraw(\"onlineGUI_drawing\", res, \"transp50,tracks\");"
//          //"JSROOT.redraw(\"onlineGUI_drawing\", res, \"transp50,tracks\");"
//          "JSROOT.draw(\"onlineGUI_drawing\", res, \"transp50,tracks\");"
//    "}).send();");



                "var xhr = JSROOT.NewHttpRequest('http://localhost:8080/multi.json?number=2', 'multi', function(res)"
                                                        "{"
                                                            "if (!res) return;"
                                                            "JSROOT.redraw('onlineGUI_drawing', res[0], 'transp50;tracks');"
                                                        "}"
                                                ");"
                "xhr.send('Objects/GeoWorld/WorldBox_1/root.json\\nObjects/GeoTracks/TObjArray/root.json\\n');"
                //"xhr.send('/Objects/GeoWorld/WorldBox_1/root.json\n');"
                );
*/

void AGeometryWindow::on_cbWireFrame_toggled(bool)
{
    ShowGeometry(true, false);
}

void AGeometryWindow::on_cbLimitVisibility_clicked()
{
    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
        ShowGeometry(true, false);
    else
    {
#ifdef __USE_ANTS_JSROOT__
        int level = ui->sbLimitVisibility->value();
        if (!ui->cbLimitVisibility->isChecked()) level = -1;
        Geometry.GeoManager->SetVisLevel(level);
        //MW->NetModule->onNewGeoManagerCreated();
        emit requestUpdateRegisteredGeoManager();

        prepareGeoManager();
        showWebView();
#endif
    }
}

void AGeometryWindow::on_sbLimitVisibility_editingFinished()
{
    on_cbLimitVisibility_clicked();
}

void AGeometryWindow::on_cbShowTop_toggled(bool)
{
    ShowGeometry(true, false);
    /*
    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
        ShowGeometry(true, false);
    else
    {
#ifdef __USE_ANTS_JSROOT__
        ShowGeometry(true, false);
        QWebEnginePage * page = WebView->page();
        QString js = "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");";
        //js += QString("painter.options.showtop = %1;").arg(checked ? "true" : "false");
        js += QString("painter.setShowTop(%1);").arg(checked ? "true" : "false");
        js += "painter.startDrawGeometry();";
        page->runJavaScript(js);
#endif
    }
*/
}

void AGeometryWindow::on_cobViewType_currentIndexChanged(int index)
{
    if (TMPignore) return;

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        TView *v = RasterWindow->fCanvas->GetView();
        if (index == 0)
        {
            ModePerspective = true;
            v->SetPerspective();
        }
        else
        {
            ModePerspective = false;
            v->SetParallel();
        }

        RasterWindow->fCanvas->Modified();
        RasterWindow->fCanvas->Update();
        readRasterWindowProperties();
        CameraControl->updateGui();

#if ROOT_VERSION_CODE < ROOT_VERSION(6,11,1)
        RasterWindow->setInvertedXYforDrag( index==1 );
#endif
    }
    else
    {
        prepareGeoManager();
        showWebView();
        // ShowGeometry(true, false);
    }
}

void AGeometryWindow::on_pbSaveAs_clicked()
{
    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        QFileDialog *fileDialog = new QFileDialog;
        fileDialog->setDefaultSuffix("png");
        //QString fileName = fileDialog->getSaveFileName(this, "Save image as file", AGlobalSettings::getInstance().LastOpenDir, "png (*.png);;gif (*.gif);;Jpg (*.jpg)");
        QString fileName = fileDialog->getSaveFileName(this, "Save image as file", "", "png (*.png);;gif (*.gif);;Jpg (*.jpg)");
        if (fileName.isEmpty()) return;
//        AGlobalSettings::getInstance().LastOpenDir = QFileInfo(fileName).absolutePath();

        QFileInfo file(fileName);
        if(file.suffix().isEmpty()) fileName += ".png";
        AGeometryWindow::SaveAs(fileName);
//        if (AGlobalSettings::getInstance().fOpenImageExternalEditor) QDesktopServices::openUrl(QUrl("file:"+fileName, QUrl::TolerantMode));
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        QWebEnginePage * page = WebView->page();
        QString js = "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");";
        js += QString("painter.createSnapshot('dummy.png')");
        page->runJavaScript(js);
#endif
    }
}

void AGeometryWindow::onDownloadPngRequested(QWebEngineDownloadItem *item)
{
/*
#ifdef __USE_ANTS_JSROOT__
    QString fileName = QFileDialog::getSaveFileName(this, "Select file name to safe image");
    if (fileName.isEmpty())
    {
        item->cancel();
        return;
    }
    item->setPath(fileName);
    item->accept();
#endif
*/
}

void AGeometryWindow::on_pbCameraDialog_clicked()
{
    if (CameraControl->xPos == 0 && CameraControl->yPos == 0)
    {
        CameraControl->xPos = x() + width() + 3;
        CameraControl->yPos = y() + 0.5*height() - 0.5*CameraControl->height();

        CameraControl->move(CameraControl->xPos, CameraControl->yPos);
        bool bVis = guitools::assureWidgetIsWithinVisibleArea(CameraControl);
        if (!bVis)
        {
            CameraControl->xPos = x() + 0.5*width()  - 0.5*CameraControl->width();
            CameraControl->yPos = y() + 0.5*height() - 0.5*CameraControl->height();
        }
    }

    CameraControl->showAndUpdate();
}

void AGeometryWindow::on_pbClearMarkers_clicked()
{
    clearGeoMarkers();
    on_pbShowGeometry_clicked();
}

void AGeometryWindow::showParticleMonIndexes()
{
    Geometry.GeoManager->ClearTracks();

    const int numMon = AMonitorHub::getConstInstance().countMonitors(AMonitorHub::Particle);
    std::vector<QString> tmp;
    for (int i = 0; i < numMon; i++) tmp.push_back( QString::number(i) );
    showText(tmp, kGreen, AGeoWriter::PartMons, true);

    /*
    emit requestUpdateRegisteredGeoManager();
    */
}

void AGeometryWindow::showPhotonMonIndexes()
{
    Geometry.GeoManager->ClearTracks();

    int numMon = AMonitorHub::getConstInstance().countMonitors(AMonitorHub::Photon);
    std::vector<QString> tmp;
    for (int i = 0; i < numMon; i++) tmp.push_back( QString::number(i) );
    showText(tmp, kBlue, AGeoWriter::PhotMons, true);

    /*
    emit requestUpdateRegisteredGeoManager();
    */
}

void AGeometryWindow::showSensorModelIndexes(int iModel)
{
    Geometry.GeoManager->ClearTracks();

    const ASensorHub & SH = ASensorHub::getConstInstance();
    int numSensors = SH.countSensors();
    std::vector<QString> tmp;
    for (int i = 0; i < numSensors; i++)
    {
        const int index = SH.getModelIndex(i);
        if (iModel != -1 && iModel != index)
            tmp.push_back("");
        else
            tmp.push_back( QString::number(index) );
    }
    showText(tmp, kRed, AGeoWriter::Sensors, true);

    /*
    emit requestUpdateRegisteredGeoManager();
    */
}

#include "ashownumbersdialog.h"
void AGeometryWindow::on_pbShowNumbers_clicked()
{
    AShowNumbersDialog d(*this);
    d.exec();
}

