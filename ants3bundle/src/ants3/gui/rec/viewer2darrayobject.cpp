#include "viewer2darrayobject.h"
#include "asensorhub.h"
#include "myqgraphicsview.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPolygonF>
#include <QDebug>

#include <math.h>

Viewer2DarrayObject::Viewer2DarrayObject(myQGraphicsView *GV) :
    PMs(ASensorHub::getInstance()), gv(GV)
{
    GVscale = 1.0;
    CursorMode = 0;
    scene = new QGraphicsScene(this);
    connect(scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

    gv->setScene(scene);
    gv->setDragMode(QGraphicsView::ScrollHandDrag); //if zoomed, can use hand to center needed area
    //gvOut->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    gv->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gv->setRenderHints(QPainter::Antialiasing);

    Viewer2DarrayObject::ResetViewport();
}

Viewer2DarrayObject::~Viewer2DarrayObject()
{
    disconnect(scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

    if (PMicons.size() > 0)
        for (int i=0; i<PMicons.size(); i++) scene->removeItem(PMicons[i]);
    PMicons.resize(0);
    scene->clear();
    delete scene;
    scene = 0;
}

#include "ageoobject.h"
void Viewer2DarrayObject::DrawAll()
{
    int NumPMs = PMs.countSensors();
    if (CursorMode == 1) gv->setCursor(Qt::CrossCursor);

    //      qDebug()<<"Starting update of graphics";
    //scene->blockSignals(true);

    if (PMicons.size() > 0)
        for (int i=0; i<PMicons.size(); i++) scene->removeItem(PMicons[i]);
    PMicons.resize(0);
    scene->clear();
    //      qDebug()<<"Scene cleared";

    PMprops.resize(NumPMs);
    //asserted size of PM properies vector

    //============================ drawing PMs ===================================
    for (int ipm = 0; ipm < NumPMs; ipm++)
    {
        //const APm &PM = PMs.at(ipm);
        const ASensorData * PM = PMs.getSensorData(ipm);
        if (!PM) continue;

        //PM object pen
        QPen pen(PMprops[ipm].pen);

        //int size = 6.0 * PMs->SizeX(ipm) / 300.0;
        int size = 6.0 * PM->GeoObj->getMaxSize() / 300.0; // !!!***
        pen.setWidth(size);

        //PM object brush
        QBrush brush(PMprops[ipm].brush);

        QGraphicsItem * tmp;
        // !!!*** TODO:
        /*
        if (PMs->getType(PM.type)->Shape == 0)
        {
            double sizex = PMs->getType(PM.type)->SizeX*GVscale;
            double sizey = PMs->getType(PM.type)->SizeY*GVscale;
            tmp = scene->addRect(-0.5*sizex, -0.5*sizey, sizex, sizey, pen, brush);
        }
        else if (PMs->getType(PM.type)->Shape == 1)
        {
            double diameter = PMs->getType(PM.type)->SizeX*GVscale;
            tmp = scene->addEllipse( -0.5*diameter, -0.5*diameter, diameter, diameter, pen, brush);
        }
        else
        {
            double radius = 0.5*PMs->getType(PM.type)->SizeX*GVscale;
            QPolygonF polygon;
            for (int j=0; j<7; j++)
            {
                double angle = 3.1415926535/3.0 * j + 3.1415926535/2.0;
                double x = radius * cos(angle);
                double y = radius * sin(angle);
                polygon.append( QPointF(x, y) );
            }
            tmp = scene->addPolygon(polygon, pen, brush);
        }
        */
        // temporarly!
        double diameter = PM->GeoObj->getMaxSize() * GVscale;
        tmp = scene->addEllipse( -0.5*diameter, -0.5*diameter, diameter, diameter, pen, brush);
        // temporarly!

        tmp->setZValue(PM->Position[2]);
        tmp->setVisible(PMprops.at(ipm).visible);

        //tmp->setRotation(-PM.psi); // !!!***

        //tmp->setPos(PM.x*GVscale, -PM.y*GVscale);
        tmp->setPos(PM->Position[0] * GVscale, -PM->Position[1] * GVscale);

        tmp->setFlag(QGraphicsItem::ItemIsSelectable);
        if      (CursorMode == 0) tmp->setCursor(Qt::PointingHandCursor);
        else if (CursorMode == 1) tmp->setCursor(Qt::CrossCursor);

        PMicons.append(tmp);
    }
    //      qDebug()<<"PM objects set, number="<<PMicons.size();


    //======================= PM signal text ===========================
    if (true)
    {
        for (int ipm = 0; ipm < NumPMs; ipm++)
        {
            const ASensorData * PM = PMs.getSensorData(ipm);
            if (!PM) continue;

            QGraphicsTextItem * io = new QGraphicsTextItem();
            //double size = 0.5*PMs->getType( PMs->at(ipm).type )->SizeX;
            double size = 0.5 * PM->GeoObj->getMaxSize();  // !!!***
            io->setTextWidth(40);
            io->setScale(0.04*size);

            //preparing text to show
            QString text = PMprops[ipm].text;

            text = "<CENTER>" + text + "</CENTER>";
            io->setDefaultTextColor(PMprops[ipm].textColor);
            io->setHtml(text);
            double x = ( PM->Position[0] - 0.75*size) * GVscale;
            double y = (-PM->Position[1] - 0.5*size ) * GVscale; //minus y to invert the scale!!!
            io->setPos(x, y);

            io->setZValue(PM->Position[2] + 0.01);
            io->setVisible(PMprops.at(ipm).visible);

            scene->addItem(io);
        }
    }

    //      qDebug()<<" update of graphics done";
}

void Viewer2DarrayObject::ResetViewport()
{
    int NumPMs = PMs.countSensors();
    if (NumPMs == 0) return;

    //calculating viewing area
    double Xmin =  1e10;
    double Xmax = -1e10;
    double Ymin =  1e10;
    double Ymax = -1e10;
    for (int ipm = 0; ipm < NumPMs; ipm++)
    {
        const ASensorData * PM = PMs.getSensorData(ipm);
        if (!PM) continue;

        double x = PM->Position[0];
        double y = PM->Position[1];
        //double size = PMs->getType(type)->SizeX;
        double size = PM->GeoObj->getMaxSize(); // !!!***

        if (x-size < Xmin) Xmin = x-size;
        if (x+size > Xmax) Xmax = x+size;
        if (y-size < Ymin) Ymin = y-size;
        if (y+size > Ymax) Ymax = y+size;
    }

    double Xdelta = Xmax-Xmin;
    double Ydelta = Ymax-Ymin;

    scene->setSceneRect((Xmin - 0.1*Xdelta)*GVscale, (Ymin - 0.1*Ydelta)*GVscale, (Xmax-Xmin + 0.2*Xdelta)*GVscale, (Ymax-Ymin + 0.2*Ydelta)*GVscale);
    gv->fitInView( (Xmin - 0.01*Xdelta)*GVscale, (Ymin - 0.01*Ydelta)*GVscale, (Xmax-Xmin + 0.02*Xdelta)*GVscale, (Ymax-Ymin + 0.02*Ydelta)*GVscale, Qt::KeepAspectRatio);
}

void Viewer2DarrayObject::ClearColors()
{
    PMprops.resize(0);
    PMprops.resize(PMs.countSensors());
}

void Viewer2DarrayObject::SetPenColor(int ipm, QColor color)
{
    int NumPMs = PMs.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (PMprops.size() < NumPMs) PMprops.resize(NumPMs);
    PMprops[ipm].pen = color;
}

void Viewer2DarrayObject::SetBrushColor(int ipm, QColor color)
{
    int NumPMs = PMs.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (PMprops.size() < NumPMs) PMprops.resize(NumPMs);
    PMprops[ipm].brush = color;
}

void Viewer2DarrayObject::SetText(int ipm, QString text)
{
    int NumPMs = PMs.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (PMprops.size() < NumPMs) PMprops.resize(NumPMs);
    PMprops[ipm].text = text;
}

void Viewer2DarrayObject::SetTextColor(int ipm, QColor color)
{
    int NumPMs = PMs.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (PMprops.size() < NumPMs) PMprops.resize(NumPMs);
    PMprops[ipm].textColor = color;
}

void Viewer2DarrayObject::SetVisible(int ipm, bool fFlag)
{
    int NumPMs = PMs.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (PMprops.size() < NumPMs) PMprops.resize(NumPMs);
    PMprops[ipm].visible = fFlag;
}

void Viewer2DarrayObject::SetCursorMode(int option)
{
    CursorMode = option; gv->setCursorMode(option);
}

void Viewer2DarrayObject::forceResize()
{
    qDebug()<<"resize!";
    Viewer2DarrayObject::ResetViewport();
}

void Viewer2DarrayObject::sceneSelectionChanged()
{
    //  qDebug()<<"Scene selection changed!";
    int selectedItems = scene->selectedItems().size();
    //  qDebug()<<"  --number of selected items:"<<selectedItems;
    if (selectedItems == 0)
    {
        //      qDebug()<<"  --empty - ignoring event";
        return;
    }

    QGraphicsItem* pointer = scene->selectedItems().first();
    if (pointer == 0)
    {
        //      qDebug()<<"  --zero pinter, ignoring event!";
        return;
    }

    int ipm = 0;
    ipm = PMicons.indexOf(pointer);
    if (ipm == -1)
    {
        //      qDebug()<<" --ipm not found!";
        return;
    }
    //  qDebug()<<" -- ipm = "<<ipm;

    QVector<int> result;
    for (int i=0; i<scene->selectedItems().size(); i++)
    {
        QGraphicsItem* pointer = scene->selectedItems().at(i);
        if (pointer == 0) continue;
        int ipm = 0;
        ipm = PMicons.indexOf(pointer);
        if (ipm == -1) continue;
        result.append(ipm);
    }
    emit PMselectionChanged(result);

    //  qDebug()<<"scene selection processing complete";
}
