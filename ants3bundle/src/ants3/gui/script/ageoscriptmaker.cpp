#include "ageoscriptmaker.h"
#include "amaterialhub.h"
#include "amaterial.h"
#include "ageometryhub.h"
#include "ageoconsts.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "ageospecial.h"

#include <QDebug>

AGeoScriptMaker::AGeoScriptMaker(ELanguage lang) : Language(lang)
{
    init();
}

void AGeoScriptMaker::init()
{
    TrueStr     = (Language == JavaScript ? "true"  : "True");
    FalseStr    = (Language == JavaScript ? "false" : "False");
    CommentStr  = (Language == JavaScript ? "//"    : "#");
    VariableStr = (Language == JavaScript ? "var "  : "");
    ArrBeginStr = (Language == JavaScript ? "["     : "(");
    ArrEndStr   = (Language == JavaScript ? "]"     : ")");
}

void AGeoScriptMaker::createScript(QString & script)
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    script = CommentStr + "== Auto-generated script ==\n\n";

    int indent = 0;
    QString indentStr;

    /*
    if (Python == Language)     // for now
    {
        script += "true = True\n";
        script += "false = False\n\n";
    }
    */

    script += indentStr + CommentStr + "Defined materials:\n";
    const QStringList mn = MatHub.getListOfMaterialNames();
    for (int i = 0; i < mn.size(); i++)
        script += indentStr + VariableStr + mn[i] + "_mat = " + QString::number(i) + "\n";
    script += "\n";

    AGeoObject * World = GeoHub.World;

    QString geoScr = AGeoConsts::getConstInstance().exportToScript(World, CommentStr, VariableStr); // boolean?
    if (!geoScr.simplified().isEmpty())
    {
        script += indentStr + CommentStr + "Geometry constants:\n";
        script += geoScr;
        script += "\n";
    }

    script += indentStr + "geo.clearWorld()\n";
    script += "\n";

    QString protoString;
    objectMembersToScript(GeoHub.Prototypes, protoString, indent, true, true);
    if (!protoString.simplified().isEmpty())
    {
        script += indentStr + CommentStr + "Prototypes:";
        script += protoString;
        script += "\n\n";
    }

    script += indentStr + CommentStr + "Geometry:";
    objectMembersToScript(World, script, indent, true, true);

    script += "\n\n" + indentStr + "geo.updateGeometry(" + TrueStr + ")";
}

void AGeoScriptMaker::objectMembersToScript(AGeoObject * Master, QString & script, int ident, bool useStrings, bool bRecursive)
{
    for (AGeoObject * obj : Master->HostedObjects)
        objectToScript(obj, script, ident, useStrings, bRecursive);
}

void AGeoScriptMaker::objectToScript(AGeoObject * obj, QString & script, int ident, bool useStrings, bool bRecursive)
{
    int bigIdent = ident + 4;
    int medIdent = ident + 2;

    if (Python == Language) bigIdent = medIdent = 0;

    const QString Starter = "\n" + QString(" ").repeated(ident);

    if (obj->Type->isLogical())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_basicObject(obj, useStrings);
        addScaledIfApplicable(script, obj, ident, useStrings);
    }
    else if (obj->Type->isCompositeContainer())
    {
        //nothing to do
    }
    else if (obj->Type->isSingle() )
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_basicObject(obj, useStrings);
        addScaledIfApplicable(script, obj, ident, useStrings);
        addRoleIfApplicable(script, obj, ident, useStrings);
        addLineProperties(script, obj, ident);
        if (bRecursive) objectMembersToScript(obj, script, medIdent, useStrings, bRecursive);
    }
    else if (obj->Type->isComposite())
    {
        script += "\n" + QString(" ").repeated(ident) + CommentStr + "-->-- logical volumes for " + obj->Name;
        objectMembersToScript(obj->getContainerWithLogical(), script, bigIdent, useStrings, bRecursive);
        script += "\n" + QString(" ").repeated(ident) + CommentStr + "--<-- logical volumes end for " + obj->Name;

        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_basicObject(obj, useStrings);
        addScaledIfApplicable(script, obj, ident, useStrings);
        addRoleIfApplicable(script, obj, ident, useStrings);
        addLineProperties(script, obj, ident);
        if (bRecursive) objectMembersToScript(obj, script, medIdent, useStrings, bRecursive);
    }
    else if (obj->Type->isHandlingArray())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_arrayObject(obj);
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + "-->-- array elements for " + obj->Name;
        objectMembersToScript(obj, script, medIdent, useStrings, bRecursive);
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + "--<-- array elements end for " + obj->Name;
    }
    else if (obj->Type->isMonitor())
    {
        script += Starter + makeScriptString_monitorBaseObject(obj);
        script += Starter + makeScriptString_monitorConfig(obj);
        addLineProperties(script, obj, ident);
    }
    else if (obj->Type->isStack())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_stackObjectStart(obj);
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + "-->-- stack elements for " + obj->Name;
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + " Values of x, y, z only matter for the stack element, refered to at InitializeStack below";
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + " For the rest of elements they are calculated automatically";
        objectMembersToScript(obj, script, medIdent, useStrings, bRecursive);
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + "--<-- stack elements end for " + obj->Name;
        if (!obj->HostedObjects.empty())
            script += "\n" + QString(" ").repeated(ident)+ makeScriptString_stackObjectEnd(obj);
    }
    else if (obj->Type->isGrid())
    {
        script += "\n";
        script += "\n" + QString(" ").repeated(ident)+ CommentStr + "=== Optical grid object is not supported! Make a request to the developers ===";
        script += "\n";
    }
    else if (obj->Type->isInstance())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_instanceObject(obj);
    }
    else if (obj->Type->isPrototype())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_prototypeObject(obj);
        if (bRecursive) objectMembersToScript(obj, script, medIdent, useStrings, bRecursive);
    }

    if (obj->isDisabled())
    {
        script += "\n" + QString(" ").repeated(ident)+ makeScriptString_DisabledObject(obj);
    }
}

#include <array>
QString AGeoScriptMaker::makeScriptString_basicObject(AGeoObject * obj, bool useStrings) const
{
    std::array<QString, 3> posStrs;
    std::array<QString, 3> oriStrs;
    for (int i = 0; i < 3; i++)
    {
        posStrs[i] = ( obj->PositionStr[i].isEmpty()    ? QString::number(obj->Position[i])    : obj->PositionStr[i]    );
        oriStrs[i] = ( obj->OrientationStr[i].isEmpty() ? QString::number(obj->Orientation[i]) : obj->OrientationStr[i] );
    }

    const QStringList MatNames = AMaterialHub::getInstance().getListOfMaterialNames();
    const QString matStr = ( useStrings && obj->Material < MatNames.size() ? MatNames[obj->Material] + "_mat"
                                                                                 : QString::number(obj->Material) );
    QString str = obj->Shape->getScriptString(true);
    if (!str.isEmpty())
    {
        str.replace("$name$", "'" + obj->Name + "'");
        if (Language == Python) convertToPython(str);
    }
    else
    {
        QString GenerationString = obj->Shape->getGenerationString(true);
        if (Python == Language) GenerationString = getPythonGenerationString(GenerationString);

        str = QString("geo.customTGeo( ") +
                      "'" + obj->Name + "', " +
                      "'" + GenerationString + "', ";
    }

    str += matStr + ", " +
           "'" + obj->Container->Name + "',   " +
           ArrBeginStr +
           posStrs[0] + ", " +
           posStrs[1] + ", " +
           posStrs[2] +
           ArrEndStr + ",  " +
           ArrBeginStr +
           oriStrs[0] + ", " +
           oriStrs[1] + ", " +
           oriStrs[2] +
           ArrEndStr +
           " )";

    AGeoConsts::getConstInstance().formulaToScript(str, (Python == Language) );
    return str;
}

QString AGeoScriptMaker::makeScriptString_arrayObject(AGeoObject * obj) const
{
    QString str;

    if (obj->Type->isCircularArray())
    {
        ATypeCircularArrayObject * c = static_cast<ATypeCircularArrayObject*>(obj->Type);
        QString snum   = (c  ->strNum           .isEmpty() ? QString::number(c  ->num)               : c->strNum);
        QString sstep  = (c  ->strAngularStep   .isEmpty() ? QString::number(c  ->angularStep)       : c->strAngularStep);
        QString srad   = (c  ->strRadius        .isEmpty() ? QString::number(c  ->radius)            : c->strRadius);
        QString sPos0  = (obj->PositionStr[0]   .isEmpty() ? QString::number(obj->Position[0])       : obj->PositionStr[0]);
        QString sPos1  = (obj->PositionStr[1]   .isEmpty() ? QString::number(obj->Position[1])       : obj->PositionStr[1]);
        QString sPos2  = (obj->PositionStr[2]   .isEmpty() ? QString::number(obj->Position[2])       : obj->PositionStr[2]);
        QString sOri0  = (obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0])    : obj->OrientationStr[0]);
        QString sOri1  = (obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1])    : obj->OrientationStr[1]);
        QString sOri2  = (obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2])    : obj->OrientationStr[2]);
        QString sIndex = (c->strStartIndex      .isEmpty() ? QString::number(c  ->startIndex)        : c->strStartIndex);

        str +=  QString("geo.circArray( ") +
                "'" + obj->Name + "', " +
                snum + ", " +
                sstep + ", " +
                srad + ",   " +
                "'" + obj->Container->Name + "',   " +
                ArrBeginStr +
                sPos0 + ", " +
                sPos1 + ", " +
                sPos2 +
                ArrEndStr + ", " +
                ArrBeginStr +
                sOri0 + ",   " +
                sOri1 + ",   " +
                sOri2 +
                ArrEndStr + ", " +
                sIndex + " )";

        return str;
    }

    if (obj->Type->isHexagonalArray())
    {
        ATypeHexagonalArrayObject * h = static_cast<ATypeHexagonalArrayObject*>(obj->Type);
        QString sStep  = (h  ->strStep          .isEmpty() ? QString::number(h  ->Step)              : h->strStep);
        QString sRings = (h  ->strRings         .isEmpty() ? QString::number(h  ->Rings)             : h->strRings);
        QString sNumX  = (h  ->strNumX          .isEmpty() ? QString::number(h  ->NumX)              : h->strNumX);
        QString sNumY  = (h  ->strNumY          .isEmpty() ? QString::number(h  ->NumY)              : h->strNumY);
        QString sPos0  = (obj->PositionStr[0]   .isEmpty() ? QString::number(obj->Position[0])       : obj->PositionStr[0]);
        QString sPos1  = (obj->PositionStr[1]   .isEmpty() ? QString::number(obj->Position[1])       : obj->PositionStr[1]);
        QString sPos2  = (obj->PositionStr[2]   .isEmpty() ? QString::number(obj->Position[2])       : obj->PositionStr[2]);
        QString sOri0  = (obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0])    : obj->OrientationStr[0]);
        QString sOri1  = (obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1])    : obj->OrientationStr[1]);
        QString sOri2  = (obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2])    : obj->OrientationStr[2]);
        QString sIndex = (h->strStartIndex      .isEmpty() ? QString::number(h  ->startIndex)        : h->strStartIndex);

        if (h->Shape == ATypeHexagonalArrayObject::Hexagonal)
        {
            str += QString("geo.hexArray( ") +
                   "'" + obj->Name + "', " +
                   sRings + ", " +
                   sStep + ",   ";
        }
        else
        {
            str += QString("geo.hexArray_rectangular( ") +
                   "'" + obj->Name + "', " +
                   sNumX + ", " +
                   sNumY + ", " +
                   sStep + ", " +
                   (h->SkipOddLast ? TrueStr : FalseStr) + ",   ";
        }
        str += "'" + obj->Container->Name + "',   " +
               ArrBeginStr +
               sPos0 + ", " +
               sPos1 + ", " +
               sPos2 +
               ArrEndStr + ", " +
               ArrBeginStr +
               sOri0 + ", " +
               sOri1 + ", " +
               sOri2 +
               ArrEndStr + ", " +
               sIndex + " )";

        return str;
    }

    if (!obj->Type->isArray())
    {
        qWarning() << "Error accessing object as array!";
        return "Error accessing object as array!";
    }
    ATypeArrayObject * a = static_cast<ATypeArrayObject*>(obj->Type);
    QString snumX  = (a  ->strNumX          .isEmpty() ? QString::number(a  ->numX)              : a->strNumX);
    QString snumY  = (a  ->strNumY          .isEmpty() ? QString::number(a  ->numY)              : a->strNumY);
    QString snumZ  = (a  ->strNumZ          .isEmpty() ? QString::number(a  ->numZ)              : a->strNumZ);
    QString sstepX = (a  ->strStepX         .isEmpty() ? QString::number(a  ->stepX)             : a->strStepX);
    QString sstepY = (a  ->strStepY         .isEmpty() ? QString::number(a  ->stepY)             : a->strStepY);
    QString sstepZ = (a  ->strStepZ         .isEmpty() ? QString::number(a  ->stepZ)             : a->strStepZ);
    QString sPos0  = (obj->PositionStr[0]   .isEmpty() ? QString::number(obj->Position[0])       : obj->PositionStr[0]);
    QString sPos1  = (obj->PositionStr[1]   .isEmpty() ? QString::number(obj->Position[1])       : obj->PositionStr[1]);
    QString sPos2  = (obj->PositionStr[2]   .isEmpty() ? QString::number(obj->Position[2])       : obj->PositionStr[2]);
    QString sOri0  = (obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0])    : obj->OrientationStr[0]);
    QString sOri1  = (obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1])    : obj->OrientationStr[1]);
    QString sOri2  = (obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2])    : obj->OrientationStr[2]);
    QString sIndex = (a->strStartIndex      .isEmpty() ? QString::number(a  ->startIndex)        : a->strStartIndex);

    str +=  QString("geo.array( ") +
            "'" + obj->Name + "', " +
            ArrBeginStr +
            snumX + ", " +
            snumY + ", " +
            snumZ +
            ArrEndStr + ", " +
            ArrBeginStr +
            sstepX + ", " +
            sstepY + ", " +
            sstepZ +
            ArrEndStr + ", "
            "'" + obj->Container->Name + "',   " +
            ArrBeginStr +
            sPos0 + ", " +
            sPos1 + ", " +
            sPos2 +
            ArrEndStr + ", " +
            ArrBeginStr +
            sOri0 + ", " +
            sOri1 + ", " +
            sOri2 +
            ArrEndStr + ", " +
            (a->bCenterSymmetric ? TrueStr : FalseStr) + ",  " +
            sIndex + " )";

    return str;
}

QString AGeoScriptMaker::makeScriptString_instanceObject(AGeoObject * obj) const
{
    ATypeInstanceObject * ins = dynamic_cast<ATypeInstanceObject*>(obj->Type);
    if (!ins)
    {
        qWarning() << "It is not an instance!";
        return "Error accessing object as instance!";
    }

    QVector<QString> posStrs(3);
    QVector<QString> oriStrs(3);
    for (int i = 0; i < 3; i++)
    {
        posStrs[i] = ( obj->PositionStr[i].isEmpty()    ? QString::number(obj->Position[i])    : obj->PositionStr[i] );
        oriStrs[i] = ( obj->OrientationStr[i].isEmpty() ? QString::number(obj->Orientation[i]) : obj->OrientationStr[i] );
    }

    QString str =  QString("geo.instance( ") +
                   "'" + obj->Name +            "', " +
                   "'" + ins->PrototypeName +   "', " +
                   "'" + obj->Container->Name + "',   " +
                   ArrBeginStr +
                   posStrs[0] + ", " +
                   posStrs[1] + ", " +
                   posStrs[2] +
                   ArrEndStr + ", " +
                   ArrBeginStr +
                   oriStrs[0] + ", " +
                   oriStrs[1] + ", " +
                   oriStrs[2] +
                   ArrEndStr + " )";

    AGeoConsts::getConstInstance().formulaToScript(str, (Python == Language) );
    return str;
}

QString AGeoScriptMaker::makeScriptString_prototypeObject(AGeoObject * obj) const
{
    ATypePrototypeObject * pro = dynamic_cast<ATypePrototypeObject*>(obj->Type);
    if (!pro)
    {
        qWarning() << "It is not a prototype!";
        return "Error accessing object as prototype!";
    }

    QString str =  QString("geo.prototype( ") +
                   "'" + obj->Name + "' )";
    return str;
}

QString AGeoScriptMaker::makeScriptString_monitorBaseObject(const AGeoObject * obj) const
{
    ATypeMonitorObject * m = dynamic_cast<ATypeMonitorObject*>(obj->Type);
    if (!m)
    {
        qWarning() << "It is not a monitor!";
        return "Error accessing monitor!";
    }

    const AMonitorConfig & c = m->config;

    //             geo.monitor( name,shape,size1, size2,  container,  [x,  y,  z],  [phi,  theta,  psi],  SensitiveTop,  SensitiveBottom,  StopsTraking )
    QString s = QString("geo.monitor( %1,  %2,  %3, %4,  %5, [%6, %7, %8], [%9, %10, %11], %12, %13, %14 )")
            .arg("'" + obj->Name + "'")
            .arg(c.shape)
            .arg(c.str2size1.isEmpty() ? QString::number(2.0 * c.size1) : c.str2size1)
            .arg(c.str2size2.isEmpty() ? QString::number(2.0 * c.size2) : c.str2size2)
            .arg("'" + obj->Container->Name + "'")
            .arg(obj->PositionStr[0].isEmpty() ? QString::number(obj->Position[0]) : obj->PositionStr[0])
            .arg(obj->PositionStr[1].isEmpty() ? QString::number(obj->Position[1]) : obj->PositionStr[1])
            .arg(obj->PositionStr[2].isEmpty() ? QString::number(obj->Position[2]) : obj->PositionStr[2])
            .arg(obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0]) : obj->OrientationStr[0])
            .arg(obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1]) : obj->OrientationStr[1])
            .arg(obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2]) : obj->OrientationStr[2])
            .arg(c.bUpper        ? TrueStr : FalseStr)
            .arg(c.bLower        ? TrueStr : FalseStr)
            .arg(c.bStopTracking ? TrueStr : FalseStr);

    if (Language == Python) convertToPython(s);
    return s;
}

QString AGeoScriptMaker::makeScriptString_monitorConfig(const AGeoObject *obj) const
{
    ATypeMonitorObject * m = dynamic_cast<ATypeMonitorObject*>(obj->Type);
    if (!m)
    {
        qWarning() << "It is not a monitor!";
        return "Error accessing monitor!";
    }
    const AMonitorConfig & c = m->config;

    if (c.PhotonOrParticle == 0)
    {
        //geo.configurePhotonMonitor( MonitorName,  Position,  Time,  Angle,  Wave )
        return QString("geo.configurePhotonMonitor( %1,  [%2, %3],  [%4, %5, %6],  [%7, %8, %9],  [%10, %11, %12] )")
                .arg("'" + obj->Name + "'")
                .arg(c.xbins)
                .arg(c.ybins)
                .arg(c.timeBins)
                .arg(c.timeFrom)
                .arg(c.timeTo)
                .arg(c.angleBins)
                .arg(c.angleFrom)
                .arg(c.angleTo)
                .arg(c.waveBins)
                .arg(c.waveFrom)
                .arg(c.waveTo);
    }
    else
    {
        //geo.configureParticleMonitor( MonitorName,  Particle,  Both_Primary_Secondary,  Both_Direct_Indirect,  Position,  Time,  Angle,  Energy )
        return QString("geo.configureParticleMonitor( %1,  %2,  %3,  %4,   [%5, %6],  [%7, %8, %9, %10],  [%11, %12, %13],  [%14, %15, %16, %17] )")
                .arg("'" + obj->Name + "'")
                .arg("'" + c.Particle + "'")
                .arg(c.bPrimary && c.bSecondary ? 0 : (c.bPrimary ? 1 : 2))
                .arg(c.bDirect  && c.bIndirect  ? 0 : (c.bDirect  ? 1 : 2))
                .arg(c.xbins)
                .arg(c.ybins)
                .arg(c.timeBins)
                .arg(c.timeFrom)
                .arg(c.timeTo)
                .arg("'" + c.timeUnits + "'")
                .arg(c.angleBins)
                .arg(c.angleFrom)
                .arg(c.angleTo)
                .arg(c.energyBins)
                .arg(c.energyFrom)
                .arg(c.energyTo)
                .arg("'" + c.energyUnits + "'");
    }
}

QString AGeoScriptMaker::makeScriptString_stackObjectStart(AGeoObject * obj) const
{
    return  QString("geo.stack( '%1', '%2',  [%3, %4, %5], [%6, %7, %8] )")
            .arg(obj->Name)
            .arg(obj->Container->Name)
            .arg(obj->PositionStr[0].isEmpty() ? QString::number(obj->Position[0]) : obj->PositionStr[0])
            .arg(obj->PositionStr[1].isEmpty() ? QString::number(obj->Position[1]) : obj->PositionStr[1])
            .arg(obj->PositionStr[2].isEmpty() ? QString::number(obj->Position[2]) : obj->PositionStr[2])
            .arg(obj->OrientationStr[0].isEmpty() ? QString::number(obj->Orientation[0]) : obj->OrientationStr[0])
            .arg(obj->OrientationStr[1].isEmpty() ? QString::number(obj->Orientation[1]) : obj->OrientationStr[1])
            .arg(obj->OrientationStr[2].isEmpty() ? QString::number(obj->Orientation[2]) : obj->OrientationStr[2]);
}

QString AGeoScriptMaker::makeScriptString_stackObjectEnd(AGeoObject * obj) const
{
    return QString("geo.initializeStack( ") +
                   "'" + obj->Name + "',  " +
                   "'" + obj->getOrMakeStackReferenceVolume()->Name + "' )";
}

QString AGeoScriptMaker::makeLinePropertiesString(AGeoObject * obj) const
{
    return "geo.setLineProperties( '" +
            obj->Name +
            "',  " +
            QString::number(obj->color) + ",  " +
            QString::number(obj->width) + ",  " +
            QString::number(obj->style) + " )";
}

QString AGeoScriptMaker::makeScriptString_DisabledObject(AGeoObject * obj) const
{
    return QString("geo.setEnabled( '%1', " + FalseStr + ")").arg(obj->Name);
}

void AGeoScriptMaker::addLineProperties(QString & script, AGeoObject * obj, int ident)
{
    if ( (obj->color == 1 || obj->color == -1) &&
         obj->width == 1 &&
         obj->style == 1 ) return;
    script += "\n" + QString(" ").repeated(ident) + makeLinePropertiesString(obj);
}

void AGeoScriptMaker::addScaledIfApplicable(QString & script, AGeoObject * obj, int ident, bool useStrings)
{
    if (obj->Shape->getShapeType() != QStringLiteral("TGeoScaledShape")) return;

    AGeoScaledShape * scs = static_cast<AGeoScaledShape*>(obj->Shape);

    QString str = scs->getScriptString_Scaled(useStrings);
    str.replace("$name$", "'" + obj->Name + "'");

    script += "\n" + QString(" ").repeated(ident) + str;
}

void AGeoScriptMaker::addRoleIfApplicable(QString & script, AGeoObject *obj, int ident, bool useStrings)
{
    if (!obj->Role) return;

    AGeoCalorimeter * cal = dynamic_cast<AGeoCalorimeter*>(obj->Role);
    if (cal)
    {
        QString bins, origin, step;
        cal->Properties.toStrings(origin, step, bins, useStrings);
        if (Language == Python)
        {
            convertToPython(bins);
            convertToPython(origin);
            convertToPython(step);
        }

        QString str = QString("geo.setCalorimeter( '%0', %1, %2, %3 )").arg(obj->Name, bins, origin, step);
        AGeoConsts::getConstInstance().formulaToScript(str, false);
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }

    AGeoSensor * pm = dynamic_cast<AGeoSensor*>(obj->Role);
    if (pm)
    {
        //void setLightSensor(QString Object, int iModel = 0);
        QString str = QString("geo.setLightSensor( '%0', %1 )").arg(obj->Name, QString::number(pm->SensorModel));
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }

    AGeoScint * sci = dynamic_cast<AGeoScint*>(obj->Role);
    if (sci)
    {
        QString str = QString("geo.setScintillator( '%1' )").arg(obj->Name);
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }

    AGeoSecScint * sec = dynamic_cast<AGeoSecScint*>(obj->Role);
    if (sec)
    {
        QString str = QString("geo.setSecondaryScintillator( '%1' )").arg(obj->Name);
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }

    AGeoPhotonTunnelIn * tunIn = dynamic_cast<AGeoPhotonTunnelIn*>(obj->Role);
    if (tunIn)
    {
        QString str = QString("geo.setPhotonTunnelIn( '%1' )").arg(obj->Name);
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }

    AGeoPhotonTunnelOut * tunOut = dynamic_cast<AGeoPhotonTunnelOut*>(obj->Role);
    if (tunOut)
    {
        QString str = QString("geo.setPhotonTunnelOut( '%1' )").arg(obj->Name);
        script += "\n" + QString(" ").repeated(ident) + str;
        return;
    }
}

QString AGeoScriptMaker::getPythonGenerationString(const QString & javaGenString) const
{
    int numberofQ = javaGenString.count("'");
    if (numberofQ == 0) return javaGenString;

    QString PythonGenString = javaGenString;
    const QString firstStr = " )";
    const QString secondStr = " str(";
    int   plusSize = QString(" + ").size();

    bool first = true;
    for (int i = 0; i < PythonGenString.size(); i++)
    {
        if (PythonGenString.at(i) == '\'')
        {
            if (!first)
            {
                PythonGenString.insert(i - plusSize , firstStr);
                i += firstStr.size();
            }
            else
            {
                PythonGenString.insert(i + plusSize , secondStr);
                i += secondStr.size();
            }
            first = !first;

        }
    }
    return PythonGenString;
}

void AGeoScriptMaker::convertToPython(QString & str) const
{
    if (Language == Python)
    {
        str.replace('[', '(');
        str.replace(']', ')');
    }
}
