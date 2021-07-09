#include "ageotype.h"
#include "ajsontools.h"
#include "ageoconsts.h"

#include <QDebug>

AGeoType *AGeoType::TypeObjectFactory(const QString & Type)
{
    if (Type == "Single")             return new ATypeSingleObject();
    if (Type == "Array" ||
        Type == "XYArray")            return new ATypeArrayObject();
    if (Type == "CircularArray")      return new ATypeCircularArrayObject();
    if (Type == "Monitor")            return new ATypeMonitorObject();
    if (Type == "Stack")              return new ATypeStackContainerObject();
    if (Type == "Instance")           return new ATypeInstanceObject();
    if (Type == "Prototype")          return new ATypePrototypeObject();
    if (Type == "Composite")          return new ATypeCompositeObject();
    if (Type == "CompositeContainer") return new ATypeCompositeContainerObject();
    if (Type == "GridElement")        return new ATypeGridElementObject();
    if (Type == "Grid")               return new ATypeGridObject();
    if (Type == "Group")              return new ATypeGroupContainerObject();
    if (Type == "PrototypeCollection")return new ATypePrototypeCollectionObject();
    if (Type == "World")              return new ATypeWorldObject(); //is not used to create World, only to check file with WorldTree starts with World and reads positioning script

    qCritical() << "Unknown opject type in TypeObjectFactory:"<<Type;
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

QString ATypeArrayObject::evaluateStringValues(ATypeArrayObject & A)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;
    bool ok;

    ok = GC.updateParameter(errorStr, A.strNumX, A.numX, true, true) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strNumY, A.numY, true, true) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strNumZ, A.numZ, true, true) ; if (!ok) return errorStr;

    ok = GC.updateParameter(errorStr, A.strStepX, A.stepX, true, false, false) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strStepY, A.stepY, true, false, false) ; if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strStepZ, A.stepZ, true, false, false) ; if (!ok) return errorStr;

    ok = GC.updateParameter(errorStr, A.strStartIndex, A.startIndex, false, true) ; if (!ok) return errorStr;

    return "";
}

bool ATypeArrayObject::isGeoConstInUse(const QRegExp &nameRegExp) const
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

void ATypeArrayObject::replaceGeoConstName(const QRegExp &nameRegExp, const QString &newName)
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

    ATypeArrayObject::evaluateStringValues(*this);
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

bool ATypeMonitorObject::isGeoConstInUse(const QRegExp & nameRegExp) const
{
    if (config.str2size1.contains(nameRegExp)) return true;
    if (config.str2size2.contains(nameRegExp)) return true;
    return false;
}

void ATypeMonitorObject::replaceGeoConstName(const QRegExp & nameRegExp, const QString & newName)
{
    config.str2size1.replace(nameRegExp, newName);
    config.str2size2.replace(nameRegExp, newName);
}

bool ATypeMonitorObject::isParticleInUse(int partId) const
{
    if (config.PhotonOrParticle == 0) return false;
    return (config.ParticleIndex == partId);
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

bool ATypeCircularArrayObject::isGeoConstInUse(const QRegExp & nameRegExp) const
{
    if (strNum.contains(nameRegExp))         return true;
    if (strAngularStep.contains(nameRegExp)) return true;
    if (strRadius.contains(nameRegExp))      return true;
    if (strStartIndex.contains(nameRegExp))  return true;
    return false;
}

void ATypeCircularArrayObject::replaceGeoConstName(const QRegExp &nameRegExp, const QString &newName)
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

    ATypeCircularArrayObject::evaluateStringValues(*this);
}

QString ATypeCircularArrayObject::evaluateStringValues(ATypeCircularArrayObject &A)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    QString errorStr;
    bool ok;

    ok = GC.updateParameter(errorStr, A.strNum,         A.num,         true,  true) ;        if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strAngularStep, A.angularStep, true,  false, false); if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strRadius,      A.radius,      true,  true,  false); if (!ok) return errorStr;
    ok = GC.updateParameter(errorStr, A.strStartIndex,  A.startIndex,  false, true) ;        if (!ok) return errorStr;

    return "";
}
