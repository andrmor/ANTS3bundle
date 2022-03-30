#include "TCanvas.h"
#include "ageometrywindow.h"
#include "ui_ageometrywindow.h"
#include "ageometryhub.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "rasterwindowbaseclass.h"
#include "a3global.h"
#include "ajsontools.h"
//#include "anetworkmodule.h"
#include "ageomarkerclass.h"
#include "ageoshape.h"
#include "ageoobject.h"
#include "acameracontroldialog.h"
#include "guitools.h"
#include "ajscripthub.h"
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
#include <QWebEngineDownloadItem>
#include "aroothttpserver.h"
#endif

#include "TView3D.h"
#include "TView.h"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"

AGeometryWindow::AGeometryWindow(QWidget * parent) :
    AGuiWindow("Geo", parent),
    Geometry(AGeometryHub::getInstance()),
    ui(new Ui::AGeometryWindow)
{    
    ui->setupUi(this);

    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    windowFlags |= Qt::WindowMinimizeButtonHint;
    windowFlags |= Qt::WindowMaximizeButtonHint;
    //windowFlags |= Qt::Tool;
    this->setWindowFlags( windowFlags );

    this->setMinimumWidth(200);

    RasterWindow = new RasterWindowBaseClass(this);
    //centralWidget()->layout()->addWidget(RasterWindow);
    connect(RasterWindow, &RasterWindowBaseClass::userChangedWindow, this, &AGeometryWindow::onRasterWindowChange);

    QVBoxLayout * layV = new QVBoxLayout();
    layV->setContentsMargins(0,0,0,0);
    layV->addWidget(RasterWindow);
    ui->swViewers->widget(0)->setLayout(layV);

#ifdef __USE_ANTS_JSROOT__
    WebView = new QWebEngineView(this);
    layV = new QVBoxLayout();
    layV->setContentsMargins(0,0,0,0);
    layV->addWidget(WebView);
    ui->swViewers->widget(1)->setLayout(layV);
    //WebView->load(QUrl("http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks;transp50"));
    QWebEngineProfile::defaultProfile()->connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
                                                 this, &GeometryWindowClass::onDownloadPngRequested);
#endif

    QActionGroup* group = new QActionGroup( this );
    ui->actionSmall_dot->setActionGroup(group);
    ui->actionLarge_dot->setActionGroup(group);
    ui->actionSmall_cross->setActionGroup(group);
    ui->actionLarge_cross->setActionGroup(group);

    ui->cbWireFrame->setVisible(false);

    CameraControl = new ACameraControlDialog(RasterWindow, this);
    CameraControl->setModal(false);

    AJScriptHub::getInstance().addInterface(new AGeoWin_SI(this), "geowin");
}

AGeometryWindow::~AGeometryWindow()
{
    delete ui;
    clearGeoMarkers(0);
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
    Geometry.Top->SetTransparency(Mode == 0 ? 0 : transp);
    adjustGeoAttributes(Geometry.Top, Mode, transp, ui->cbLimitVisibility->isChecked(), level, 0);

    //making contaners visible
    Geometry.Top->SetVisContainers(true);
}

void AGeometryWindow::on_pbShowGeometry_clicked()
{
    //qDebug() << "Redraw triggered!";
    ShowAndFocus();

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot
    if (Mode == 0)
    {
        //RasterWindow->ForceResize();
        //ResetView();
        fRecallWindow = false;
    }

    ShowGeometry(true, false); //not doing "same" option!
}

void AGeometryWindow::ShowGeometry(bool ActivateWindow, bool SAME, bool ColorUpdateAllowed)
{
    //qDebug()<<"  ----Showing geometry----" << MW->GeometryDrawDisabled;
    if (bDisableDraw) return;

    prepareGeoManager(ColorUpdateAllowed);

    int Mode = ui->cobViewer->currentIndex(); // 0 - standard, 1 - jsroot

    if (Mode == 0)
    {
        if (ActivateWindow) ShowAndFocus(); //window is activated (focused)
        else SetAsActiveRootWindow(); //no activation in this mode

        //DRAW
        setHideUpdate(true);
        ClearRootCanvas();
        if (SAME) Geometry.Top->Draw("SAME");  // is it needed at all?
        else      Geometry.Top->Draw("");
        PostDraw();

        //drawing dots
        showGeoMarkers();

        //ResetView();  // angles are resetted, by rotation (with mouse) starts with a wrong angles
        UpdateRootCanvas();

        CameraControl->updateGui();
    }
    else
    {
#ifdef __USE_ANTS_JSROOT__
        //qDebug() << "Before:" << Detector.GeoManager->GetListOfTracks()->GetEntriesFast() << "markers: "<< MW->GeoMarkers.size();

        //deleting old markers
        TObjArray * Arr = Detector.GeoManager->GetListOfTracks();
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

        if (!GeoMarkers.isEmpty())
        {
            for (int i = 0; i < GeoMarkers.size(); i++)
            {
                GeoMarkerClass * gm = GeoMarkers[i];
                //overrides
                if (gm->Type == "Recon" || gm->Type == "Scan" || gm->Type == "Nodes")
                {
                    gm->SetMarkerStyle(GeoMarkerStyle);
                    gm->SetMarkerSize(GeoMarkerSize);
                }

                TPolyMarker3D * mark = new TPolyMarker3D(*gm);
                Detector.GeoManager->GetListOfTracks()->Add(mark);
            }
        }
        //qDebug() << "After:" << Detector.GeoManager->GetListOfTracks()->GetEntriesFast();

        //MW->NetModule->onNewGeoManagerCreated();
        emit requestUpdateRegisteredGeoManager();

        QWebEnginePage * page = WebView->page();
        QString js = "var painter = JSROOT.GetMainPainter(\"onlineGUI_drawing\");";
        js += QString("painter.setAxesDraw(%1);").arg(ui->cbShowAxes->isChecked());
        js += QString("painter.setWireFrame(%1);").arg(ui->cbWireFrame->isChecked());
        js += QString("JSROOT.GEO.GradPerSegm = %1;").arg(ui->cbWireFrame->isChecked() ? 360 / AGlobalSettings::getInstance().NumSegments : 6);
        js += QString("painter.setShowTop(%1);").arg(ui->cbShowTop->isChecked() ? "true" : "false");
        js += "if (JSROOT.hpainter) JSROOT.hpainter.updateAll();";
        page->runJavaScript(js);
#endif
    }
}

void AGeometryWindow::PostDraw()
{
    TView3D *v = dynamic_cast<TView3D*>(RasterWindow->fCanvas->GetView());
    if (!v) return;

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
    RasterWindow->fCanvas->cd();
    this->show();
    this->activateWindow();
    this->raise();
}

void AGeometryWindow::SetAsActiveRootWindow()
{
    RasterWindow->fCanvas->cd();
}

void AGeometryWindow::ClearRootCanvas()
{
    RasterWindow->fCanvas->Clear();
}

void AGeometryWindow::UpdateRootCanvas()
{
    RasterWindow->UpdateRootCanvas();
}

void AGeometryWindow::SaveAs(const QString & filename)
{
    RasterWindow->SaveAs(filename);
}

void AGeometryWindow::ResetView()
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

void AGeometryWindow::setHideUpdate(bool flag)
{
    RasterWindow->setVisible(!flag);
}

void AGeometryWindow::onBusyOn()
{
    this->setEnabled(false);
    RasterWindow->setBlockEvents(true);
}

void AGeometryWindow::onBusyOff()
{
    this->setEnabled(true);
    RasterWindow->setBlockEvents(false);
}

/*
void GeometryWindowClass::writeToJson(QJsonObject & json) const
{
    json["ZoomLevel"] = ZoomLevel;
}

void GeometryWindowClass::readFromJson(const QJsonObject &json)
{
    fRecallWindow = false;
    bool ok = parseJson(json, "ZoomLevel", ZoomLevel);
    if (ok) Zoom(true);
}
*/

bool AGeometryWindow::IsWorldVisible()
{
    return ui->cbShowTop->isChecked();
}

bool AGeometryWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        RasterWindow->UpdateRootCanvas();

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
void AGeometryWindow::ShowPMnumbers()
{
    QVector<QString> tmp;
    for (int i = 0; i < ASensorHub::getConstInstance().countSensors(); i++)
        tmp.append( QString::number(i) );
    ShowText(tmp, kBlack, AGeoWriter::PMs);

    /*
    emit requestUpdateRegisteredGeoManager();
    */
}

void AGeometryWindow::ShowMonitorIndexes()
{
    Geometry.GeoManager->ClearTracks();

    int numMon = AMonitorHub::getConstInstance().countMonitors(AMonitorHub::Photon);
    QVector<QString> tmp;
    for (int i = 0; i < numMon; i++) tmp.append( QString::number(i) );
    ShowText(tmp, kBlue, AGeoWriter::PhotMons, false);

    numMon     = AMonitorHub::getConstInstance().countMonitors(AMonitorHub::Particle);
    tmp.clear();
    for (int i = 0; i < numMon; i++) tmp.append( QString::number(i) );
    ShowText(tmp, kGreen, AGeoWriter::PartMons, false);

    ShowGeometry(false);
    Geometry.GeoManager->DrawTracks();
    UpdateRootCanvas();
}

void AGeometryWindow::ShowText(const QVector<QString> & strData, int color, AGeoWriter::EDraw onWhat, bool bFullCycle)
{
    if (bFullCycle) Geometry.GeoManager->ClearTracks();
    if (!isVisible()) showNormal();

    QString err = GeoWriter.showText(strData, color, onWhat);
    if (!err.isEmpty())
    {
        guitools::message(err, this);
        return;
    }

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

/*
#include "aeventtrackingrecord.h"
#include "asimulationmanager.h"
#include "amaterialparticlecolection.h"
#include "TGeoTrack.h"
#include "atrackrecords.h"
void GeometryWindowClass::ShowEvent_Particles(size_t iEvent, bool withSecondaries)
{
    if (iEvent < SimulationManager.TrackingHistory.size())
    {
        const AEventTrackingRecord * er = SimulationManager.TrackingHistory.at(iEvent);
        er->makeTracks(SimulationManager.Tracks, Detector.MpCollection->getListOfParticleNames(), SimulationManager.TrackBuildOptions, withSecondaries);

        for (int iTr=0; iTr < (int)SimulationManager.Tracks.size(); iTr++)
        {
            const TrackHolderClass* th = SimulationManager.Tracks.at(iTr);
            TGeoTrack* track = new TGeoTrack(1, th->UserIndex);
            track->SetLineColor(th->Color);
            track->SetLineWidth(th->Width);
            track->SetLineStyle(th->Style);
            for (int iNode=0; iNode<th->Nodes.size(); iNode++)
                track->AddPoint(th->Nodes[iNode].R[0], th->Nodes[iNode].R[1], th->Nodes[iNode].R[2], th->Nodes[iNode].Time);
            if (track->GetNpoints()>1)
                Detector.GeoManager->AddTrack(track);
            else delete track;
        }
    }

    DrawTracks();
}
*/

void AGeometryWindow::ShowPMsignals(const QVector<float> & Event, bool bFullCycle)
{
    QVector<QString> tmp;
    for (const float & f : Event)
        tmp.append( QString::number(f) );
    ShowText(tmp, kBlack, AGeoWriter::PMs, bFullCycle);
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

void AGeometryWindow::on_pbShowPMnumbers_clicked()
{
    ShowPMnumbers();
}

void AGeometryWindow::on_pbShowMonitorIndexes_clicked()
{
    ShowMonitorIndexes();
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
    if (ZoomLevel>0) zoomFactor = pow(1.25, ZoomLevel);
    else if (ZoomLevel<0) zoomFactor = pow(0.8, -ZoomLevel);
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

//#include <QElapsedTimer>
void AGeometryWindow::showWebView()
{
#ifdef __USE_ANTS_JSROOT__
    //WebView->load(QUrl("http://localhost:8080/?nobrowser&item=[Objects/GeoWorld/WorldBox_1,Objects/GeoTracks/TObjArray]&opt=nohighlight;dray;all;tracks;transp50"));
    //WebView->load(QUrl("http://localhost:8080/?item=[Objects/GeoWorld/WorldBox_1,Objects/GeoTracks/TObjArray]&opt=nohighlight;dray;all;tracks;transp50"));
    //WebView->load(QUrl("http://localhost:8080/?item=[Objects/GeoWorld/world,Objects/GeoTracks/TObjArray]&opt=nohighlight;dray;all;tracks;transp50"));

    QString s = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=nohighlight;dray;all;tracks";
    //QString s = "http://localhost:8080/?item=Objects/GeoWorld/world&opt=nohighlight;dray;all;tracks";
    if (ui->cbShowTop->isChecked())
        s += ";showtop";
    if (ui->cobViewType->currentIndex() == 1)
        s += ";ortho_camera_rotate";
    if (ui->cbWireFrame->isChecked())
        s += ";wireframe";
    s += QString(";transp%1").arg(ui->sbTransparency->value());

    prepareGeoManager(true);

    WebView->load(QUrl(s));
    WebView->show();

    /*
    QWebEnginePage * page = WebView->page();
    QString js = ""
    "var wait = true;"
    "if ((typeof JSROOT != \"undefined\") && JSROOT.GetMainPainter)"
    "{"
    "   var p = JSROOT.GetMainPainter(\"onlineGUI_drawing\");"
    "   if (p && p.isDrawingReady()) wait = false;"
    "}"
    "wait";

    bool bWait = true;
    QElapsedTimer timer;
    timer.start();
    do
    {
        qApp->processEvents();

        page->runJavaScript(js, [&bWait](const QVariant &v)
        {
            bWait = v.toBool();
        });
        //qDebug() << bWait << timer.elapsed();
    }
    while (bWait && timer.elapsed() < 2000);

    qDebug() << "exit!";
    */

    //ShowGeometry(true, false);
#endif
}

//#include "globalsettingswindowclass.h"
void AGeometryWindow::on_cobViewer_currentIndexChanged(int index)
{
#ifdef __USE_ANTS_JSROOT__
    if (index == 0)
    {
        ui->swViewers->setCurrentIndex(0);
        on_pbShowGeometry_clicked();
    }
    else
    {
        ANetworkModule * NetModule = AGlobalSettings::getInstance().getNetworkModule();
        if (!NetModule->isRootServerRunning())
        {
            bool bOK = NetModule->StartRootHttpServer();
            if (!bOK)
            {
                ui->cobViewer->setCurrentIndex(0);
                message("Failed to start root http server. Check if another server is running at the same port", this);
                emit requestShowNetSettings();
                return;
            }
        }

        ui->swViewers->setCurrentIndex(1);
        showWebView();
    }
    ui->cbWireFrame->setVisible(index == 1);
#else
    if (index != 0)
    {
        ui->cobViewer->setCurrentIndex(0);
        index = 0;
        guitools::message("Enable ants2_jsroot in ants2.pro and rebuild ants2", this);
    }
#endif

    ui->pbCameraDialog->setVisible(index == 0);
    if (index != 0) CameraControl->hide();
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
    ANetworkModule * NetModule = AGlobalSettings::getInstance().getNetworkModule();
    if (NetModule->isRootServerRunning())
    {
        //QString t = "http://localhost:8080/?nobrowser&item=[Objects/GeoWorld/WorldBox_1,Objects/GeoTracks/TObjArray]&opt=dray;all;tracks";
        QString t = "http://localhost:8080/?nobrowser&item=Objects/GeoWorld/world&opt=dray;all;tracks";
        t += ";transp"+QString::number(ui->sbTransparency->value());
        if (ui->cbShowAxes->isChecked()) t += ";axis";

        QDesktopServices::openUrl(QUrl(t));
    }
    else
    {
        message("Root html server has to be started:"
                "\nUse MainWindow->Menu->Settings->Net->Run_CERN_ROOT_HTML_server", this);
    }
#else
    guitools::message("ANTS2 has to be compiled with the activated option in ants2.pro:"
            "\nCONFIG += ants2_RootServer\n", this);
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
        Detector.GeoManager->SetVisLevel(level);
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

