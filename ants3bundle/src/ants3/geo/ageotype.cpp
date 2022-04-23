#include "ageotype.h"
#include "ajsontools.h"
#include "ageoconsts.h"

#include <QDebug>

//static const QString World = "World";
//bool AGeoType::isWorld() const {return pType == &World;}
//ATypeWorldObject::ATypeWorldObject() {Type = &World;}

AGeoType *AGeoType::makeTypeObject(const QString & typeStr)
{
    if (typeStr == "Single")             return new ATypeSingleObject();
    if (typeStr == "Array")              return new ATypeArrayObject();
    if (typeStr == "CircularArray")      return new ATypeCircularArrayObject();
    if (typeStr == "HexagonalArray")     return new ATypeHexagonalArrayObject();
    if (typeStr == "Monitor")            return new ATypeMonitorObject();
    if (typeStr == "Stack")              return new ATypeStackContainerObject();
    if (typeStr == "Instance")           return new ATypeInstanceObject();
    if (typeStr == "Prototype")          return new ATypePrototypeObject();
    if (typeStr == "Composite")          return new ATypeCompositeObject();
    if (typeStr == "CompositeContainer") return new ATypeCompositeContainerObject();
    if (typeStr == "GridElement")        return new ATypeGridElementObject();
    if (typeStr == "Grid")               return new ATypeGridObject();
    if (typeStr == "PrototypeCollection")return new ATypePrototypeCollectionObject();
    if (typeStr == "World")              return new ATypeWorldObject(); //is not used to create World, only to check file with WorldTree starts with World and reads positioning script

    qCritical() << "Unknown opject type in TypeObjectFactory:"<<typeStr;
    return nullptr;
}

void AGeoType::writeToJson(QJsonObject & json) const
{
    json["Type"] = Type;
}

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

void ATypeArrayObject::writeToJson(QJsonObject &json) const
{
    AGeoType::writeToJson(json);

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

void ATypeGridElementObject::writeToJson(QJsonObject &json) const
{
    AGeoType::writeToJson(json);

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

void ATypeMonitorObject::writeToJson(QJsonObject &json) const
{
    AGeoType::writeToJson(json);

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


void ATypeWorldObject::writeToJson(QJsonObject & json) const
{
    AGeoType::writeToJson(json);
    json["FixedSize"] = bFixedSize;
}

void ATypeWorldObject::readFromJson(const QJsonObject &json)
{
    bFixedSize = false;
    jstools::parseJson(json, "FixedSize", bFixedSize);
}

void ATypeStackContainerObject::writeToJson(QJsonObject & json) const
{
    AGeoType::writeToJson(json);
    json["ReferenceVolume"] = ReferenceVolume;
}

void ATypeStackContainerObject::readFromJson(const QJsonObject & json)
{
    ReferenceVolume.clear();
    jstools::parseJson(json, "ReferenceVolume", ReferenceVolume);
}

void ATypeInstanceObject::writeToJson(QJsonObject & json) const
{
    AGeoType::writeToJson(json);
    json["PrototypeName"] = PrototypeName;
}

void ATypeInstanceObject::readFromJson(const QJsonObject & json)
{
    PrototypeName.clear();
    jstools::parseJson(json, "PrototypeName", PrototypeName);
}

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

void ATypeCircularArrayObject::writeToJson(QJsonObject &json) const
{
    AGeoType::writeToJson(json);

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

void ATypeHexagonalArrayObject::writeToJson(QJsonObject & json) const
{
    AGeoType::writeToJson(json);

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
