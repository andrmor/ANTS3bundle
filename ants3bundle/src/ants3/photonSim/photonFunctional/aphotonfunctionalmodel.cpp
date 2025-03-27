#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"
#include "avector.h"
#include "ageometryhub.h"

#include "TGeoNode.h"

// synchronize with APhotonFunctionalModel::factory !!!
QStringList APhotonFunctionalModel::getKnownModels()
{
    return {"Dummy", "ThinLens", "OpticalFiber", "Filter"};
}

// synchronize with APhotonFunctionalModel::getKnownModels !!!
APhotonFunctionalModel * APhotonFunctionalModel::factory(const QString & type)
{
    if (type == "Dummy")        return new APFM_Dummy();
    if (type == "ThinLens")     return new APFM_ThinLens();
    if (type == "OpticalFiber") return new APFM_OpticalFiber();
    if (type == "Filter")       return new APFM_Filter();

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

APhotonFunctionalModel * APhotonFunctionalModel::clone(const APhotonFunctionalModel * other)
{
    if (!other) return nullptr;

    QJsonObject js;
    other->writeToJson(js);
    return APhotonFunctionalModel::factory(js);
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

QString APFM_OpticalFiber::checkModel() const
{
    if (Length_mm < 0) return "Fiber length cannot be negative";

    if (MaxAngle_deg < 0) return "Max angle cannot be negative";

    for (const auto & p : MaxAngleSpectrum_deg)
        if (p.second < 0) return "Max angle cannot be negative! Check wavelength-resolved data.";

    return "";
}

void APFM_OpticalFiber::writeSettingsToJson(QJsonObject & json) const
{
    json["Length_mm"] = Length_mm;
    json["MaxAngle_deg"] = MaxAngle_deg;

    QJsonArray ar;
    jstools::writeDPairVectorToArray(MaxAngleSpectrum_deg, ar);
    json["MaxAngleSpectrum_deg"] = ar;
}

void APFM_OpticalFiber::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Length_mm", Length_mm);
    jstools::parseJson(json, "MaxAngle_deg", MaxAngle_deg);

    MaxAngleSpectrum_deg.clear();
    QJsonArray ar;
    jstools::parseJson(json, "MaxAngleSpectrum_deg", ar);
    jstools::readDPairVectorFromArray(ar, MaxAngleSpectrum_deg);
}

QString APFM_OpticalFiber::printSettingsToString() const
{
    QString txt = QString("L = %0 mm; ").arg(Length_mm);

    if (MaxAngleSpectrum_deg.empty())
        txt += QString("MaxAngle = %1 deg").arg(MaxAngle_deg);
    else
        txt += QString("MaxAngle(%0): %1 points; for not wavelength-resolved sim: %2 deg").arg(QChar(0x3bb)).arg(MaxAngleSpectrum_deg.size()).arg(MaxAngle_deg);

    return txt;

}

QString APFM_OpticalFiber::updateRuntimeProperties()
{
    QString err = APFM_OpticalFiber::checkModel();
    if (!err.isEmpty()) return err;

    _TanMaxAngle = tan(MaxAngle_deg * 3.1415926535 / 180.0);

    _TanMaxAngleSpectrumBinned.clear();
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (WaveSet.Enabled)
    {
        if (MaxAngleSpectrum_deg.empty())
            _TanMaxAngleSpectrumBinned = std::vector<double>(WaveSet.countNodes(), MaxAngle_deg);
        else
            WaveSet.toStandardBins(MaxAngleSpectrum_deg, _TanMaxAngleSpectrumBinned);

        for (size_t i = 0; i < _TanMaxAngleSpectrumBinned.size(); i++)
            _TanMaxAngleSpectrumBinned[i] = tan(_TanMaxAngleSpectrumBinned[i] * 3.1415926535 / 180.0);
    }

    return "";
}

#include "ageoobject.h"
#include "amaterialhub.h"
bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, int index, int /*linkedToIndex*/)
{
    // check angle inside is within max angle
    if (photonData.LocalDirection[2] == 0) return false;
    const double tanAngle = sqrt(photonData.LocalDirection[0]*photonData.LocalDirection[0] + photonData.LocalDirection[1]*photonData.LocalDirection[1]) / fabs(photonData.LocalDirection[2]);
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    double maxTan;
    if (photonData.WaveIndex == -1 || !WaveSet.Enabled)
        maxTan = _TanMaxAngle;
    else
        maxTan = _TanMaxAngleSpectrumBinned[photonData.WaveIndex];
    //qDebug() << "\ntan:" << tanAngle << " max tan:" << maxTan;
    if (tanAngle > maxTan) return false;

    // check absorption
    const AGeoObject * obj = std::get<0>(AGeometryHub::getConstInstance().PhotonFunctionals[index]);
    const int iMat = obj->Material;
    const AMaterial * mat = AMaterialHub::getConstInstance()[iMat];
    const double absCoeff = mat->getAbsorptionCoefficient(photonData.WaveIndex); // mm-1
    const double absProb = 1.0 - exp( - absCoeff * Length_mm);
    //qDebug() << "abs prob:" << absProb;
    if (ARandomHub::getInstance().uniform() < absProb) return false;

    // teleporting
    //qDebug() << photonData.LocalPosition[2];
    if (photonData.LocalPosition[2] != 0)
    {
        const double sign = photonData.LocalPosition[2] / fabs(photonData.LocalPosition[2]);
        photonData.LocalPosition[2] -= sign * 1e-9; // safity to be inside
        photonData.LocalPosition[2] = - photonData.LocalPosition[2]; // on the other side
    }
    //qDebug() << photonData.LocalPosition[2];

    // time increase
    const double speed = mat->getSpeedOfLight(photonData.WaveIndex); // mm/ns
    const double deltaT = Length_mm * sqrt(1.0 + tanAngle * tanAngle) / speed;
    //qDebug() << "t0" << photonData.Time << "speed" << speed << "deltaT" << deltaT;
    photonData.Time += deltaT;

    return true;
}

// ---

QString APFM_ThinLens::checkModel() const
{
    if (FocalLength_mm == 0) return "Focal length cannot be zero!";

    for (const auto & p : FocalLengthSpectrum_mm)
        if (p.second == 0) return "Focal length cannot be zero! Check wavelength-resolved data.";

    return "";
}

void APFM_ThinLens::writeSettingsToJson(QJsonObject & json) const
{
    json["FocalLength_mm"] = FocalLength_mm;

    QJsonArray ar;
    jstools::writeDPairVectorToArray(FocalLengthSpectrum_mm, ar);
    json["FocalLengthSpectrum_mm"] = ar;
}

void APFM_ThinLens::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "FocalLength_mm", FocalLength_mm);

    FocalLengthSpectrum_mm.clear();
    QJsonArray ar;
    jstools::parseJson(json, "FocalLengthSpectrum_mm", ar);
    jstools::readDPairVectorFromArray(ar, FocalLengthSpectrum_mm);
}

QString APFM_ThinLens::printSettingsToString() const
{
    if (FocalLengthSpectrum_mm.empty())
        return QString("F = %0 mm").arg(FocalLength_mm);

    return QString("FocalLength(%0): %1 points; for not wavelength-resolved sim: %2 mm").arg(QChar(0x3bb)).arg(FocalLengthSpectrum_mm.size()).arg(FocalLength_mm);
}

QString APFM_ThinLens::updateRuntimeProperties()
{
    QString err = APFM_ThinLens::checkModel();
    if (!err.isEmpty()) return err;

    _FocalLengthBinned.clear();

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (WaveSet.Enabled)
    {
        if (!FocalLengthSpectrum_mm.empty())
            WaveSet.toStandardBins(FocalLengthSpectrum_mm, _FocalLengthBinned);
        else _FocalLengthBinned = std::vector<double>(WaveSet.countNodes(), FocalLength_mm);
    }
    return "";
}

// !!!*** == 0 to double safe version
bool APFM_ThinLens::applyModel(APhotonExchangeData & photonData, int index, int linkedToIndex)
{
    if (photonData.LocalDirection[2] == 0) return false;

    // transport to central plane
    const double ZatContact = photonData.LocalPosition[2]; // never = 0
    const double travelFactor = fabs( photonData.LocalDirection[2] / ZatContact ); // = 1 over Number of Z-projections of direction unit vector to get to center plane
    const double XatZ0 = photonData.LocalPosition[0] + photonData.LocalDirection[0] / travelFactor;
    const double YatZ0 = photonData.LocalPosition[1] + photonData.LocalDirection[1] / travelFactor;

    photonData.LocalPosition[0] = XatZ0;
    photonData.LocalPosition[1] = YatZ0;
    photonData.LocalPosition[2] = 0;

    TGeoNode * node = std::get<1>(AGeometryHub::getConstInstance().PhotonFunctionals[index]);
    if (node)
    {
        // check is this point inside the lens "aperture"
        bool bInside = node->GetVolume()->GetShape()->Contains(photonData.LocalPosition);
        if (!bInside) return false;
    }

    if (XatZ0 == 0 && YatZ0 == 0) return true; // no direction change in this case

    // get focal length
    double focalLength;
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (photonData.WaveIndex == -1 || !WaveSet.Enabled) focalLength = FocalLength_mm;
    else focalLength = _FocalLengthBinned[photonData.WaveIndex];

    const double signFocalLength = ( (focalLength > 0) ? 1.0 : -1.0);
    const double signDirection = ( (photonData.LocalDirection[2] > 0) ? 1.0 : -1.0);

    const double travelFactor2f = 0.5 * fabs(photonData.LocalDirection[2]) / focalLength;
    const double XatMinus2f = XatZ0 - photonData.LocalDirection[0] / travelFactor2f;
    const double YatMinus2f = YatZ0 - photonData.LocalDirection[1] / travelFactor2f;

    // first line contains (XatMinus2f, YatMinus2f, 2f) and (0,0,0) points
    // second line contains (XatMinus2f, YatMinus2f, 0) and (0,0,-f) points

    // too many zeros in coefficents, need 2D case
    //ALine3D first({XatMinus2f, YatMinus2f, 2.0*focalLength}, {0,0,0});
    //ALine3D second({XatMinus2f, YatMinus2f, 0}, {0,0,-focalLength});
    //AVector3 crossing;
    //bool ok = first.getIntersect(second, crossing);
    //qDebug() << "ok?" << ok << "crossing" << crossing[0] << crossing[1] << crossing[2];

    // looking for crossing in the plane containing Z axis and vecor (XatMinus2f, YatMinus2f,0)
    const double R = sqrt(XatMinus2f * XatMinus2f + YatMinus2f * YatMinus2f);
    if (R == 0)
    {
        photonData.LocalDirection[0] = -photonData.LocalDirection[0];
        photonData.LocalDirection[1] = -photonData.LocalDirection[1];
        return true;
    }

    ALine2D first({R, -2.0*signDirection*focalLength}, {0,0});
    ALine2D second({R, 0}, {0,signDirection*focalLength});
    AVector2 crossing2D;
    bool ok = first.getIntersect(second, crossing2D);
    //qDebug() << "ok?" << ok << "crossing2D" << crossing2D[0] << crossing2D[1];
        // todo: process not ok case!!!***  seems it cannot appear for allowed conditions
    if (!ok) exit(888);
    AVector3 crossing{crossing2D[0]*XatMinus2f/R, crossing2D[0]*YatMinus2f/R, crossing2D[1]};

    AVector3 newDir = crossing - AVector3{XatZ0, YatZ0, 0};
    newDir.toUnitVector();

    photonData.LocalDirection[0] = signFocalLength * newDir[0];
    photonData.LocalDirection[1] = signFocalLength * newDir[1];
    photonData.LocalDirection[2] = signFocalLength * newDir[2];

    return true;
}

// ---

QString APFM_Filter::checkModel() const
{
    if (GrayTransmission < 0 || GrayTransmission > 1.0) return "Filter transmission values should be within [0, 1] range";

    for (const auto & p : TransmissionSpectrum)
        if (p.second < 0 || p.second > 1.0) return "Filter transmission values should be within [0, 1] range. Check wavelength-resolved data";

    return "";
}

void APFM_Filter::writeSettingsToJson(QJsonObject & json) const
{
    json["Gray"] = Gray;
    json["GrayTransmission"] = GrayTransmission;

    QJsonArray ar;
    jstools::writeDPairVectorToArray(TransmissionSpectrum, ar);
    json["TransmissionSpectrum"] = ar;
}

void APFM_Filter::readSettingsFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Gray", Gray);
    jstools::parseJson(json, "GrayTransmission", GrayTransmission);

    TransmissionSpectrum.clear();
    QJsonArray ar;
    jstools::parseJson(json, "TransmissionSpectrum", ar);
    jstools::readDPairVectorFromArray(ar, TransmissionSpectrum);
}

QString APFM_Filter::printSettingsToString() const
{
    if (Gray)
        return QString("Gray filter with T = %0").arg(GrayTransmission);

    return QString("Transmission(%0): %1 points; for not wavelength-resolved sim: %2").arg(QChar(0x3bb)).arg(TransmissionSpectrum.size()).arg(GrayTransmission);
}

QString APFM_Filter::updateRuntimeProperties()
{
    QString err = APFM_Filter::checkModel();
    if (!err.isEmpty()) return err;

    _TransmissionBinned.clear();

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (WaveSet.Enabled)
    {
        if (!TransmissionSpectrum.empty())
            WaveSet.toStandardBins(TransmissionSpectrum, _TransmissionBinned);
        else _TransmissionBinned = std::vector<double>(WaveSet.countNodes(), GrayTransmission);
    }
    return "";
}

bool APFM_Filter::applyModel(APhotonExchangeData & photonData, int , int)
{
    double Trans;
    if (Gray || photonData.WaveIndex == -1) Trans = GrayTransmission;
    else Trans = _TransmissionBinned[photonData.WaveIndex];

    return (ARandomHub::getInstance().uniform() < Trans);
}

// ---
