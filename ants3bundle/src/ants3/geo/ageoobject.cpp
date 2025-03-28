#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "ageospecial.h"
#include "ajsontools.h"
#include "ageoconsts.h"
#include "aerrorhub.h"

#include <cmath>

#include <QDebug>
#include <QRegularExpression>

void AGeoObject::constructorInit()
{
    if (Name.isEmpty()) Name = generateRandomObjectName();

    Position[0] = Position[1] = Position[2] = 0;
    Orientation[0] = Orientation[1] = Orientation[2] = 0;
}

AGeoObject::AGeoObject(QString name, QString ShapeGenerationString)
{
    Type = new ATypeSingleObject();

    if (!name.isEmpty()) Name = name;
    color = -1;

    constructorInit();

    Shape = new AGeoBox();
    if (!ShapeGenerationString.isEmpty())
        readShapeFromString(ShapeGenerationString);
}

AGeoObject::AGeoObject(const QString & name, AGeoShape * shape)
{
    Type = new ATypeSingleObject();

    if (!name.isEmpty()) Name = name;
    color = -1;

    constructorInit();

    if (!shape) shape = new AGeoBox();
    Shape = shape;
}

AGeoObject::AGeoObject(const AGeoObject *objToCopy)
{
    constructorInit();

    Container = objToCopy->Container;
    Type = new ATypeSingleObject();

    Material = objToCopy->Material;

    Position[0] = objToCopy->Position[0];
    Position[1] = objToCopy->Position[1];
    Position[2] = objToCopy->Position[2] + 10;  // !!! to not overlap
    Orientation[0] = objToCopy->Orientation[0];
    Orientation[1] = objToCopy->Orientation[1];
    Orientation[2] = objToCopy->Orientation[2];

    style = objToCopy->style;
    width = objToCopy->style;

    bool ok = readShapeFromString(objToCopy->Shape->getGenerationString());
    if (!ok) Shape = new AGeoBox();
}

AGeoObject::AGeoObject(const QString & name, const QString & container, int iMat, AGeoShape * shape,  double x, double y, double z,  double phi, double theta, double psi)
{
    constructorInit();

    Type = new ATypeSingleObject();

    Name = name;
    tmpContName = container;
    Material = iMat;

    Position[0] = x;
    Position[1] = y;
    Position[2] = z;
    Orientation[0] = phi;
    Orientation[1] = theta;
    Orientation[2] = psi;

    Shape = shape;
}

AGeoObject::AGeoObject(const QString & name, const QString & container, int iMat, AGeoShape * shape, const std::array<double,3> & pos, const std::array<double,3> & ori)
{
    constructorInit();

    Type = new ATypeSingleObject();

    Name = name;
    tmpContName = container;
    Material = iMat;

    for (size_t i = 0; i < 3; i++)
    {
        Position[i]    = pos[i];
        Orientation[i] = ori[i];
    }

    Shape = shape;
}

AGeoObject::AGeoObject(AGeoType *objType, AGeoShape* shape)
{
    constructorInit();

    Type = objType;
    if (shape) Shape = shape;
    else Shape = new AGeoBox();
}

AGeoObject::~AGeoObject()
{
    delete Shape;
    delete Type;
}

bool AGeoObject::readShapeFromString(const QString & GenerationString, bool OnlyCheck)
{  
    QStringList sl = GenerationString.split(QString("("), Qt::SkipEmptyParts);
    if (sl.size()<2)
    {
        qWarning() << "Format error in Shape generation string!";
        return false;
    }
    QString type = sl.first().simplified();
    qDebug() <<"sl" <<sl <<"   type"<< type;

    AGeoShape* newShape = AGeoShape::GeoShapeFactory(type);
    if (!newShape)
    {
        qWarning() << "Failed to create Shape using generation string!";
        return false;
    }

    //Composite shape requires additional cofiguration, and it is not possible to convert to composite in this way!
    if (newShape->getShapeType() == "TGeoCompositeShape")
    {
        if (!Type->isComposite())
        {
            qWarning() << "Cannot convert to composite in this way, use context menu!";
            delete newShape;
            return false;
        }
        refreshShapeCompositeMembers(newShape);
    }

    bool ok = newShape->readFromString(GenerationString);
    if (!ok)
    {
        qWarning() << "Failed to read shape properties from the generation string!";
        delete newShape;
        return false;
    }

    if (OnlyCheck)
    {
        delete newShape;
        return true;
    }

    delete Shape; Shape = newShape;
    return true;
}

void AGeoObject::onMaterialRemoved(int imat)
{
    if (Material > imat) Material--;

    for (AGeoObject * obj : HostedObjects)
        obj->onMaterialRemoved(imat);
}

bool AGeoObject::isWorld() const
{
    if (!Type) return false;
    return Type->isWorld();
}

bool AGeoObject::isSensor() const
{
    if (!Role) return false;
    return dynamic_cast<AGeoSensor*>(Role);
}

bool AGeoObject::isCalorimeter() const
{
    if (!Role) return false;
    return dynamic_cast<AGeoCalorimeter*>(Role);
}

bool AGeoObject::isScintillator() const
{
    if (!Role) return false;
    return dynamic_cast<AGeoScint*>(Role);
}

bool AGeoObject::isParticleAnalyzer() const
{
    if (!Role) return false;
    return dynamic_cast<AGeoParticleAnalyzer*>(Role);
}

int AGeoObject::getMaterial() const
{
    if (Type->isHandlingArray() || Type->isHandlingSet())
        return Container->getMaterial();
    return Material;
}

const AGeoObject * AGeoObject::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    for (int i = 0; i < 3; i++)
    {
        if (PositionStr[i]   .contains(nameRegExp)) return this;
        if (OrientationStr[i].contains(nameRegExp)) return this;
    }
    if (Shape && Shape->isGeoConstInUse(nameRegExp)) return this;
    if (Type  && Type->isGeoConstInUse(nameRegExp))  return this;
    if (Role  && Role->isGeoConstInUse(nameRegExp))  return this;
    return nullptr;
}

void AGeoObject::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    for (int i = 0; i < 3; i++)
    {
        PositionStr[i]   .replace(nameRegExp, newName);
        OrientationStr[i].replace(nameRegExp, newName);
    }
    if (Shape) Shape->replaceGeoConstName(nameRegExp, newName);
    if (Type)  Type->replaceGeoConstName(nameRegExp, newName);
    if (Role)  Role->replaceGeoConstName(nameRegExp, newName);
}

const AGeoObject * AGeoObject::isGeoConstInUseRecursive(const QRegularExpression & nameRegExp) const
{
    //qDebug() <<"name of current "<<this->Name;
    if (isGeoConstInUse(nameRegExp)) return this;

    for (AGeoObject * hosted : HostedObjects)
    {
        const AGeoObject * obj = hosted->isGeoConstInUseRecursive(nameRegExp);
        if (obj) return obj;
    }
    return nullptr;
}

void AGeoObject::replaceGeoConstNameRecursive(const QRegularExpression & nameRegExp, const QString & newName)
{
    replaceGeoConstName(nameRegExp, newName);

    for (AGeoObject * hosted : HostedObjects)
        hosted->replaceGeoConstNameRecursive(nameRegExp, newName);
}

void AGeoObject::writeToJson(QJsonObject &json) const
{
    json["Name"] = Name;

    QJsonObject jj;
    Type->writeToJson(jj);
    json["Type"] = jj;
    json["Container"] = Container ? Container->Name : "";
    json["fActive"] = fActive;
    json["fExpanded"] = fExpanded;

    if ( Type->isHandlingStandard() || Type->isWorld())
    {
        json["Material"] = Material;
        json["color"] = color;
        json["style"] = style;
        json["width"] = width;

        if (!Type->isMonitor()) // monitor shape is in Type
        {
            json["Shape"] = Shape->getShapeType();
            QJsonObject js;
            Shape->writeToJson(js);
            json["ShapeSpecific"] = js;
        }
    }

    if ( Type->isHandlingStandard() || Type->isHandlingArray() || Type->isStack())
    {
        json["X"] = Position[0];
        json["Y"] = Position[1];
        json["Z"] = Position[2];
        json["Phi"]   = Orientation[0];
        json["Theta"] = Orientation[1];
        json["Psi"]   = Orientation[2];

        if (!PositionStr[0].isEmpty()) json["strX"] = PositionStr[0];
        if (!PositionStr[1].isEmpty()) json["strY"] = PositionStr[1];
        if (!PositionStr[2].isEmpty()) json["strZ"] = PositionStr[2];

        if (!OrientationStr[0].isEmpty()) json["strPhi"]   = OrientationStr[0];
        if (!OrientationStr[1].isEmpty()) json["strTheta"] = OrientationStr[1];
        if (!OrientationStr[2].isEmpty()) json["strPsi"]   = OrientationStr[2];
    }

    if (Role)
    {
        QJsonObject js;
        Role->writeToJson(js);
        json["SpecialRole"] = js;
    }
}

void AGeoObject::readFromJson(const QJsonObject & json)
{
    Name = json["Name"].toString();

    jstools::parseJson(json, "color", color);
    jstools::parseJson(json, "style", style);
    jstools::parseJson(json, "width", width);

    jstools::parseJson(json, "fActive", fActive);
    jstools::parseJson(json, "fExpanded", fExpanded);

    jstools::parseJson(json, "Container", tmpContName);
    //Hosted are not loaded, they are populated later

    jstools::parseJson(json, "Material", Material);
    jstools::parseJson(json, "X",     Position[0]);
    jstools::parseJson(json, "Y",     Position[1]);
    jstools::parseJson(json, "Z",     Position[2]);
    jstools::parseJson(json, "Phi",   Orientation[0]);
    jstools::parseJson(json, "Theta", Orientation[1]);
    jstools::parseJson(json, "Psi",   Orientation[2]);

    for (int i = 0; i < 3; i++)
    {
        PositionStr[i].clear();
        OrientationStr[i].clear();
    }

    jstools::parseJson(json, "strX",     PositionStr[0]);
    jstools::parseJson(json, "strY",     PositionStr[1]);
    jstools::parseJson(json, "strZ",     PositionStr[2]);
    jstools::parseJson(json, "strPhi",   OrientationStr[0]);
    jstools::parseJson(json, "strTheta", OrientationStr[1]);
    jstools::parseJson(json, "strPsi",   OrientationStr[2]);

    //Shape
    if (json.contains("Shape"))
    {
        QString ShapeType = json["Shape"].toString();
        QJsonObject js = json["ShapeSpecific"].toObject();

        Shape = AGeoShape::GeoShapeFactory(ShapeType);
        if (Shape) Shape->readFromJson(js);
        else
        {
            Shape = new AGeoBox();
            AErrorHub::addQError("Unknown shape " + ShapeType + " for object " + Name);
        }

        //composite: cannot update memebers at this phase - HostedObjects are not set yet
    }

    //Type
    QJsonObject jj = json["Type"].toObject();
    if (!jj.isEmpty())
    {
        QString tmpType;
        jstools::parseJson(jj, "Type", tmpType);
        AGeoType * newType = AGeoType::makeTypeObject(tmpType);
        if (newType)
        {
            delete Type; Type = newType;
            Type->readFromJson(jj);
        }
        else qDebug() << "Type read failed for object:" << Name << ", keeping default type"; // error added to AErrorHub
    }
    else qDebug() << "Type is empty for object:" << Name << ", keeping default type";

    // Special role
    delete Role; Role = nullptr;
    QJsonObject jsRole;
    if (jstools::parseJson(json, "SpecialRole", jsRole))
        Role = GeoRoleFactory::make(jsRole);
}

void AGeoObject::introduceGeoConstValues()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;

    if (!PositionStr[0].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, PositionStr[0], Position[0]);
        if (!ok) errorStr += "in X position\n";
    }
    if (!PositionStr[1].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, PositionStr[1], Position[1]);
        if (!ok) errorStr += "in Y position\n";
    }
    if (!PositionStr[2].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, PositionStr[2], Position[2]);
        if (!ok) errorStr += "in Z position\n";
    }

    if (!OrientationStr[0].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, OrientationStr[0], Orientation[0]);
        if (!ok) errorStr += "in Phi orientation\n";
    }
    if (!OrientationStr[1].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, OrientationStr[1], Orientation[1]);
        if (!ok) errorStr += "in Theta orientation\n";
    }
    if (!OrientationStr[2].isEmpty())
    {
        bool ok = GC.evaluateFormula(errorStr, OrientationStr[2], Orientation[2]);
        if (!ok) errorStr += "in Psi orientation\n";
    }

    if (Shape) Shape->introduceGeoConstValues(errorStr);
    if (Type)  Type->introduceGeoConstValues(errorStr);
    if (Role)  Role->introduceGeoConstValues(errorStr);

    if (!errorStr.isEmpty())
        AErrorHub::addQError(Name + ":\n" + errorStr);
}

void AGeoObject::introduceGeoConstValuesRecursive()
{
    introduceGeoConstValues();

    for (auto * obj : HostedObjects)
        obj->introduceGeoConstValuesRecursive();
}

void AGeoObject::writeAllToJarr(QJsonArray &jarr)
{
    QJsonObject js;
    writeToJson(js);
    jarr << js;

    if (!Type->isInstance())
    {
        for (AGeoObject * obj : HostedObjects)
            obj->writeAllToJarr(jarr);
    }
}

QString AGeoObject::readAllFromJarr(AGeoObject * World, const QJsonArray & jarr)
{
    QString ErrorString;
    const int size = jarr.size();
    if (size < 1)
    {
        ErrorString = "Read World tree: size cannot be < 1";
        qWarning() << ErrorString;
        return ErrorString;
    }

    QJsonObject worldJS = jarr[0].toObject();
    World->readFromJson(worldJS);

    AGeoObject * prevObj = World;
    for (int iObj = 1; iObj < size; iObj++)
    {
        AGeoObject * newObj = new AGeoObject();
        //qDebug() << "--record in array:"<<json;
        QJsonObject json = jarr[iObj].toObject();
        newObj->readFromJson(json);
        //qDebug() << "--read success, object name:"<<newObj->Name<< "Container:"<<newObj->tmpContName;

        AGeoObject * cont = prevObj->findContainerUp(newObj->tmpContName);
        if (cont)
        {
            newObj->Container = cont;
            cont->HostedObjects.push_back(newObj);
            //qDebug() << "Success! Registered"<<newObj->Name<<"in"<<newObj->tmpContName;
            if (newObj->isCompositeMemeber())
            {
                //qDebug() << "---It is composite member!";
                if (cont->Container)
                    cont->Container->refreshShapeCompositeMembers(); //register in the Shape
            }
            prevObj = newObj;
        }
        else
        {
            ErrorString = "ERROR reading geo objects! Not found container: " + newObj->tmpContName;
            qWarning() << ErrorString;
            delete newObj;
            return ErrorString;
        }
    }

    return "";
}

AGeoObject * AGeoObject::getContainerWithLogical()
{
    if ( !Type->isComposite()) return nullptr;

    for (AGeoObject * obj : HostedObjects)
    {
        if (obj->Type->isCompositeContainer())
            return obj;
    }
    return nullptr;
}

const AGeoObject *AGeoObject::getContainerWithLogical() const
{
    if ( !Type->isComposite()) return nullptr;

    for (AGeoObject * obj : HostedObjects)
    {
        if (obj->Type->isCompositeContainer())
            return obj;
    }
    return nullptr;
}

bool AGeoObject::isCompositeMemeber() const
{
    if (Container)
        return Container->Type->isCompositeContainer();
    return false;
}

void AGeoObject::refreshShapeCompositeMembers(AGeoShape* ExternalShape)
{
    if (Type->isComposite())
    {
        AGeoShape* ShapeToUpdate = ( ExternalShape ? ExternalShape : Shape );
        AGeoComposite* shape = dynamic_cast<AGeoComposite*>(ShapeToUpdate);
        if (shape)
        {
            shape->members.clear();
            AGeoObject* logicals = getContainerWithLogical();
            if (logicals)
                for (AGeoObject * obj : logicals->HostedObjects)
                    shape->members << obj->Name;
        }
    }
}

bool AGeoObject::isInUseByComposite()
{
    if (!Container) return false;
    if (!Container->Type->isCompositeContainer()) return false;

    AGeoObject* compo = Container->Container;
    if (!compo) return false;
    AGeoComposite* shape = dynamic_cast<AGeoComposite*>(compo->Shape);
    if (!shape) return false;

    return shape->GenerationString.contains(Name);
}

void AGeoObject::clearCompositeMembers()
{
    AGeoObject * logicals = getContainerWithLogical();
    if (!logicals) return;

    for (AGeoObject * obj : logicals->HostedObjects)
        obj->clearAll(); //overkill here
    logicals->HostedObjects.clear();
}

void AGeoObject::removeCompositeStructure()
{
    clearCompositeMembers();

    delete Type;
    Type = new ATypeSingleObject();

    for (auto it = HostedObjects.begin(); it != HostedObjects.end(); ++it)
    {
        if ( (*it)->Type->isCompositeContainer())
        {
            delete (*it);
            HostedObjects.erase(it);
            return;
        }
    }
}

void AGeoObject::updateNameOfLogicalMember(const QString & oldName, const QString & newName)
{
    if (Container && Container->Type->isCompositeContainer())
    {
        AGeoObject * Composite = Container->Container;
        if (Composite)
        {
            AGeoComposite * cs = dynamic_cast<AGeoComposite*>(Composite->Shape);
            if (cs)
            {
                cs->members.replaceInStrings(oldName, newName);
                cs->GenerationString.replace(oldName, newName);
            }
        }
    }
}

AGeoObject * AGeoObject::getGridElement()
{
    if (!Type->isGrid()) return nullptr;
    if (HostedObjects.empty()) return nullptr;

    AGeoObject * obj = HostedObjects.front();
    if (!obj->Type->isGridElement()) return nullptr;
    return obj;
}

void AGeoObject::updateGridElementShape()
{
    if (!Type->isGridElement()) return;

    ATypeGridElementObject* GE = static_cast<ATypeGridElementObject*>(Type);

    if (Shape) delete Shape;
    if (GE->shape == 0 || GE->shape == 1)
        Shape = new AGeoBox(GE->size1, GE->size2, GE->dz);
    else
        Shape = new AGeoPolygon(6, GE->dz, GE->size1, GE->size1);
}

const AMonitorConfig * AGeoObject::getMonitorConfig() const
{
    if (!Type) return nullptr;
    ATypeMonitorObject * mon = dynamic_cast<ATypeMonitorObject*>(Type);
    if (!mon) return nullptr;

    return &mon->config;
}

const ACalorimeterProperties * AGeoObject::getCalorimeterProperties() const
{
    if (!Role) return nullptr;
    const AGeoCalorimeter * gc = dynamic_cast<const AGeoCalorimeter*>(Role);
    if (!gc) return nullptr;
    return & gc->Properties;
}

#include "aphotonfunctionalmodel.h"
APhotonFunctionalModel * AGeoObject::getDefaultPhotonFunctionalModel()
{
    if (!Role) return nullptr;
    AGeoPhotonFunctional * pf = dynamic_cast<AGeoPhotonFunctional*>(Role);
    if (!pf) return nullptr;
    return pf->DefaultModel;
}

bool AGeoObject::isStackMember() const
{
    if (!Container || !Container->Type) return false;
    return Container->Type->isStack();
}

bool AGeoObject::isStackReference() const
{
    if (!Container || !Container->Type) return false;

    ATypeStackContainerObject * sc = dynamic_cast<ATypeStackContainerObject*>(Container->Type);
    if (sc && sc->ReferenceVolume == Name) return true;
    return false;
}

AGeoObject * AGeoObject::getOrMakeStackReferenceVolume()
{
    AGeoObject * Stack;
    ATypeStackContainerObject * StackTypeObj = dynamic_cast<ATypeStackContainerObject*>(Type);
    if (StackTypeObj) Stack = this;
    else
    {
        if (Container)
        {
            StackTypeObj = dynamic_cast<ATypeStackContainerObject*>(Container->Type);
            if (StackTypeObj) Stack = Container;
        }
    }
    if (!StackTypeObj) return nullptr;
    if (Stack->HostedObjects.empty()) return nullptr;

    AGeoObject * RefVolume = nullptr;
    if (StackTypeObj->ReferenceVolume.isEmpty())
    {
        RefVolume = Stack->HostedObjects.front();
        StackTypeObj->ReferenceVolume = RefVolume->Name;
    }
    else
    {
        for (AGeoObject * obj : Stack->HostedObjects)
        {
            if (obj->Name == StackTypeObj->ReferenceVolume)
            {
                RefVolume = obj;
                break;
            }
        }
        if (!RefVolume)
        {
            qWarning() << "Declared reference volume of the stack not found, setting first volume as the reference!";
            RefVolume = Stack->HostedObjects.front();
            StackTypeObj->ReferenceVolume = RefVolume->Name;
        }
    }
    return RefVolume;
}

AGeoObject * AGeoObject::findObjectByName(const QString & name)
{
    if (Name == name) return this;

    for (AGeoObject * obj : HostedObjects)
    {
        AGeoObject * tmp = obj->findObjectByName(name);
        if (tmp) return tmp;
    }
    return nullptr; //not found
}

void AGeoObject::findObjectsByWildcard(const QString & name, std::vector<AGeoObject*> & foundObjs)
{
    if (Name.startsWith(name, Qt::CaseSensitive)) foundObjs.push_back(this);

    for (AGeoObject * obj : HostedObjects)
        obj->findObjectsByWildcard(name, foundObjs);
}

void AGeoObject::changeLineWidthRecursive(int delta)
{
    width += delta;
    if (width < 1) width = 1;

    for (AGeoObject * obj : HostedObjects)
        obj->changeLineWidthRecursive(delta);
}

bool AGeoObject::isNameExists(const QString &name)
{
    return (findObjectByName(name)) ? true : false;
}

bool AGeoObject::isContainsLocked()
{
    for (AGeoObject * obj : HostedObjects)
    {
        if (obj->fLocked) return true;
        if (obj->isContainsLocked()) return true;
    }
    return false;
}

bool AGeoObject::isContainsObjectRecursive(const AGeoObject * otherObj)
{
    for (AGeoObject * obj : HostedObjects)
    {
        if (obj == otherObj) return true;
        if (obj->isContainsObjectRecursive(otherObj)) return true;
    }
    return false;
}

bool AGeoObject::isDisabled() const
{
    if (Type->isWorld()) return false;

    if (!fActive) return true;
    if (!Container) return false;
    return Container->isDisabled();
}

void AGeoObject::enableUp()
{
    if (Type->isWorld()) return;

    fActive = true;
    if (Container) Container->enableUp();
}

void AGeoObject::addObjectFirst(AGeoObject * Object)
{
    auto it = HostedObjects.begin();
    if (isWorld()) ++it; // world has a prototype container
    if (getContainerWithLogical()) ++it;
    if (getGridElement()) ++it;
    HostedObjects.insert(it, Object);
    Object->Container = this;
}

void AGeoObject::addObjectLast(AGeoObject * Object)
{
    Object->Container = this;
    HostedObjects.push_back(Object);
}

bool AGeoObject::migrateTo(AGeoObject * objTo, bool fAfter, AGeoObject *reorderObj)
{
    if (Type->isWorld()) return false;

    if (Container)
    {
        if (objTo != Container)
        {
            //check: cannot migrate down the chain (including to itself)
            //assuming nobody asks to migrate world
            if (Container && !objTo->Type->isWorld())
            {
                AGeoObject * obj = objTo;
                do
                {
                    if (obj == this) return false;
                    obj = obj->Container;
                }
                while (obj);
            }
        }
        //Container->HostedObjects.removeOne(this);
        for (auto it = Container->HostedObjects.begin(); it != Container->HostedObjects.end(); ++it)
            if (*it == this)
            {
                Container->HostedObjects.erase(it);
                break;
            }
    }

    if (fAfter) objTo->addObjectLast(this);
    else        objTo->addObjectFirst(this);

    if (reorderObj) return repositionInHosted(reorderObj, fAfter);
    else            return true;
}

bool AGeoObject::repositionInHosted(AGeoObject * objTo, bool fAfter)
{
    if (!objTo) return false;
    if (Container != objTo->Container) return false;

    for (auto it = Container->HostedObjects.begin(); it != Container->HostedObjects.end(); ++it)
        if (*it == this)
        {
            Container->HostedObjects.erase(it);
            break;
        }

    for (auto it = Container->HostedObjects.begin(); it != Container->HostedObjects.end(); ++it)
        if (*it == objTo)
        {
            if (fAfter) ++it;
            Container->HostedObjects.insert(it, this);
            return true;
        }
    return false;
}

bool AGeoObject::suicide()
{
    //if (fLocked) return false;
    if (Type->isWorld()) return false; //cannot delete world

    //cannot remove logicals used by composite (and the logicals container itself); the composite kills itself!
    if (Type->isCompositeContainer()) return false;
    if (isInUseByComposite()) return false;

    //if (Type->isGridElement()) return false; //in ants2: cannot remove grid elementary - it is deleted when grid bulk is deleted

    //qDebug() << "!!--Suicide triggered for object:"<<Name;
    AGeoObject * ObjectContainer = Container;

    size_t iObj = 0;
    for (; iObj < Container->HostedObjects.size(); iObj++)
        if (Container->HostedObjects[iObj] == this)
            break;
    if (iObj == Container->HostedObjects.size()) return false;
    Container->HostedObjects.erase(Container->HostedObjects.begin() + iObj);

    //for composite, clear all unused logicals then kill the composite container
    if (Type->isComposite())
    {
        AGeoObject * cl = this->getContainerWithLogical();
        if (cl)
        {
            for (AGeoObject * obj : cl->HostedObjects) delete obj;
            delete cl;
            for (auto itlc = HostedObjects.begin(); itlc != HostedObjects.end(); ++itlc)
                if (*itlc == cl)
                {
                    HostedObjects.erase(itlc);
                    break;
                }
        }
    }
    else if (Type->isGrid())
    {
        AGeoObject * ge = getGridElement();
        if (ge)
        {
            for (AGeoObject * obj : ge->HostedObjects) delete obj;
            delete ge;

            for (auto itge = HostedObjects.begin(); itge != HostedObjects.end(); ++itge)
                if (*itge == ge)
                {
                    HostedObjects.erase(itge);
                    break;
                }
        }
    }

    for (AGeoObject * obj : HostedObjects)
    {
        obj->Container = ObjectContainer;
        Container->HostedObjects.insert(Container->HostedObjects.begin() + iObj, obj);
        iObj++;
    }

    delete this;
    return true;
}

void AGeoObject::recursiveSuicide()
{
    if (Type->isPrototypeCollection()) return;

    for (int i=HostedObjects.size()-1; i >= 0; i--)
        HostedObjects[i]->recursiveSuicide();

    suicide();
}

void AGeoObject::lockUpTheChain()
{  
    if (Type->isWorld()) return;

    fLocked = true;
    lockBuddies();
    Container->lockUpTheChain();
}

void AGeoObject::lockBuddies()
{
    if (!Container) return;

    if (isCompositeMemeber())
    {
        for (size_t i=0; i<Container->HostedObjects.size(); i++)
            Container->HostedObjects[i]->fLocked = true;
        if (Container->Container)
            Container->Container->fLocked = true;
    }
    else if (Container->Type->isHandlingSet())
    {
        for (size_t i=0; i<Container->HostedObjects.size(); i++)
            Container->HostedObjects[i]->fLocked = true;
    }
}

void AGeoObject::lockRecursively()
{
    fLocked = true;
    lockBuddies();
    //if it is a grouped object, lock group buddies  ***!!!

    //for individual:
    for (AGeoObject * obj : HostedObjects)
        obj->lockRecursively();
}

void AGeoObject::unlockAllInside()
{
    fLocked = false;
    //if it is a grouped object, unlock group buddies  ***!!!

    //for individual:
    for (AGeoObject * obj : HostedObjects)
        obj->unlockAllInside();
}

void AGeoObject::updateStack()
{
    AGeoObject * RefObj = getOrMakeStackReferenceVolume();
    if (!RefObj) return;

    double Edge = 0;
    double RefPos = 0;
    for (AGeoObject * obj : RefObj->Container->HostedObjects)
    {
        if (!obj->fActive) continue;
        obj->Orientation[0] = 0; obj->OrientationStr[0].clear();
        obj->Orientation[1] = 0; obj->OrientationStr[1].clear();

        const double halfHeight = obj->Shape->getHeight();
        double relPosrefobj = RefObj->Shape->getRelativePosZofCenter();

        if (obj == RefObj) RefPos = Edge - halfHeight - relPosrefobj;
        else
        {
            obj->Orientation[2] = RefObj->Orientation[2]; obj->OrientationStr[2] = RefObj->OrientationStr[2];

            obj->Position[0] = RefObj->Position[0];  obj->PositionStr[0] = RefObj->PositionStr[0];
            obj->Position[1] = RefObj->Position[1];  obj->PositionStr[1] = RefObj->PositionStr[1];
            obj->Position[2] = Edge - halfHeight;    obj->PositionStr[2].clear();

            double relPos = obj->Shape->getRelativePosZofCenter();
            obj->Position[2] -= relPos;
        }
        Edge -= 2.0 * halfHeight;
    }

    const double dZ = RefPos - RefObj->Position[2];
    for (AGeoObject * obj : RefObj->Container->HostedObjects)
    {
        if (!obj->fActive) continue;
        if (obj != RefObj) obj->Position[2] -= dZ;
    }
}

void AGeoObject::updateAllStacks()
{
    if (Type && Type->isStack()) updateStack();

    for (AGeoObject * obj : HostedObjects) obj->updateAllStacks();
}

void AGeoObject::updateAllMonitors()
{
    if (Type && Type->isMonitor())
    {
        ATypeMonitorObject * mon = static_cast<ATypeMonitorObject*>(Type);

        delete Shape;
        if (mon->config.shape == 0)
            Shape = new AGeoBox(mon->config.size1, mon->config.size2, mon->config.dz);
        else
            Shape = new AGeoTube(0, mon->config.size1, mon->config.dz);

        if (Container) Material = Container->getMaterial();
    }

    for (AGeoObject * obj : HostedObjects) obj->updateAllMonitors();
}

void AGeoObject::removeHostedObject(AGeoObject *obj)
{
    for (auto it = HostedObjects.begin(); it < HostedObjects.end(); ++it)
        if (*it == obj)
        {
            HostedObjects.erase(it);
            break;
        }
}

AGeoObject * AGeoObject::findContainerUp(const QString & name)
{
    //qDebug() << "Looking for:"<<name<<"  Now in:"<<Name;
    if (Name == name) return this;
    //qDebug() << Container;
    if (!Container) return nullptr;
    return Container->findContainerUp(name);
}

void AGeoObject::clearAll()
{
    for (AGeoObject * obj : HostedObjects)
        obj->clearAll();
    HostedObjects.clear();

    delete this;
}

void AGeoObject::clearContent()
{
    for (AGeoObject * obj : HostedObjects)
        obj->clearAll();
    HostedObjects.clear();
}

void AGeoObject::updateWorldSize(double & XYm, double & Zm)
{    
    if (!isWorld())
    {
        const double msize = getMaxSize();
        const double mxy = std::max(fabs(Position[0]), fabs(Position[1]));
        const double mz  = fabs(Position[2]);
        XYm = std::max(XYm, mxy + msize);
        Zm  = std::max(Zm,  mz  + msize);
    }

    for (AGeoObject * obj : HostedObjects)
        obj->updateWorldSize(XYm, Zm);
}

bool AGeoObject::isMaterialInUse(int imat, QString & volName) const
{
    if (Type->isMonitor())             return false; //monitors are always made of Container's material and cannot host objects
    if (Type->isHandlingArray())       return false;
    if (Type->isHandlingSet())         return false;
    if (Type->isInstance())            return false;
    if (Type->isPrototypeCollection()) return false;

    if (Material == imat)
    {
        if ( !Type->isGridElement() && !Type->isCompositeContainer() )
        {
            volName = Name;
            return true;
        }
    }

    for (AGeoObject * obj : HostedObjects)
        if (obj->isMaterialInUse(imat, volName)) return true;

    return false;
}

bool AGeoObject::isMaterialInActiveUse(int imat) const
{
    if (!fActive) return false;

    if (Type->isMonitor()) return false; //monitors are always made of Container's material and cannot host objects
    if (Material == imat)
    {
        if ( !Type->isGridElement() && !Type->isCompositeContainer() )
            return true;
    }

    for (AGeoObject * obj : HostedObjects)
        if (obj->isMaterialInActiveUse(imat)) return true;

    return false;
}

void AGeoObject::collectContainingObjects(std::vector<AGeoObject*> & vec) const
{
    for (AGeoObject * obj : HostedObjects)
    {
        vec.push_back(obj);
        obj->collectContainingObjects(vec);
    }
}

double AGeoObject::getMaxSize() const
{
    if (!Type)
    {
        qWarning() << "||| GeoObject without type:" << Name;
        return 100.0;
    }

    if      (Type->isArray())
    {
        const ATypeArrayObject * a = static_cast<const ATypeArrayObject*>(Type);
        double X = a->stepX * (2.0 + a->numX);   // assuming non-overlapping, so one extra step on each side
        double Y = a->stepY * (2.0 + a->numY);
        double Z = a->stepZ * (2.0 + a->numZ);
        return sqrt(X*X + Y*Y + Z*Z);
    }
    else if (Type->isCircularArray())
    {
        const ATypeCircularArrayObject * a = static_cast<const ATypeCircularArrayObject*>(Type);
        return 2.0 * a->radius;
    }
    else if (Type->isHexagonalArray())
    {
        const ATypeHexagonalArrayObject * a = static_cast<const ATypeHexagonalArrayObject*>(Type);
        if (a->Shape == ATypeHexagonalArrayObject::Hexagonal) return 2.0 * a->Rings * a->Step;
        return 2.0 * a->Step * sqrt(a->NumX * a->NumX  +  a->NumY * a->NumY);
    }
    else if (Type->isComposite())
    {
        const AGeoObject * cont = getContainerWithLogical();
        if (!cont)
        {
            qWarning() << "||| Composite GeoObject without composite container:" << Name;
            return 100.0;
        }

        double msize = 0;
        for (AGeoObject * lo : cont->HostedObjects)
        {
            double thisMax = 0;
            for (int i=0; i<3; i++)
                thisMax  = std::max(thisMax, fabs(lo->Position[i]));
            thisMax += lo->getMaxSize();

            msize = std::max(msize, thisMax);
        }
        return msize;
    }

    if (!Shape)
    {
        qWarning() << "GeoObject without Shape detected:" << Name;
        return 100.0;
    }

    return Shape->maxSize();
}

#include "TGeoMatrix.h"
bool AGeoObject::getPositionInWorld(double * Pos) const
{
    for (int i=0; i<3; i++) Pos[i] = 0;
    if (isWorld()) return true;

    const AGeoObject * obj = this;

    double PosInContainer[3];
    const AGeoObject * cont = obj->Container;
    while (cont)
    {
        for (int i=0; i<3; i++) Pos[i] += obj->Position[i];

        TGeoRotation Trans("Rot", cont->Orientation[0], cont->Orientation[1], cont->Orientation[2]);
        Trans.LocalToMaster(Pos, PosInContainer);

        for (int i=0; i<3; i++) Pos[i] = PosInContainer[i];
        if (cont->isWorld()) return true;

        obj = cont;
        cont = obj->Container;
    }

    return false;
}

bool AGeoObject::isContainerValidForDrop(QString & errorStr) const
{
    if (Type->isGrid())
    {
        errorStr = "Grid cannot contain anything but the grid element!";
        return false;
    }
    if (isCompositeMemeber())
    {
        errorStr = "Cannot move objects to composite logical objects!";
        return false;
    }
    if (Type->isMonitor())
    {
        errorStr = "Monitors cannot host objects!";
        return false;
    }
    return true;
}

void AGeoObject::enforceUniqueNameForCloneRecursive(AGeoObject * World, AGeoObject & tmpContainer)
{
    if (!World) return;

    QString newName = generateCloneObjName(Name);
    while (World->isNameExists(newName) || tmpContainer.isNameExists(newName))
        newName = generateCloneObjName(newName);

    if (Container && Container->Type->isStack())
    {
        ATypeStackContainerObject * Stack = static_cast<ATypeStackContainerObject*>(Container->Type);
        if (Stack->ReferenceVolume == Name)
            Stack->ReferenceVolume = newName;
    }

    if (isCompositeMemeber())
        updateNameOfLogicalMember(Name, newName);

    Name = newName;
    tmpContainer.HostedObjects.push_back(this);

    for (AGeoObject * hostedObj : HostedObjects)
        hostedObj->enforceUniqueNameForCloneRecursive(World, tmpContainer);
}

void AGeoObject::addSuffixToNameRecursive(const QString & suffix)
{
    const QString newName = Name + "_at_" + suffix;

    if (Container && Container->Type->isStack())
    {
        ATypeStackContainerObject * Stack = static_cast<ATypeStackContainerObject*>(Container->Type);
        if (Stack->ReferenceVolume == Name)
            Stack->ReferenceVolume = newName;
    }
    if (isCompositeMemeber()) updateNameOfLogicalMember(Name, newName);

    NameWithoutSuffix = Name;
    Name = newName;

    for (AGeoObject * obj : HostedObjects)
        obj->addSuffixToNameRecursive(suffix);
}

AGeoObject * AGeoObject::makeClone(AGeoObject * World)
{
    QJsonArray ar;
    writeAllToJarr(ar);
    AGeoObject * clone = new AGeoObject();
    QString errStr = clone->readAllFromJarr(clone, ar);
    if (!errStr.isEmpty())
    {
        clone->clearAll();
        return nullptr;
    }

    //updating names to make them unique
    if (World)
    {
        AGeoObject tmpContainer;
        clone->enforceUniqueNameForCloneRecursive(World, tmpContainer);
        tmpContainer.HostedObjects.clear();  // not deleting the objects inside!
    }
    return clone;
}

AGeoObject * AGeoObject::makeCloneForInstance(const QString & suffix)
{
    QJsonArray ar;
    writeAllToJarr(ar);
    AGeoObject * clone = new AGeoObject();
    QString errStr = clone->readAllFromJarr(clone, ar);
    if (!errStr.isEmpty())
    {
        clone->clearAll();
        return nullptr;
    }

    clone->addSuffixToNameRecursive(suffix);
    return clone;
}

void AGeoObject::findAllInstancesRecursive(std::vector<AGeoObject *> & Instances)
{
    if (Type->isInstance()) Instances.push_back(this);

    for (AGeoObject * obj : HostedObjects)
        obj->findAllInstancesRecursive(Instances);
}

bool AGeoObject::isContainInstanceRecursive() const
{
    if (Type->isInstance()) return true;

    for (AGeoObject * obj : HostedObjects)
        if (obj->isContainInstanceRecursive()) return true;

    return false;
}

bool AGeoObject::isInstanceMember() const
{
    const AGeoObject * obj = this;

    while (obj)
    {
        if (obj->Type->isInstance()) return true;
        obj = obj->Container;
    }
    return false;
}

bool AGeoObject::isPossiblePrototype(QString * sReason) const
{
    if (isWorld())
    {
        if (sReason) *sReason = "World cannot be a prototype";
        return false;
    }

    if (isContainInstanceRecursive())
    {
        if (sReason) *sReason = "Prototype cannot contain instances of other prototypes";
        return false;
    }
    if (isInstanceMember())
    {
        if (sReason) *sReason = "Cannot make a prototype form an object which is a part of an instance";
        return false;
    }

    if (Type->isLogical())
    {
        if (sReason) *sReason = "Logical volume cannot be a prototype";
        return false;
    }
    if (Type->isCompositeContainer())
    {
        if (sReason) *sReason = "Composite container cannot be a prototype";
        return false;
    }
    return true;
}

QString AGeoObject::makeItPrototype(AGeoObject * Prototypes)
{
    QString errStr;

    isPossiblePrototype(&errStr);
    if (errStr.isEmpty()) migrateTo(Prototypes);

    return errStr;
}

bool AGeoObject::isPrototypeInUseRecursive(const QString & PrototypeName, QStringList * Users) const
{
    if (Type->isInstance())
    {
        ATypeInstanceObject * instance = static_cast<ATypeInstanceObject*>(Type);
        if (PrototypeName == instance->PrototypeName)
        {
            if (Users) *Users << Name;
            return true;
        }
        return false; // instance cannot contain other instances
    }

    bool bFoundInUse = false;
    for (AGeoObject * obj : HostedObjects)
        if (obj->isPrototypeInUseRecursive(PrototypeName, Users))
            bFoundInUse = true;

    return bFoundInUse;
}

bool AGeoObject::isPrototypeMember() const
{
    if (Type->isPrototype()) return false;

    const AGeoObject * obj = this;
    while (obj)
    {
        if (obj->Type->isPrototype()) return true;
        obj = obj->Container;
    }
    return false;
}

bool AGeoObject::isGoodContainerForInstance() const
{
    if (Type->isSingle() || Type->isHandlingArray() || Type->isWorld()) return true;
    return false;
}

/*
void AGeoObject::makeItWorld()
{
    Name = "World";
    delete Type; Type = new ATypeWorldObject();
}
*/

void AGeoObject::clearTrueRotationRecursive()
{
    TrueRot = nullptr;
    for (AGeoObject * obj : HostedObjects) obj->clearTrueRotationRecursive();
}

void AGeoObject::scaleRecursive(double factor)
{
    for (size_t i = 0; i < 3; i++) Position[i] *= factor;

    if (Shape) Shape->scale(factor);
    Type->scale(factor);

    for (AGeoObject * obj : HostedObjects) obj->scaleRecursive(factor);
}

bool AGeoObject::checkCompatibleWithGeant4() const
{
    if (!fActive) return true;

    if (Shape && !Shape->isCompatibleWithGeant4())
    {
        AErrorHub::addQError(Name + ": shape is incopatible with Geant4");
        return false;
    }

    for (AGeoObject * obj : HostedObjects)
        if (!obj->checkCompatibleWithGeant4()) return false;

    return true;
}

#include <QRandomGenerator>
QString randomString(int lettLength, int numLength)
{
    //const QString possibleLett("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    const QString possibleLett("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const QString possibleNum("0123456789");

    QString randomString;
    for(int i=0; i<lettLength; i++)
    {
        int index =  QRandomGenerator::global()->generate() % possibleLett.length();
        QChar nextChar = possibleLett[index];
        randomString.append(nextChar);
    }
    for(int i=0; i<numLength; i++)
    {
        int index =  QRandomGenerator::global()->generate() % possibleNum.length();
        QChar nextChar = possibleNum[index];
        randomString.append(nextChar);
    }
    return randomString;
}

QString AGeoObject::generateRandomName()
{
    return randomString(2, 1);
}

QString AGeoObject::generateRandomObjectName()
{
    QString str = randomString(2, 1);
    str = "New_" + str;
    return str;
}

QString AGeoObject::generateCloneObjName(const QString & initialName)
{
    QString newName;
    const QStringList sl = initialName.split("_c");
    if (sl.size() > 1)
    {
        for (int i = 0; i < sl.size()-1; i++)  // in case the name was clone of clone with broken indexes (e.g. aaa_c:Xbbb_c:0)
            newName += sl.at(i) + "_c";
        bool ok;
        int oldIndex = sl.last().toInt(&ok);
        if (ok) newName += QString::number(oldIndex + 1);
        else    newName.clear();
    }
    if (newName.isEmpty())
        return initialName + "_c0";
    return newName;
}
