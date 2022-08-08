#include "agridhub.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ageoshape.h"
#include "agridelementrecord.h"
#include "ageometryhub.h"

#include <QDebug>

AGridHub::~AGridHub()
{
    clear();
}

void AGridHub::clear()
{
    for (AGridElementRecord * gr : GridRecords) delete gr;
    GridRecords.clear();
}

AGridHub & AGridHub::getInstance()
{
    static AGridHub instance;
    return instance;
}

const AGridHub & AGridHub::getConstInstance()
{
    return getInstance();
}

AGridElementRecord * AGridHub::createGridRecord(AGeoObject * obj)
{
    AGeoObject * geObj = obj->getGridElement();
    if (!geObj) return nullptr;
    if (!geObj->Type->isGridElement()) return nullptr;

    ATypeGridElementObject * GE = static_cast<ATypeGridElementObject*>(geObj->Type);
    return new AGridElementRecord(GE->shape, GE->size1, GE->size2);
}

void AGridHub::convertObjToGrid(AGeoObject * obj)
{
    delete obj->Type; obj->Type = new ATypeGridObject();
    QString name = obj->Name;

    //grid element inside
    AGeoObject* elObj = new AGeoObject();
    delete elObj->Type;
    ATypeGridElementObject * GE = new ATypeGridElementObject();
    elObj->Type = GE;
    GE->dz = obj->Shape->getHeight();
    if (GE->dz == 0) GE->dz = 1.001;
    elObj->Name = "GridElement_" + name;
    elObj->color = 1;
    obj->addObjectFirst(elObj);
    elObj->updateGridElementShape();

    shapeGrid(obj, 1, 10, 10, 1, 0);
        //mesh - 1, pitchX, pitchY, wireDiameter
}

void AGridHub::shapeGrid(AGeoObject *obj, int shape, double p0, double p1, double p2, int wireMat)
{
    //qDebug() << "Grid shape request:"<<shape<<p0<<p1<<p2;
    if (!obj) return;
    if (!obj->Type->isGrid()) return;

    AGeoObject * GEobj = obj->getGridElement();
    if (!GEobj) return;

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();
    ATypeGridElementObject * GE = static_cast<ATypeGridElementObject*>(GEobj->Type);

    //clear anything which is inside grid element
    for (int i = GEobj->HostedObjects.size() - 1; i > -1; i--)
    {
        //qDebug() << "...."<< GEobj->HostedObjects[i]->Name;
        GEobj->HostedObjects[i]->clearAll();
    }
    GEobj->HostedObjects.clear();

    obj->Shape->setHeight(0.5*p2 + 0.001); // !!!*** hardcoded number!
    GE->shape = shape;
    GE->dz    = 0.5 * p2 + 0.001;
    GE->size1 = 0.5 * p0;
    GE->size2 = 0.5 * p1;

    switch (shape)
    {
    case 0: // parallel wires
    {
        AGeoObject * w = new AGeoObject(new ATypeSingleObject(), new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, 90, 270));
        w->Position[0] = 0.5 * p0;
        w->Orientation[1] = 90.0;
        w->Material = wireMat;
        w->Name = GeoHub.generateObjectName("Wire");
        GEobj->addObjectFirst(w);

        w = new AGeoObject(new ATypeSingleObject(), new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, -90, 90));
        w->Position[0] = -0.5 * p0;
        w->Orientation[1] = 90.0;
        w->Material = wireMat;
        w->Name = AGeometryHub::getInstance().generateObjectName("Wire");
        GEobj->addObjectFirst(w);

        break;
    }
    case 1: // mesh
    {
        AGeoObject* com = new AGeoObject();
        com->Name = GeoHub.generateObjectName("Wires");
        GeoHub.convertObjToComposite(com);
        com->Material = wireMat;
        com->clearCompositeMembers();
        AGeoObject * logicals = com->getContainerWithLogical();

        AGeoObject * w1 = new AGeoObject();
        w1->Name = GeoHub.generateObjectName("Wire_1");
        delete w1->Shape; w1->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, -90.0, 90.0);
        w1->Position[0] = -0.5 * p0;
        w1->Orientation[1] = 90.0;
        w1->Material = wireMat;
        logicals->addObjectFirst(w1);

        AGeoObject * w2 = new AGeoObject();
        w2->Name = GeoHub.generateObjectName("Wire_2");
        delete w2->Shape; w2->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, 90.0, 270.0);
        w2->Position[0] = 0.5 * p0;
        w2->Orientation[1] = 90.0;
        w2->Material = wireMat;
        logicals->addObjectLast(w2);

        AGeoObject * w3 = new AGeoObject();
        w3->Name = GeoHub.generateObjectName("Wire_3");
        delete w3->Shape; w3->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p0, 90.0, 270.0);
        w3->Position[1] = 0.5 * p1;
        w3->Orientation[0] = 90.0;
        w3->Orientation[1] = 90.0;
        w3->Material = wireMat;
        logicals->addObjectLast(w3);

        AGeoObject * w4 = new AGeoObject();
        w4->Name = GeoHub.generateObjectName("Wire_4");
        delete w4->Shape; w4->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p0, -90.0, 90.0);
        w4->Position[1] = -0.5 * p1;
        w4->Orientation[0] = 90.0;
        w4->Orientation[1] = 90.0;
        w4->Material = wireMat;
        logicals->addObjectLast(w4);

        AGeoComposite * comSh = static_cast<AGeoComposite*>(com->Shape);
        comSh->GenerationString = "TGeoCompositeShape( " +
                w1->Name + " + " +
                w2->Name + " + " +
                w3->Name + " + " +
                w4->Name + " )";

        GEobj->addObjectFirst(com);
        break;
    }
    case 2: //hexagon
    {
        AGeoObject* com = new AGeoObject();
        com->Name = GeoHub.generateObjectName("Wires");
        com->Material = wireMat;
        GeoHub.convertObjToComposite(com);
        com->clearCompositeMembers();
        AGeoObject * logicals = com->getContainerWithLogical();

        //qDebug() << "p0, p1, p2"<<p0<<p1<<p2;
        double d = 0.5 * p0 / 0.86603; //radius of circumscribed circle
        double delta = 0.5 * (p0 - p1); // wall thickness
        //qDebug() << "d, delta:"<<d << delta;
        double dd = d + 2.0*delta*0.57735;
        double x = 0.5*(p0-delta)*0.866025;

        AGeoObject * w1 = new AGeoObject();
        w1->Name = GeoHub.generateObjectName("Wire_1");
        delete w1->Shape; w1->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w1->Position[1] = 0.5*p0 - 0.5*delta;
        w1->Orientation[1] = 90.0;
        w1->Material = wireMat;
        logicals->addObjectFirst(w1);

        AGeoObject * w2 = new AGeoObject();
        w2->Name = GeoHub.generateObjectName("Wire_2");
        delete w2->Shape; w2->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w2->Position[1] = -0.5*p0 + 0.5*delta;
        w2->Orientation[0] = 180.0;
        w2->Orientation[1] = 90.0;
        w2->Material = wireMat;
        logicals->addObjectFirst(w2);

        AGeoObject * w3 = new AGeoObject();
        w3->Name = GeoHub.generateObjectName("Wire_3");
        delete w3->Shape; w3->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w3->Position[0] = -x;
        w3->Position[1] = 0.5*(0.5*p0 - 0.5*delta);
        w3->Orientation[0] = 60.0;
        w3->Orientation[1] = 90.0;
        w3->Material = wireMat;
        logicals->addObjectFirst(w3);

        AGeoObject * w4 = new AGeoObject();
        w4->Name = GeoHub.generateObjectName("Wire_4");
        delete w4->Shape; w4->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w4->Position[0] = x;
        w4->Position[1] = 0.5*(0.5*p0 - 0.5*delta);
        w4->Orientation[0] = -60.0;
        w4->Orientation[1] = 90.0;
        w4->Material = wireMat;
        logicals->addObjectFirst(w4);

        AGeoObject * w5 = new AGeoObject();
        w5->Name = GeoHub.generateObjectName("Wire_5");
        delete w5->Shape; w5->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w5->Position[0] = -x;
        w5->Position[1] = -0.5*(0.5*p0 - 0.5*delta);
        w5->Orientation[0] = 120.0;
        w5->Orientation[1] = 90.0;
        w5->Material = wireMat;
        logicals->addObjectFirst(w5);

        AGeoObject * w6 = new AGeoObject();
        w6->Name = GeoHub.generateObjectName("Wire_6");
        delete w6->Shape; w6->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w6->Position[0] = x;
        w6->Position[1] = -0.5*(0.5*p0 - 0.5*delta);
        w6->Orientation[0] = -120.0;
        w6->Orientation[1] = 90.0;
        w6->Material = wireMat;
        logicals->addObjectFirst(w6);

        AGeoComposite * comSh = static_cast<AGeoComposite*>(com->Shape);
        comSh->GenerationString = "TGeoCompositeShape( " +
                w1->Name + " + " +
                w2->Name + " + " +
                w3->Name + " + " +
                w4->Name + " + " +
                w5->Name + " + " +
                w6->Name + " )";

        GEobj->addObjectFirst(com);
        break;
    }

    default:
        qWarning() << "Unknown grid element shape!";
        return;
    }
    GEobj->updateGridElementShape();
}
