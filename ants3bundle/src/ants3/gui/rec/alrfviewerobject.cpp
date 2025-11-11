#include "alrfviewerobject.h"
#include "asensorhub.h"
#include "alrfgraphicsview.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPolygonF>
#include <QDebug>

#include <math.h>

ALrfViewerObject::ALrfViewerObject(ALrfGraphicsView *GV) :
    SensHub(ASensorHub::getInstance()), GrView(GV)
{
    GVscale = 1.0;
    CursorMode = 0;
    Scene = new QGraphicsScene(this);
    connect(Scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

    GrView->setScene(Scene);
    GrView->setDragMode(QGraphicsView::ScrollHandDrag); //if zoomed, can use hand to center needed area
    //gvOut->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    GrView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    GrView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GrView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GrView->setRenderHints(QPainter::Antialiasing);

    ALrfViewerObject::ResetViewport();
}

ALrfViewerObject::~ALrfViewerObject()
{
    disconnect(Scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

    if (SensIcons.size() > 0)
        for (int i=0; i<SensIcons.size(); i++) Scene->removeItem(SensIcons[i]);
    SensIcons.resize(0);
    Scene->clear();
    delete Scene;
    Scene = 0;
}

#include "ageoobject.h"
void ALrfViewerObject::DrawAll()
{
    int NumPMs = SensHub.countSensors();
    if (CursorMode == 1) GrView->setCursor(Qt::CrossCursor);

    //      qDebug()<<"Starting update of graphics";
    //scene->blockSignals(true);

    if (SensIcons.size() > 0)
        for (int i=0; i<SensIcons.size(); i++) Scene->removeItem(SensIcons[i]);
    SensIcons.resize(0);
    Scene->clear();
    //      qDebug()<<"Scene cleared";

    SensProps.resize(NumPMs);
    //asserted size of PM properies vector

    //============================ drawing PMs ===================================
    for (int ipm = 0; ipm < NumPMs; ipm++)
    {
        //const APm &PM = SensHub.at(ipm);
        const ASensorData * PM = SensHub.getSensorData(ipm);
        if (!PM) continue;

        //PM object pen
        QPen pen(SensProps[ipm].pen);

        //int size = 6.0 * PMs->SizeX(ipm) / 300.0;
        int size = 6.0 * PM->GeoObj->getMaxSize() / 300.0; // !!!***
        pen.setWidth(size);

        //PM object brush
        QBrush brush(SensProps[ipm].brush);

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
        tmp = Scene->addEllipse( -0.5*diameter, -0.5*diameter, diameter, diameter, pen, brush);
        // temporarly!

        tmp->setZValue(PM->Position[2]);
        tmp->setVisible(SensProps.at(ipm).visible);

        //tmp->setRotation(-PM.psi); // !!!***

        //tmp->setPos(PM.x*GVscale, -PM.y*GVscale);
        tmp->setPos(PM->Position[0] * GVscale, -PM->Position[1] * GVscale);

        tmp->setFlag(QGraphicsItem::ItemIsSelectable);
        if      (CursorMode == 0) tmp->setCursor(Qt::PointingHandCursor);
        else if (CursorMode == 1) tmp->setCursor(Qt::CrossCursor);

        SensIcons.push_back(tmp);
    }
    //      qDebug()<<"PM objects set, number="<<PMicons.size();


    //======================= PM signal text ===========================
    if (true)
    {
        for (int ipm = 0; ipm < NumPMs; ipm++)
        {
            const ASensorData * PM = SensHub.getSensorData(ipm);
            if (!PM) continue;

            QGraphicsTextItem * io = new QGraphicsTextItem();
            //double size = 0.5*PMs->getType( PMs->at(ipm).type )->SizeX;
            double size = 0.5 * PM->GeoObj->getMaxSize();  // !!!***
            io->setTextWidth(50);  // was 40
            io->setScale(0.04*size);

            //preparing text to show
            QString text = SensProps[ipm].text;

            text = "<CENTER>" + text + "</CENTER>";
            io->setDefaultTextColor(SensProps[ipm].textColor);
            io->setHtml(text);
            double x = ( PM->Position[0] - 1.0*size) * GVscale;  // was 0.75
            double y = (-PM->Position[1] - 0.5*size ) * GVscale; //minus y to invert the scale!!!
            io->setPos(x, y);

            io->setZValue(PM->Position[2] + 0.01);
            io->setVisible(SensProps.at(ipm).visible);

            Scene->addItem(io);
        }
    }

    //      qDebug()<<" update of graphics done";
}

void ALrfViewerObject::ResetViewport()
{
    int NumPMs = SensHub.countSensors();
    if (NumPMs == 0) return;

    //calculating viewing area
    double Xmin =  1e10;
    double Xmax = -1e10;
    double Ymin =  1e10;
    double Ymax = -1e10;
    for (int ipm = 0; ipm < NumPMs; ipm++)
    {
        const ASensorData * PM = SensHub.getSensorData(ipm);
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

    Scene->setSceneRect((Xmin - 0.1*Xdelta)*GVscale, (Ymin - 0.1*Ydelta)*GVscale, (Xmax-Xmin + 0.2*Xdelta)*GVscale, (Ymax-Ymin + 0.2*Ydelta)*GVscale);
    GrView->fitInView( (Xmin - 0.01*Xdelta)*GVscale, (Ymin - 0.01*Ydelta)*GVscale, (Xmax-Xmin + 0.02*Xdelta)*GVscale, (Ymax-Ymin + 0.02*Ydelta)*GVscale, Qt::KeepAspectRatio);
}

void ALrfViewerObject::ClearColors()
{
    SensProps.resize(0);
    SensProps.resize(SensHub.countSensors());
}

void ALrfViewerObject::SetPenColor(int ipm, QColor color)
{
    int NumPMs = SensHub.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (SensProps.size() < NumPMs) SensProps.resize(NumPMs);
    SensProps[ipm].pen = color;
}

void ALrfViewerObject::SetBrushColor(int ipm, QColor color)
{
    int NumPMs = SensHub.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (SensProps.size() < NumPMs) SensProps.resize(NumPMs);
    SensProps[ipm].brush = color;
}

void ALrfViewerObject::SetText(int ipm, QString text)
{
    int NumPMs = SensHub.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (SensProps.size() < NumPMs) SensProps.resize(NumPMs);
    SensProps[ipm].text = text;
}

void ALrfViewerObject::SetTextColor(int ipm, QColor color)
{
    int NumPMs = SensHub.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (SensProps.size() < NumPMs) SensProps.resize(NumPMs);
    SensProps[ipm].textColor = color;
}

void ALrfViewerObject::SetVisible(int ipm, bool fFlag)
{
    int NumPMs = SensHub.countSensors();
    if (ipm < 0 || ipm >= NumPMs) return;

    if (SensProps.size() < NumPMs) SensProps.resize(NumPMs);
    SensProps[ipm].visible = fFlag;
}

void ALrfViewerObject::SetCursorMode(int option)
{
    CursorMode = option; GrView->setCursorMode(option);
}

void ALrfViewerObject::forceResize()
{
    // qDebug() << "Force resize!";
    ALrfViewerObject::ResetViewport();
}

void ALrfViewerObject::sceneSelectionChanged()
{
    /*
    //  Not implemented?
    //  qDebug()<<"Scene selection changed!";
    int selectedItems = Scene->selectedItems().size();
    //  qDebug()<<"  --number of selected items:"<<selectedItems;
    if (selectedItems == 0)
    {
        //  qDebug()<<"  --empty - ignoring event";
        return;
    }

    QGraphicsItem * pointer = Scene->selectedItems().first();
    if (pointer == nullptr)
    {
        //  qDebug()<<"  --zero pinter, ignoring event!";
        return;
    }


    //int ipm = 0;
    //ipm = SensIcons.indexOf(pointer);
    //if (ipm == -1)
    //{
    //    //  qDebug()<<" --ipm not found!";
    //    return;
    //}
    //  qDebug()<<" -- ipm = "<<ipm;

    auto it = std::find(SensIcons.begin(), SensIcons.end(), pointer);
    if (it == SensIcons.end())
    {
        //  qDebug()<<" --ipm not found!";
        return;
    }

    QVector<int> result;
    for (int i = 0; i < Scene->selectedItems().size(); i++)
    {
        QGraphicsItem * pointer = Scene->selectedItems().at(i);
        if (pointer == nullptr) continue;
        //int ipm = SensIcons.indexOf(pointer);
        auto it = std::find(SensIcons.begin(), SensIcons.end(), pointer);
        int ipm = (it != SensIcons.end()) ? std::distance(SensIcons.begin(), it) : -1;
        if (ipm == -1) continue;
        result.append(ipm);
    }
    emit PMselectionChanged(result);

    //  qDebug()<<"scene selection processing complete";
    */
}
