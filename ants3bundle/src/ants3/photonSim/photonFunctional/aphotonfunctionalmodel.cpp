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
    // !!!*** guard against side entrance when Zdirection component is 0

    // transport to central plane
    const double ZatContact = photonData.LocalPosition[2]; // never = 0
    const double travelFactor = fabs( photonData.LocalDirection[2] / ZatContact ); // = 1 over Number of Z-projections of direction unit vector to get to center plane
    const double XatZ0 = photonData.LocalPosition[0] + photonData.LocalDirection[0] / travelFactor;
    const double YatZ0 = photonData.LocalPosition[1] + photonData.LocalDirection[1] / travelFactor;

    // distance from lens center
    const double R = sqrt(XatZ0 * XatZ0 + YatZ0 * YatZ0);

    // unitVectors
    // //ZUnit - (0,0,dz)->unit
    // RUnit - (XatZ0, YatZ0, 0) -> unitLength
    // SideUnit: (-YatZ0, XatZ0, 0) -> unitLength

    // rotate by angle = - artan(R / F)
    // rotate (dx,dy,dz) around SideUnit by angle

    double rotAngle = -atan(R / FocalLength_mm);
    AVector3 dir(photonData.LocalDirection);
    dir.rotate(-rotAngle, AVector3(-YatZ0, XatZ0, 0).toUnitVector());

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
    photonData.LocalPosition[0] = XatZ0;
    photonData.LocalPosition[1] = YatZ0;
    photonData.LocalPosition[2] = 0;
    photonData.LocalDirection[0] = dir[0];
    photonData.LocalDirection[1] = dir[1];
    photonData.LocalDirection[2] = dir[2];

    return true;
}
