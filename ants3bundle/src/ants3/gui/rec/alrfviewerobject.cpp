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
    Scene = new QGraphicsScene(this);
    connect(Scene, &QGraphicsScene::selectionChanged, this, &ALrfViewerObject::sceneSelectionChanged);

    GrView->setScene(Scene);
    GrView->setDragMode(QGraphicsView::ScrollHandDrag);               // when zoomed, one can use "hand" to center on the needed area
    GrView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    GrView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GrView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GrView->setRenderHints(QPainter::Antialiasing);

    ALrfViewerObject::ResetViewport();
}

ALrfViewerObject::~ALrfViewerObject()
{
    connect(Scene, &QGraphicsScene::selectionChanged, this, &ALrfViewerObject::sceneSelectionChanged);

    if (SensIcons.size() > 0)
        for (size_t i = 0; i < SensIcons.size(); i++)
            Scene->removeItem(SensIcons[i]);

    SensIcons.resize(0);
    Scene->clear();
    delete Scene; Scene = nullptr;
}

#include "ageoobject.h"
#include "guitools.h"
void ALrfViewerObject::DrawAll()
{
    int NumPMs = SensHub.countSensors();
    if (CursorMode == 1) GrView->setCursor(Qt::CrossCursor);

    //      qDebug()<<"Starting update of graphics";
    //scene->blockSignals(true);

    if (SensIcons.size() > 0)
        for (size_t i = 0; i < SensIcons.size(); i++)
            Scene->removeItem(SensIcons[i]);

    SensIcons.resize(0);
    Scene->clear();
    //      qDebug()<<"Scene cleared";

    SensProps.resize(NumPMs);
    //asserted size of PM properies vector

    // Drawing sensors
    for (int ipm = 0; ipm < NumPMs; ipm++)
    {
        const ASensorData * sensor = SensHub.getSensorData(ipm);
        if (!sensor) continue;

        QBrush brush(SensProps[ipm].brush);
        QGraphicsItem * tmp = guitools::addGeoObjectToScene(sensor->GeoObj, Scene, GVscale, brush);

        tmp->setPos(sensor->Position[0] * GVscale, -sensor->Position[1] * GVscale);
        tmp->setZValue(sensor->Position[2]);
        tmp->setVisible(SensProps.at(ipm).visible);

        tmp->setFlag(QGraphicsItem::ItemIsSelectable);
        if      (CursorMode == 0) tmp->setCursor(Qt::PointingHandCursor);
        else if (CursorMode == 1) tmp->setCursor(Qt::CrossCursor);

        SensIcons.push_back(tmp);
    }

    // Adding signal texts
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
