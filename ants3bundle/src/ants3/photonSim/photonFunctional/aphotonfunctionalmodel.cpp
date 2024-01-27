#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"

APhotonFunctionalModel * APhotonFunctionalModel::factory(const QString & type)
{
    if (type == "Dummy")        return new APFM_Dummy();
    if (type == "ThinLens")     return new APFM_ThinLens();
    if (type == "OpticalFiber") return new APFM_OpticalFiber();

    qWarning() << "Photom functional model type (" << type << ") is unknown, returning Dummy model";
    return new APFM_Dummy();
}

APhotonFunctionalModel * APhotonFunctionalModel::factory(QJsonObject & json)
{
    QString type;
    jstools::parseJson(json, "Type", type);
    APhotonFunctionalModel * model = APhotonFunctionalModel::factory(type);
    model->readFromJson(json);
    return model;
}

void APhotonFunctionalModel::writeToJson(QJsonObject & json) const
{
    json["Type"] = getType();

    QJsonObject js;
    writeSettingsToJson(js);
    json["Settings"] = js;
}

void APhotonFunctionalModel::readFromJson(const QJsonObject & json)
{
    // Type is external as it will be used to construct the model
    QJsonObject js;
    jstools::parseJson(json, "Settings", js);
    readSettingsFromJson(js);
}

// ---

void APFM_OpticalFiber::writeSettingsToJson(QJsonObject & json) const
{
    json["Length_mm"] = Length_mm;
    json["MaxAngle_deg"] = MaxAngle_deg;
}

void APFM_OpticalFiber::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Length_mm", Length_mm);
    jstools::parseJson(json, "MaxAngle_deg", MaxAngle_deg);
}

QString APFM_OpticalFiber::printSettingsToString() const
{
    return QString("L = %0 mm; MaxAngle = %1 deg").arg(Length_mm).arg(MaxAngle_deg);
}

bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target)
{
    // check angle of incidence inside acceptance come
    //if (photonData.LocalDirection[])

    // check absorption

    photonData.LocalPosition[2] = 0;

    // time increase

    return true;
}

// ---

void APFM_ThinLens::writeSettingsToJson(QJsonObject & json) const
{
    json["FocalLength_mm"] = FocalLength_mm;
}

void APFM_ThinLens::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "FocalLength_mm", FocalLength_mm);
}

QString APFM_ThinLens::printSettingsToString() const
{
    return QString("F = %0 mm").arg(FocalLength_mm);
}

#include "avector.h"
bool APFM_ThinLens::applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target)
{
    // ALine3D a1({1,2,3},{0,-2,2});
    // ALine3D a2({0,1,2},{2,3,4});
    // AVector3 in = a1.getIntersect(a2);
    // qDebug() <<  "crossssssss"<<in[0] << in[1] << in[2];


    // !!!*** guard against side entrance when Zdirection component is 0

    // transport to central plane
    const double ZatContact = photonData.LocalPosition[2]; // never = 0
    const double travelFactor = fabs( photonData.LocalDirection[2] / ZatContact ); // = 1 over Number of Z-projections of direction unit vector to get to center plane
    const double XatZ0 = photonData.LocalPosition[0] + photonData.LocalDirection[0] / travelFactor;
    const double YatZ0 = photonData.LocalPosition[1] + photonData.LocalDirection[1] / travelFactor;


    const double travelFactor2f = fabs( 0.5 * photonData.LocalDirection[2] / FocalLength_mm );
    const double XatMinus2f = XatZ0 - photonData.LocalDirection[0] / travelFactor2f;
    const double YatMinus2f = YatZ0 - photonData.LocalDirection[1] / travelFactor2f;

    // first line contains (XatMinus2f, YatMinus2f, 2f) and (0,0,0) points
    // second line contains (XatMinus2f, YatMinus2f, 0) and (0,0,-f) points

    // too many zeros in coefficents, need 2D case
    //ALine3D first({XatMinus2f, YatMinus2f, 2.0*FocalLength_mm}, {0,0,0});
    //ALine3D second({XatMinus2f, YatMinus2f, 0}, {0,0,-FocalLength_mm});
    //AVector3 crossing;
    //bool ok = first.getIntersect(second, crossing);
    //qDebug() << "ok?" << ok << "crossing" << crossing[0] << crossing[1] << crossing[2];

    // looking for crossing in the plane containing Z axis and vecor (XatMinus2f, YatMinus2f,0)
    const double R = sqrt(XatMinus2f * XatMinus2f + YatMinus2f * YatMinus2f);
      // todo R = 0 case !!!***
    ALine2D first({R, 2.0*FocalLength_mm}, {0,0});
    ALine2D second({R, 0}, {0,-FocalLength_mm});
    AVector2 crossing2D;
    bool ok = first.getIntersect(second, crossing2D);
    qDebug() << "ok?" << ok << "crossing2D" << crossing2D[0] << crossing2D[1];
     // todo: process not ok case!!!***
    AVector3 crossing{crossing2D[0]*XatMinus2f/R, crossing2D[0]*YatMinus2f/R, crossing2D[1]};

    AVector3 newDir = crossing - AVector3{XatZ0, YatZ0, 0};
    newDir.toUnitVector();

    photonData.LocalPosition[0] = XatZ0;
    photonData.LocalPosition[1] = YatZ0;
    photonData.LocalPosition[2] = 0;
    photonData.LocalDirection[0] = newDir[0];
    photonData.LocalDirection[1] = newDir[1];
    photonData.LocalDirection[2] = newDir[2];

    // distance from lens center

    // unitVectors
    // //ZUnit - (0,0,dz)->unit
    // RUnit - (XatZ0, YatZ0, 0) -> unitLength
    // SideUnit: (-YatZ0, XatZ0, 0) -> unitLength

    // rotate by angle = - artan(R / F)
    // rotate (dx,dy,dz) around SideUnit by angle

//    double rotAngle = -atan(R / FocalLength_mm);
//    AVector3 dir(photonData.LocalDirection);
//    dir.rotate(-rotAngle, AVector3(-YatZ0, XatZ0, 0).toUnitVector());

    // projections
    // Z-Proj = dz
    // R_Proj = dot((dx,dy,dz),(RUnit))
    // Side_Proj = dot((dxdydz),(SideUnit))

    // ZRvecor

    // initial angle
    // start = arctn(R_Proj/Z_proj)

    // lens changes angle
    // end = start - artan(R / F)

    // new vectors
    // newZproj =

/*
    // test
    photonData.LocalPosition[0] = XatZ0;
    photonData.LocalPosition[1] = YatZ0;
    photonData.LocalPosition[2] = 0;
    photonData.LocalDirection[2] = -photonData.LocalDirection[2];
*/
    /*
    photonData.LocalPosition[0] = XatZ0;
    photonData.LocalPosition[1] = YatZ0;
    photonData.LocalPosition[2] = 0;
    photonData.LocalDirection[0] = dir[0];
    photonData.LocalDirection[1] = dir[1];
    photonData.LocalDirection[2] = dir[2];
*/
    return true;
}
