#include "ageotype.h"
#include "ajsontools.h"
#include "ageoconsts.h"
#include "aerrorhub.h"

#include <QDebug>

static const QString World               = "World";
static const QString Single              = "Single";
static const QString Logical             = "Logical";
static const QString Stack               = "Stack";
static const QString Array               = "Array";
static const QString CircularArray       = "CircularArray";
static const QString HexagonalArray      = "HexagonalArray";
static const QString Monitor             = "Monitor";
static const QString Prototype           = "Prototype";
static const QString Instance            = "Instance";
static const QString PrototypeCollection = "PrototypeCollection";
static const QString Composite           = "Composite";
static const QString CompositeContainer  = "CompositeContainer";
static const QString Grid                = "Grid";
static const QString GridElement         = "GridElement";

AGeoType * AGeoType::makeTypeObject(const QString & typeStr)
{
    if (typeStr == "Single")              return new ATypeSingleObject();
    if (typeStr == "Array")               return new ATypeArrayObject();
    if (typeStr == "CircularArray")       return new ATypeCircularArrayObject();
    if (typeStr == "HexagonalArray")      return new ATypeHexagonalArrayObject();
    if (typeStr == "Monitor")             return new ATypeMonitorObject();
    if (typeStr == "Stack")               return new ATypeStackContainerObject();
    if (typeStr == "Instance")            return new ATypeInstanceObject();
    if (typeStr == "Prototype")           return new ATypePrototypeObject();
    if (typeStr == "Composite")           return new ATypeCompositeObject();
    if (typeStr == "CompositeContainer")  return new ATypeCompositeContainerObject();
    if (typeStr == "GridElement")         return new ATypeGridElementObject();
    if (typeStr == "Grid")                return new ATypeGridObject();
    if (typeStr == "PrototypeCollection") return new ATypePrototypeCollectionObject();
    if (typeStr == "World")               return new ATypeWorldObject(); //is not used to create World, only to check file with WorldTree starts with World and reads positioning script

    QString err = "Unknown object type in makeTypeObject() factory: " + typeStr;
    AErrorHub::addQError(err);
    qCritical() << err;
    return nullptr;
}

bool AGeoType::isWorld() const               {return pType == &World;}
bool AGeoType::isSingle() const              {return pType == &Single;}
bool AGeoType::isLogical() const             {return pType == &Logical;}
bool AGeoType::isStack() const               {return pType == &Stack;}
bool AGeoType::isArray() const               {return pType == &Array;}
bool AGeoType::isCircularArray() const       {return pType == &CircularArray;}
bool AGeoType::isHexagonalArray() const      {return pType == &HexagonalArray;}
bool AGeoType::isMonitor() const             {return pType == &Monitor;}
bool AGeoType::isPrototype() const           {return pType == &Prototype;}
bool AGeoType::isInstance() const            {return pType == &Instance;}
bool AGeoType::isPrototypeCollection() const {return pType == &PrototypeCollection;}
bool AGeoType::isComposite() const           {return pType == &Composite;}
bool AGeoType::isCompositeContainer() const  {return pType == &CompositeContainer;}
bool AGeoType::isGrid() const                {return pType == &Grid;}
bool AGeoType::isGridElement() const         {return pType == &GridElement;}

bool AGeoType::isHandlingStandard() const
{
    return isSingle() || isMonitor() || isInstance() || isComposite() || isGrid() || isGridElement() ;
}

bool AGeoType::isHandlingSet() const
{
    return isStack() || isPrototype() || isCompositeContainer();
}

bool AGeoType::isHandlingArray() const
{
    return isArray() || isCircularArray() || isHexagonalArray();
}

void AGeoType::writeToJson(QJsonObject & json) const
{
    json["Type"] = *pType;
    doWriteToJson(json);
}

// --- Concrete types ---

ATypeWorldObject::ATypeWorldObject() {pType = &World;}

void ATypeWorldObject::doWriteToJson(QJsonObject & json) const
{
    json["FixedSize"] = bFixedSize;
}

void ATypeWorldObject::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "FixedSize", bFixedSize);
}

// ---

ATypePrototypeCollectionObject::ATypePrototypeCollectionObject() {pType = &PrototypeCollection;}

// ---

ATypeSingleObject::ATypeSingleObject() {pType = &Single;}

// ---

ATypeCompositeObject::ATypeCompositeObject() {pType = &Composite;}

// ---

ATypeStackContainerObject::ATypeStackContainerObject() {pType = &Stack;}

void ATypeStackContainerObject::doWriteToJson(QJsonObject & json) const
{
    json["ReferenceVolume"] = ReferenceVolume;
}

void ATypeStackContainerObject::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "ReferenceVolume", ReferenceVolume);
}

// ---

ATypeCompositeContainerObject::ATypeCompositeContainerObject() {pType = &CompositeContainer;}

// ---

ATypePrototypeObject::ATypePrototypeObject() {pType = &Prototype;}

// ---

ATypeInstanceObject::ATypeInstanceObject(QString PrototypeName) :
    PrototypeName(PrototypeName) {pType = &Instance;}

void ATypeInstanceObject::doWriteToJson(QJsonObject & json) const
{
    json["PrototypeName"] = PrototypeName;
}

void ATypeInstanceObject::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "PrototypeName", PrototypeName);
}

// ---

ATypeArrayObject::ATypeArrayObject() {pType = &Array;}

ATypeArrayObject::ATypeArrayObject(int numX, int numY, int numZ, double stepX, double stepY, double stepZ, int startIndex)
    : numX(numX), numY(numY), numZ(numZ), stepX(stepX), stepY(stepY), stepZ(stepZ), startIndex(startIndex) {pType = &Array;}

void ATypeArrayObject::Reconfigure(int NumX, int NumY, int NumZ, double StepX, double StepY, double StepZ)
{
    numX = NumX; numY = NumY; numZ = NumZ;
    stepX = StepX; stepY = StepY; stepZ = StepZ;
}

QString ATypeArrayObject::introduceGeoConstValues()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;
    bool ok;

    ok = GC.updateParameter(errorStr, strNumX, numX, true, true) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strNumY, numY, true, true) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strNumZ, numZ, true, true) ; if (!ok) return errorStr;

    ok = GC.updateParameter(errorStr, strStepX, stepX, true, false, false) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strStepY, stepY, true, false, false) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strStepZ, stepZ, true, false, false) ; if (!ok) return errorStr;

    ok = GC.updateParameter(errorStr, strStartIndex, startIndex, false, true) ; if (!ok) return errorStr;

    return "";
}

bool ATypeArrayObject::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strNumX .contains(nameRegExp)) return true;
    if (strNumY .contains(nameRegExp)) return true;
    if (strNumZ .contains(nameRegExp)) return true;
    if (strStepX.contains(nameRegExp)) return true;
    if (strStepY.contains(nameRegExp)) return true;
    if (strStepZ.contains(nameRegExp)) return true;
    if (strStartIndex.contains(nameRegExp)) return true;
    return false;
}

void ATypeArrayObject::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strNumX .replace(nameRegExp, newName);
    strNumY .replace(nameRegExp, newName);
    strNumZ .replace(nameRegExp, newName);
    strStepX.replace(nameRegExp, newName);
    strStepY.replace(nameRegExp, newName);
    strStepZ.replace(nameRegExp, newName);
    strStartIndex.replace(nameRegExp, newName);
}

void ATypeArrayObject::doWriteToJson(QJsonObject &json) const
{
    json["numX"]  = numX;
    json["numY"]  = numY;
    json["numZ"]  = numZ;
    json["stepX"] = stepX;
    json["stepY"] = stepY;
    json["stepZ"] = stepZ;
    json["startIndex"] = startIndex;

    if (!strNumX .isEmpty()) json["strNumX"]  = strNumX;
    if (!strNumY .isEmpty()) json["strNumY"]  = strNumY;
    if (!strNumZ .isEmpty()) json["strNumZ"]  = strNumZ;
    if (!strStepX.isEmpty()) json["strStepX"] = strStepX;
    if (!strStepY.isEmpty()) json["strStepY"] = strStepY;
    if (!strStepZ.isEmpty()) json["strStepZ"] = strStepZ;
    if (!strStartIndex.isEmpty()) json["strStartIndex"] = strStartIndex;
}

void ATypeArrayObject::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "numX",  numX);
    jstools::parseJson(json, "numY",  numY);
    jstools::parseJson(json, "numZ",  numZ);
    jstools::parseJson(json, "stepX", stepX);
    jstools::parseJson(json, "stepY", stepY);
    jstools::parseJson(json, "stepZ", stepZ);
    jstools::parseJson(json, "startIndex", startIndex);

    if (!jstools::parseJson(json, "strNumX",  strNumX))  strNumX .clear();
    if (!jstools::parseJson(json, "strNumY",  strNumY))  strNumY .clear();
    if (!jstools::parseJson(json, "strNumZ",  strNumZ))  strNumZ .clear();
    if (!jstools::parseJson(json, "strStepX", strStepX)) strStepX.clear();
    if (!jstools::parseJson(json, "strStepY", strStepY)) strStepY.clear();
    if (!jstools::parseJson(json, "strStepZ", strStepZ)) strStepZ.clear();
    if (!jstools::parseJson(json, "strStartIndex", strStartIndex)) strStartIndex.clear();
}

// ---

ATypeCircularArrayObject::ATypeCircularArrayObject() {pType = &CircularArray;}

ATypeCircularArrayObject::ATypeCircularArrayObject(int num, double angularStep, double radius, int StartIndex)
    : num(num), angularStep(angularStep), radius(radius) {startIndex = StartIndex; pType = &CircularArray;}

void ATypeCircularArrayObject::Reconfigure(int Num, double AngularStep, double Radius)
{
    num = Num;
    angularStep = AngularStep;
    radius = Radius;
}

bool ATypeCircularArrayObject::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strNum.contains(nameRegExp))         return true;
    if (strAngularStep.contains(nameRegExp)) return true;
    if (strRadius.contains(nameRegExp))      return true;
    if (strStartIndex.contains(nameRegExp))  return true;
    return false;
}

void ATypeCircularArrayObject::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strNum.replace(nameRegExp, newName);
    strAngularStep.replace(nameRegExp, newName);
    strRadius.replace(nameRegExp, newName);
    strStartIndex.replace(nameRegExp, newName);
}

void ATypeCircularArrayObject::doWriteToJson(QJsonObject &json) const
{
    json["num"]         = num;
    json["angularStep"] = angularStep;
    json["radius"]      = radius;
    json["startIndex"]  = startIndex;

    if (!strNum.isEmpty())         json["strNum"]         = strNum;
    if (!strAngularStep.isEmpty()) json["strAngularStep"] = strAngularStep;
    if (!strRadius.isEmpty())      json["strRadius"]      = strRadius;
    if (!strStartIndex.isEmpty())  json["strStartIndex"]  = strStartIndex;
}

void ATypeCircularArrayObject::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "num",  num);
    jstools::parseJson(json, "angularStep", angularStep);
    jstools::parseJson(json, "radius", radius);
    jstools::parseJson(json, "startIndex", startIndex);

    if (!jstools::parseJson(json, "strNum",         strNum))         strNum.clear();
    if (!jstools::parseJson(json, "strAngularStep", strAngularStep)) strAngularStep.clear();
    if (!jstools::parseJson(json, "strRadius",      strRadius))      strRadius.clear();
    if (!jstools::parseJson(json, "strStartIndex",  strStartIndex))  strStartIndex.clear();
}

QString ATypeCircularArrayObject::introduceGeoConstValues()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;
    bool ok;

    ok = GC.updateParameter(errorStr, strNum,         num,         true,  true);         if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strAngularStep, angularStep, true,  false, false); if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strRadius,      radius,      true,  true,  false); if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strStartIndex,  startIndex,  false, true);         if (!ok) return errorStr;

    return "";
}

// ---

ATypeHexagonalArrayObject::ATypeHexagonalArrayObject() {pType = &HexagonalArray;}

void ATypeHexagonalArrayObject::Reconfigure(double step, EShapeMode shape, int rings, int numX, int numY, bool skipOddLast)
{
    Step        = step;
    Shape       = shape;
    Rings       = rings;
    NumX        = numX;
    NumY        = numY;
    SkipOddLast = skipOddLast;
}

bool ATypeHexagonalArrayObject::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strStep.contains(nameRegExp))       return true;
    if (strRings.contains(nameRegExp))      return true;
    if (strNumX.contains(nameRegExp))       return true;
    if (strNumY.contains(nameRegExp))       return true;
    if (strStartIndex.contains(nameRegExp)) return true;
    return false;
}

void ATypeHexagonalArrayObject::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strStep.replace(nameRegExp, newName);
    strRings.replace(nameRegExp, newName);
    strNumX.replace(nameRegExp, newName);
    strNumY.replace(nameRegExp, newName);
    strStartIndex.replace(nameRegExp, newName);
}

void ATypeHexagonalArrayObject::doWriteToJson(QJsonObject & json) const
{
    json["Step"]        = Step;
    json["Shape"]       = ( Shape == Hexagonal ? "Hexagonal" : "XY" );
    json["Rings"]       = Rings;
    json["NumX"]        = NumX;
    json["NumY"]        = NumY;
    json["SkipOddLast"] = SkipOddLast;
    json["startIndex"]  = startIndex;

    if (!strStep.isEmpty())       json["strStep"]       = strStep;
    if (!strRings.isEmpty())      json["strRings"]      = strRings;
    if (!strNumX.isEmpty())       json["strNumX"]       = strNumX;
    if (!strNumY.isEmpty())       json["strNumY"]       = strNumY;
    if (!strStartIndex.isEmpty()) json["strStartIndex"] = strStartIndex;
}

void ATypeHexagonalArrayObject::readFromJson(const QJsonObject & json)
{
    QString ModeStr;
    jstools::parseJson(json, "Shape",  ModeStr);
    if (ModeStr == "XY") Shape = XY;
    else                 Shape = Hexagonal;

    jstools::parseJson(json, "Step",       Step);
    jstools::parseJson(json, "Rings",      Rings);
    jstools::parseJson(json, "NumX",       NumX);
    jstools::parseJson(json, "NumY",       NumY);
    jstools::parseJson(json, "startIndex", startIndex);

    if (!jstools::parseJson(json, "strStep",       strStep))       strStep.clear();
    if (!jstools::parseJson(json, "strRings",      strRings))      strRings.clear();
    if (!jstools::parseJson(json, "strNumX",       strNumX))       strNumX.clear();
    if (!jstools::parseJson(json, "strNumY",       strNumY))       strNumY.clear();
    if (!jstools::parseJson(json, "strStartIndex", strStartIndex)) strStartIndex.clear();
}

QString ATypeHexagonalArrayObject::introduceGeoConstValues()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;
    bool ok;

    ok = GC.updateParameter(errorStr, strStep,       Step,       true,  true, false);  if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strRings,      Rings,      false, true);         if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strNumX,       NumX,       true,  true);         if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strNumY,       NumY,       true,  true);         if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, strStartIndex, startIndex, false, true);         if (!ok) return errorStr;

    return "";
}

// ---

ATypeMonitorObject::ATypeMonitorObject() {pType = &Monitor;}

void ATypeMonitorObject::doWriteToJson(QJsonObject &json) const
{
    config.writeToJson(json);
}

void ATypeMonitorObject::readFromJson(const QJsonObject &json)
{
    config.readFromJson(json);
}

bool ATypeMonitorObject::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (config.str2size1.contains(nameRegExp)) return true;
    if (config.str2size2.contains(nameRegExp)) return true;
    return false;
}

void ATypeMonitorObject::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString & newName)
{
    config.str2size1.replace(nameRegExp, newName);
    config.str2size2.replace(nameRegExp, newName);
}

QString ATypeMonitorObject::introduceGeoConstValues()
{
    return config.updateFromGeoConstants();
}

// ---

ATypeGridObject::ATypeGridObject() {pType = &Grid;}

// ---

ATypeGridElementObject::ATypeGridElementObject() {pType = &GridElement;}

void ATypeGridElementObject::doWriteToJson(QJsonObject & json) const
{
    json["size1"] = size1;
    json["size2"] = size2;
    json["shape"] = shape;
    json["dz"]    = dz;
}

void ATypeGridElementObject::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "size1", size1);
    jstools::parseJson(json, "size2", size2);
    jstools::parseJson(json, "shape", shape);
    jstools::parseJson(json, "dz",    dz);
}

// ---
