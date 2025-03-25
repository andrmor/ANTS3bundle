#include "arasterwindow.h"

#include <QDebug>
#include <QMouseEvent>
#include <QMainWindow>
#include <QtGlobal>

#include "TCanvas.h"
#include "TView.h"
#include "TView3D.h"
#include "TVirtualX.h"

ARasterWindow::ARasterWindow(QMainWindow *MasterWindow) : QWidget(MasterWindow)
{
    qDebug()<<"->Creating raster window";

    // set options needed to properly update the canvas when resizing the widget
    // and to properly handle context menus and mouse move events
    setAttribute(Qt::WA_PaintOnScreen, true); // required to add in header: QPaintEngine * paintEngine() const override {return nullptr;}
       //setAttribute(Qt::WA_PaintOnScreen, false);
       //setAttribute(Qt::WA_OpaquePaintEvent, true);
       //setAttribute(Qt::WA_NativeWindow, true);

    setUpdatesEnabled(false);
    setMouseTracking(true);
    setMinimumSize(300, 200);

    // register the QWidget in TVirtualX, giving its native window id
    int wid = gVirtualX->AddWindow((ULong_t)winId(), width(), height());
    // create the ROOT TCanvas, giving as argument the QWidget registered id
    fCanvas = new TCanvas("Root Canvas", width(), height(), wid);
    //TQObject::Connect("TGPopupMenu", "PoppedDown()", "TCanvas", fCanvas, "Update()");

    fCanvas->SetBorderMode(0);
    //fCanvas->SetRightMargin(0.10);
    //fCanvas->SetLeftMargin(0.10);
    fCanvas->SetFillColor(0);

    //qDebug() << "  ->Root canvas created";
}

ARasterWindow::~ARasterWindow()
{
    //qDebug()<< "     <--Starting cleanup for raster window base...";

    fCanvas->Clear();
    delete fCanvas; fCanvas = nullptr;
    //qDebug()<<"        canvas deleted";

    //gVirtualX->RemoveWindow(wid); //causes strange warnings !!!***
    //qDebug()<<"        window unregistered in Root";
    //qDebug()<< "  <-Done";
}

void ARasterWindow::setAsActiveRootWindow()
{
    fCanvas->cd();
}

void ARasterWindow::clearRootCanvas()
{
    fCanvas->Clear();
}

void ARasterWindow::updateRootCanvas()
{
    fCanvas->Modified();
    fCanvas->Update();
}

/*
void ARasterWindow::exposeEvent(QExposeEvent *)
{
  if (!fCanvas) return;
  if (BlockEvents) return;
  //qDebug() << "raster window -> Expose event";
  if (isExposed()) fCanvas->Update();
}
*/

void ARasterWindow::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "Mouse move event";
    if (!fCanvas) return;
    fCanvas->cd();
    if (BlockEvents) return;

    if (event->buttons() & Qt::LeftButton)
    {
        //qDebug() << "-->Mouse left-pressed move event";
        if (!PressEventRegistered) return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (fInvertedXYforDrag) fCanvas->HandleInput(kButton1Motion, event->pos().y(), event->pos().x());
        else                    fCanvas->HandleInput(kButton1Motion, event->pos().x(), event->pos().y());
#else
        if (InvertedXYforDrag) fCanvas->HandleInput(kButton1Motion, event->position().y(), event->position().x()); // !!!*** Round()?
        else                    fCanvas->HandleInput(kButton1Motion, event->position().x(), event->position().y());
#endif

        onViewChanged();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        //qDebug() << "Mouse right-pressed move event";  // not implemented by root!
        //right-pressed move
        //if (!PressEventRegistered) return;
        //fCanvas->HandleInput(kButton3Motion, event->x(), event->y());
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        //qDebug() << "-->Mid button move!";
        //fCanvas->HandleInput(kButton2Motion, event->x(), event->y());
        if (!PressEventRegistered) return;
        if (!fCanvas->HasViewer3D()) return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        int x = event->pos().x();
        int y = event->pos().y();
#else
        int x = event->position().x(); // !!!*** Round()?
        int y = event->position().y();
#endif

        int dx = x - LastX;
        int dy = y - LastY;
        if (fCanvas->GetView())
        {
            ViewParameters.read(fCanvas);
            ViewParameters.WinX = LastCenterX - 2.0 * dx / (double)this->width()  * ViewParameters.WinW;
            ViewParameters.WinY = LastCenterY + 2.0 * dy / (double)this->height() * ViewParameters.WinH;
            ViewParameters.apply(fCanvas);

            onViewChanged();
        }
    }
    else
    {
        //move
        //qDebug() << "mouse plain move event"<<event->x() << event->y();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        fCanvas->HandleInput(kMouseMotion, event->pos().x(), event->pos().y());
#else
        fCanvas->HandleInput(kMouseMotion, event->position().x(), event->position().y()); // !!!*** Round()?
#endif
    }
    //qDebug() << "done";
}

void ARasterWindow::mousePressEvent(QMouseEvent *event)
{
    if (!fCanvas) return;
    fCanvas->cd();
    if (BlockEvents) return;
    //qDebug() << "Mouse press event";
    PressEventRegistered = true;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (event->button() == Qt::LeftButton)  fCanvas->HandleInput(kButton1Down, event->pos().x(), event->pos().y());
    if (event->button() == Qt::RightButton) fCanvas->HandleInput(kButton3Down, event->pos().x(), event->pos().y());
#else
    if (event->button() == Qt::LeftButton)  fCanvas->HandleInput(kButton1Down, event->position().x(), event->position().y()); // !!!*** Round()?
    if (event->button() == Qt::RightButton) fCanvas->HandleInput(kButton3Down, event->position().x(), event->position().y());
#endif

    if (!fCanvas->HasViewer3D() || !fCanvas->GetView()) return;
    if (event->button() == Qt::MiddleButton)
    {
        //fCanvas->HandleInput(kButton2Down, event->x(), event->y());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        lastX = event->pos().x();
        lastY = event->pos().y();
#else
        LastX = event->position().x(); // !!!***
        LastY = event->position().y();
#endif
        Double_t viewSizeX, viewSizeY;
        fCanvas->cd();
        fCanvas->GetView()->GetWindow(LastCenterX, LastCenterY, viewSizeX, viewSizeY);
        BlockZoom = true;
    }
    // qDebug() << "done";

    // future: maybe it is possible to hide menu on left click using signal-slot mechanism?
    //TQObject::Connect("TGPopupMenu", "PoppedDown()", "TCanvas", fCanvas, "Update()");
}

#include <QTimer>
void ARasterWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (!fCanvas) return;
    fCanvas->cd();
    if (BlockEvents) return;
    if (!PressEventRegistered) return;
    PressEventRegistered = false;
    if (event->button() == Qt::LeftButton)
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        fCanvas->HandleInput(kButton1Up, event->pos().x(), event->pos().y());
#else
        fCanvas->HandleInput(kButton1Up, event->position().x(), event->position().y()); // !!!***
#endif
        emit LeftMouseButtonReleased();
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    else if (event->button() == Qt::RightButton) fCanvas->HandleInput(kButton3Up, event->pos().x(), event->pos().y());
#else
    else if (event->button() == Qt::RightButton) fCanvas->HandleInput(kButton3Up, event->position().x(), event->position().y()); // !!!***
#endif
    else if (event->button() == Qt::MiddleButton) QTimer::singleShot(300, this, &ARasterWindow::releaseZoomBlock);
}

void ARasterWindow::wheelEvent(QWheelEvent *event)
{
    if (!fCanvas) return;
    if (BlockEvents || BlockZoom) return;

    if (!fCanvas->HasViewer3D() || !fCanvas->GetView()) return;
    fCanvas->cd();

    ViewParameters.read(fCanvas);
    double oldX0 = ViewParameters.WinX;
    double oldY0 = ViewParameters.WinY;
    //double RotCenterX = ViewParameters.RotCenter[0];
    //double RotCenterY = ViewParameters.RotCenter[1];

    double factor = ( event->angleDelta().y() < 0 ? 1.25 : 0.8 );

    setVisible(false);
    fCanvas->GetView()->ZoomView(0, 1.0/factor);
    setVisible(true);

    ViewParameters.read(fCanvas); //after zoom X0,Y0 will become 0, RotCenter remains unchanged
    ViewParameters.WinX = oldX0;
    ViewParameters.WinY = oldY0;
    ViewParameters.apply(fCanvas);

    onViewChanged();
}

void ARasterWindow::paintEvent(QPaintEvent * /*event*/)
{
    if (fCanvas)
    {
        fCanvas->Resize();
        fCanvas->Update();
    }
}

void ARasterWindow::resizeEvent(QResizeEvent * event)
{
    if (fCanvas)
    {
        fCanvas->SetCanvasSize(event->size().width(), event->size().height());
        fCanvas->Resize();
        fCanvas->Update();
    }
}

void ARasterWindow::forceResize()
{
    if (fCanvas)
    {
        fCanvas->Resize();
        //fCanvas->Modified();
        fCanvas->Update();
    }
}

void ARasterWindow::saveAs(const QString & filename)
{
    fCanvas->SaveAs(filename.toLocal8Bit().data());
}

void ARasterWindow::setWindowProperties()
{
    ViewParameters.apply(fCanvas);
}

void ARasterWindow::onViewChanged()
{
    fCanvas->cd();
    ViewParameters.read(fCanvas);
    emit userChangedWindow();
}

void AGeoViewParameters::read(TCanvas * Canvas)
{
    TView3D * v = dynamic_cast<TView3D*>(Canvas->GetView());
    if (!v) return;

    v->GetRange(RangeLL, RangeUR);

    for (int i = 0; i < 3; i++)
        RotCenter[i] = 0.5 * (RangeUR[i] + RangeLL[i]);

    v->GetWindow(WinX, WinY, WinW, WinH);

    Lat  = v->GetLatitude();
    Long = v->GetLongitude();
    Psi  = v->GetPsi();
}

void AGeoViewParameters::apply(TCanvas *Canvas) const
{
    TView3D * v = dynamic_cast<TView3D*>(Canvas->GetView());
    if (!v) return;

    v->SetRange(RangeLL, RangeUR);
    v->SetWindow(WinX, WinY, WinW, WinH);
    int err;
    v->SetView(Long, Lat, Psi, err);

    Canvas->Modified();
    Canvas->Update();
}
