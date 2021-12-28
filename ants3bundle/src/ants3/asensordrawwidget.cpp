#include "asensordrawwidget.h"
#include "ui_asensordrawwidget.h"
#include "asensorviewer.h"
#include "asensorgview.h"
#include "asensorhub.h"
#include "ageoobject.h"
#include "ageoshape.h"

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDebug>

#include <math.h>

ASensorDrawWidget::ASensorDrawWidget(const std::vector<float> & sensorSignals, QWidget *parent) :
    QWidget(parent), SensorSignals(sensorSignals),
    ui(new Ui::ASensorDrawWidget)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->gvOut->setScene(scene);
    ui->gvOut->setDragMode(QGraphicsView::ScrollHandDrag); //if zoomed, can use hand to center needed area
    ui->gvOut->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->gvOut->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->gvOut->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->gvOut->setRenderHints(QPainter::Antialiasing);

    for (int i = 0; i < 16; i++)
    {
        QLabel * l = new QLabel();
        Labels.push_back(l);
    }
}

ASensorDrawWidget::~ASensorDrawWidget()
{
    delete ui;
}

void ASensorDrawWidget::clearGrItems()
{
    for (int i=0; i<grItems.size(); i++)
    {
        scene->removeItem(grItems[i]);
        delete grItems[i];
    }
    grItems.clear();
}

void ASensorDrawWidget::updateGui()
{
    const int numSensors = SensorSignals.size();

    scene->clear();

    float MaxSignal = 0;
    for (int i = 0; i < numSensors; i++)
        if (SensorSignals[i] > MaxSignal) MaxSignal = SensorSignals[i];
    if (MaxSignal < 1.0e-25) MaxSignal = 1.0;

    updateLegend(MaxSignal);
    addSensorItems(MaxSignal);
    //if (ui->cbShowPMsignals->isChecked())
    addTextItems(MaxSignal);
}

void ASensorDrawWidget::updateLegend(float MaxSignal)
{
    for (int i = 0; i < 16; i++)
    {
        QColor color;
        int level = 8 + i * 16;
        if      (level < 64)  color.setRgb(0,             level*4,           255);
        else if (level < 128) color.setRgb(0,             255,               255-(level-64)*4);
        else if (level < 192) color.setRgb((level-128)*4, 255,               0);
        else                  color.setRgb(255,           255-(level-192)*4, 0);

        QBrush brush(Qt::white);
        brush.setColor(color);

        QLabel * l = Labels[i];
        QPalette palette = l->palette();
        palette.setColor(l->backgroundRole(), color);
        //palette.setColor(l->foregroundRole(), Qt::yellow);
        l->setPalette(palette);

        QString txt = "";
        if (MaxSignal > 0)
        {
            switch (i)
            {
                case 0  : txt = "0";                            break;
                case 8  : txt = QString::number(0.5*MaxSignal); break;
                case 15 : txt = QString::number(MaxSignal);     break;
                default :;
            }
        }
        if (!txt.isEmpty()) l->setText(txt);
    }
}

void ASensorDrawWidget::resetViewport()
{
    const ASensorHub & SensorHub = ASensorHub::getConstInstance();
    const int numSensors = SensorHub.countSensors();

    double Xmin =  1e10;
    double Xmax = -1e10;
    double Ymin =  1e10;
    double Ymax = -1e10;
    for (int iSens = 0; iSens < numSensors; iSens++)
    {
        const double x =  SensorHub.SensorData[iSens].Position[0];
        const double y = -SensorHub.SensorData[iSens].Position[1];

        AGeoObject * obj = SensorHub.SensorData[iSens].GeoObj;
        const double size = (obj->Shape ? obj->Shape->maxSize() : 0);

        if (x - size < Xmin) Xmin = x - size;
        if (x + size > Xmax) Xmax = x + size;
        if (y - size < Ymin) Ymin = y - size;
        if (y + size > Ymax) Ymax = y + size;
    }

    const double Xdelta = Xmax - Xmin;
    const double Ydelta = Ymax - Ymin;

    const double frac = 1.05;
    scene->setSceneRect( (Xmin - frac*Xdelta)*GVscale,
                         (Ymin - frac*Ydelta)*GVscale,
                         ( (2.0*frac + 1.0) * Xdelta)*GVscale,
                         ( (2.0*frac + 1.0) * Ydelta)*GVscale );

    ui->gvOut->fitInView( (Xmin - 0.01*Xdelta)*GVscale,
                      (Ymin - 0.01*Ydelta)*GVscale,
                      (1.02*Xdelta)*GVscale,
                      (1.02*Ydelta)*GVscale,
                      Qt::KeepAspectRatio);
}

void ASensorDrawWidget::addSensorItems(float MaxSignal)
{
    const ASensorHub & SensorHub = ASensorHub::getConstInstance();
    const int numSensors = SensorHub.countSensors();

    for (int iSens = 0; iSens < numSensors; iSens++)
    {
        AGeoObject * obj = SensorHub.SensorData[iSens].GeoObj;
        if (!obj->Shape)
        {
            qWarning() << "Shape is not defined for sensor #" << iSens;
            continue;
        }

        //pen
        //int size = 6.0 * std::min(MW->PMs->SizeX(ipm), MW->PMs->SizeY(ipm)) / 30.0;
        double minSize = obj->Shape->minSize();
        if (minSize == 0) minSize = 1.0; // !!!***
        QPen pen(Qt::black);
        pen.setWidth(6.0 * minSize / 30.0);

        //brush
        QBrush brush(Qt::white);

        const float sig = SensorSignals[iSens];

        if (sig > 0)
        {
            QColor color = QColor(0,0,0);
            int level = 255.0*sig/MaxSignal;
            if (level>255) level = 255;
            //            qDebug()<<"Level = "<<level;

            if (level <64) color.setRgb(0, level*4,255);
            else if (level < 128) color.setRgb(0,255,255 - (level-64)*4);
            else if (level < 192) color.setRgb((level-128)*4,255,0);
            else color.setRgb(255,255 -(level-192)*4,0);

            brush.setColor(color);
        }
        else
        {
            if (sig == 0) brush.setColor(Qt::white);
            else brush.setColor(Qt::black);
        }

        QGraphicsItem * item = nullptr;
        const QString shapeType = obj->Shape->getShapeType();
        if (shapeType == "TGeoBBox")
        {
            AGeoBox * box = static_cast<AGeoBox*>(obj->Shape);
            double sizex = box->dx * GVscale;
            double sizey = box->dy * GVscale;
            item = scene->addRect(-sizex, -sizey, 2.0*sizex, 2.0*sizey, pen, brush);
        }
        else if (shapeType == "AGeoTube")
        {
            AGeoTube * tube = static_cast<AGeoTube*>(obj->Shape);
            double radius = tube->rmax * GVscale;
            item = scene->addEllipse( -radius, -radius, 2.0*radius, 2.0*radius, pen, brush);
        }
        /*
        else if !!!***polygon!
        {
            double radius = 0.5*tp->SizeX*GVscale;
            QPolygon polygon;
            for (int j=0; j<7; j++)
            {
                double angle = 3.1415926535/3.0 * j + 3.1415926535/2.0;
                double x = radius * cos(angle);
                double y = radius * sin(angle);
                polygon << QPoint(x, y);
            }
            item = scene->addPolygon(polygon, pen, brush);
        }
        */
        else
        {
            qDebug() << "Representing" << shapeType << "shaped sensor with a square of size 20 mm";
            double size = 10.0;
            item = scene->addRect(-size, -size, 2.0*size, 2.0*size, pen, brush);
        }

        const AVector3 & Pos = SensorHub.SensorData[iSens].Position;
        //if (ui->cbViewFromBelow->isChecked()) tmp->setZValue(-PM.z); else
        item->setZValue(Pos[2]);

        //if (PM.phi != 0) item->setRotation(-PM.phi);
        //else if (PM.psi != 0) item->setRotation(-PM.psi);

        item->setTransform(QTransform().translate(Pos[0] * GVscale, -Pos[1] * GVscale)); //minus!!!!
    }
}

void ASensorDrawWidget::addTextItems(float MaxSignal)
{
    const ASensorHub & SensorHub = ASensorHub::getConstInstance();
    const int numSensors = SensorHub.countSensors();

    const int prec = ui->sbDecimals->value();
    for (int iSens = 0; iSens < numSensors; iSens++)
    {
        AGeoObject * obj = SensorHub.SensorData[iSens].GeoObj;
        if (!obj->Shape)
        {
            qWarning() << "Shape is not defined for sensor #" << iSens;
            continue;
        }

        const double size = obj->Shape->minSize();

        const float sig = SensorSignals[iSens];
        QString text = QString::number(sig, 'g', prec);

        //color correction for dark blue
        QGraphicsSimpleTextItem *io = scene->addSimpleText(text);

        if (sig != 0 && sig < 0.15*MaxSignal)
            io->setBrush(Qt::white);

        int wid = (io->boundingRect().width()-6)/6;
        const AVector3 & Pos = SensorHub.SensorData[iSens].Position;
        double x = ( Pos[0] -wid*size*0.125 -0.15*size )*GVscale;
        double y = (-Pos[1] - 0.36*size) * GVscale; //minus y to invert the scale!!!
        io->setPos(x, y);
        io->setScale(0.4*size);

        //if (ui->cbViewFromBelow->isChecked()) io->setZValue(-PM.z+0.001); else
        io->setZValue(Pos[2] + 0.001);
    }
}

void ASensorDrawWidget::on_pbResetView_clicked()
{
    updateGui();
    resetViewport();
}

