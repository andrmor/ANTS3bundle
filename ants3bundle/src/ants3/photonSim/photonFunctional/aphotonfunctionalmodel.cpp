#include "aphotonfunctionalmodel.h"
#include "ajsontools.h"
#include "arandomhub.h"
#include "aphotonsimhub.h"
#include "avector.h"
#include "ageometryhub.h"
#include "aerrorhub.h"

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

    AErrorHub::addQError("Unknown photom functional model type: " + type);
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

    if (CutOffAngle_deg < 0) return "Cut-off angle cannot be negative";

    for (const auto & p : CutOffAngleSpectrum_deg)
        if (p.second < 0) return "Cut-off angle cannot be negative! Check wavelength-resolved data.";

    return "";
}

void APFM_OpticalFiber::writeSettingsToJson(QJsonObject & json) const
{
    json["Length_mm"] = Length_mm;
    json["CutOffAngle_deg"] = CutOffAngle_deg;

    {
        QJsonArray ar;
        jstools::writeDPairVectorToArray(CutOffAngleSpectrum_deg, ar);
        json["CutOffAngleSpectrum_deg"] = ar;
    }
}

void APFM_OpticalFiber::readSettingsFromJson(const QJsonObject & json)
{
    CutOffAngle_deg = 80.0;
    jstools::parseJson(json, "Length_mm", Length_mm);
    jstools::parseJson(json, "CutOffAngle_deg", CutOffAngle_deg);
    //jstools::parseJson(json, "AbsCoeff", AbsCoeff);

    {
        CutOffAngleSpectrum_deg.clear();
        QJsonArray ar;
        jstools::parseJson(json, "CutOffAngleSpectrum_deg", ar);
        jstools::readDPairVectorFromArray(ar, CutOffAngleSpectrum_deg);
    }
}

QString APFM_OpticalFiber::printSettingsToString() const
{
    QString txt = QString("L = %0 mm; ").arg(Length_mm);

    if (CutOffAngleSpectrum_deg.empty())
        txt += QString("CutOffAngle = %1 deg").arg(CutOffAngle_deg);
    else
        txt += QString("CutOffAngle(%0): %1 points; for not wavelength-resolved sim: %2 deg").arg(QChar(0x3bb)).arg(CutOffAngleSpectrum_deg.size()).arg(CutOffAngle_deg);

    return txt;
}

#include "ageoobject.h"
#include "ageoshape.h"
#include "amaterialhub.h"
QString APFM_OpticalFiber::updateRuntimeProperties(int iModel)
{
    QString err = APFM_OpticalFiber::checkModel();
    if (!err.isEmpty()) return err;

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    _cutOffAngleSpectrumBinned.clear();
    if (WaveSet.Enabled)
    {
        if (CutOffAngleSpectrum_deg.empty())
            _cutOffAngleSpectrumBinned = std::vector<double>(WaveSet.countNodes(), CutOffAngle_deg);
        else
            WaveSet.toStandardBins(CutOffAngleSpectrum_deg, _cutOffAngleSpectrumBinned, AWaveResSettings::ExpandWithLastValues);
    }

    if (iModel != -1)
    {
        const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();
        const AGeoObject * obj = std::get<0>(GeoHub.PhotonFunctionals[iModel]);
        if (!obj->Shape) return "Shape is not defined";
        AGeoTube * tube = dynamic_cast<AGeoTube*>(obj->Shape);
        if (tube)
        {
            _radius = tube->rmax;
            if (tube->rmin != 0) return "Photon fiber model cannot accept tube with non-zero inner radius";
        }
        else
        {
            return "Photon fiber model can be assigned only to 'tube' (cylinder) shaped objects";
        }

        const int iMat = obj->Material;
        _material = AMaterialHub::getConstInstance()[iMat];
    }

    return "";
}

QString APFM_OpticalFiber::checkLinkingConsistency(size_t iModelFrom, size_t iModelTo)
{
    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    const AGeoObject * objFrom = std::get<0>(GeoHub.PhotonFunctionals[iModelFrom]);
    const AGeoObject * objTo   = std::get<0>(GeoHub.PhotonFunctionals[iModelTo]);
    if (!objFrom->Shape) return "Shape 'from' is not defined";
    if (!objTo  ->Shape) return "Shape 'to' is not defined";
    const AGeoTube * tubeFrom = dynamic_cast<AGeoTube*>(objFrom->Shape);
    const AGeoTube * tubeTo   = dynamic_cast<AGeoTube*>(objTo  ->Shape);
    if (!tubeFrom || !tubeTo) return "Both linked objects of the photon fiber should have tube shape!";
    if (tubeFrom->rmin != 0 || tubeTo->rmin != 0) return "In/out of the photon fiber cannot have non-zero internal radius";
    if (tubeFrom->rmax != tubeTo->rmax) return "In/out of the photon fiber should have the same radius";

    const int iMatFrom = objFrom->Material;
    const int iMatTo   = objTo  ->Material;
    if (iMatFrom != iMatTo) return "In/out of the photon fiber should have the same material";

    return "";
}

double computeAngleOfIncidence(double R, double x0, double y0, double dx, double dy, double dz, bool & bFail)
{
    bFail = false;
    dz = fabs(dz);
    const double z0 = 0;
    const double L = 1e10;
    const double R2 = R * R;
    const double r02 = x0 * x0 + y0 * y0;

    if (r02 >= R2)
    {
        bFail = true; // outside input radius
        return 0; // STOP
    }

    // Solve |p0 + t d| at XYplane = R^2  =>  (x0 + t dx)^2 + (y0 + t dy)^2 = R^2
    // a t^2 + b t + c = 0
    const double a = dx * dx + dy * dy;
    const double b = 2.0 * (x0 * dx + y0 * dy);
    const double c = r02 - R2;

    if (std::abs(a) < 1e-12)
    {
        // never reaches side wall, assuming its along the axis
        return 90.0; // PASS
    }

    const double disc = b * b - 4.0 * a * c;
    if (disc < 0.0)
    {
        // no real intersection
        bFail = true;
        return 90; // PASS
    }

    const double sqrtDisc = std::sqrt(disc);
    const double t1 = (-b - sqrtDisc) / (2.0 * a);
    const double t2 = (-b + sqrtDisc) / (2.0 * a);

    // We want the positive t (forward direction)
    double t = -1.0;
    if      (t1 > 1e-12) t = t1;
    else if (t2 > 1e-12) t = t2;
    else
    {
        // intersection is behind or at the start
        bFail = true;
        return 0; // STOP
    }

    // Compute hit point
    const double xh = x0 + t * dx;
    const double yh = y0 + t * dy;
    const double zh = z0 + t * dz;

    // Check that hit is within the cylinder length
    if (zh < 0.0 || zh > L)
    {
        // hits side wall outside the physical cylinder
        bFail = true;
        return 90; // PASS
    }

    // Outward normal at hit point (on side wall)
    const double nx = xh / R;
    const double ny = yh / R;
    const double nz = 0.0;

    // Direction magnitude
    const double dMag = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dMag < 1e-12)
    {
        bFail = true;
        return 0; // STOP
    }

    // Cosine of angle between direction and outward normal
    const double dot = dx * nx + dy * ny + dz * nz;
    double cosTheta = std::abs(dot) / dMag;
    if (cosTheta > 1.0) cosTheta = 1.0;
    if (cosTheta < -1.0) cosTheta = -1.0;

    const double theta = std::acos(cosTheta) * 180.0 / 3.1415926535; // in degrees [0, 90]
    return theta;
}

#include "ageoobject.h"
#include "amaterialhub.h"
bool APFM_OpticalFiber::applyModel(APhotonExchangeData & photonData, int index, int /*linkedToIndex*/)
{
    if (photonData.LocalDirection[2] == 0) return false;

    //qDebug() << photonData.LocalPosition[0] << photonData.LocalPosition[1] << photonData.LocalPosition[2] ;
    //qDebug() << photonData.LocalDirection[0] << photonData.LocalDirection[1] << photonData.LocalDirection[2] ;

    // check the insidence angle is within the cut-off
    bool bFail = false;
    double angleIncidence = computeAngleOfIncidence(_radius, photonData.LocalPosition[0], photonData.LocalPosition[1],
                                                    photonData.LocalDirection[0], photonData.LocalDirection[1], photonData.LocalDirection[2],
                                                    bFail);
    //qDebug() << bFail << angleIncidence;
    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    double cutOff;
    if (photonData.WaveIndex == -1 || !WaveSet.Enabled)
        cutOff = CutOffAngle_deg;
    else
        cutOff = _cutOffAngleSpectrumBinned[photonData.WaveIndex];
    if (angleIncidence < cutOff) return false;

    const double tanAngle = sqrt(photonData.LocalDirection[0]*photonData.LocalDirection[0] + photonData.LocalDirection[1]*photonData.LocalDirection[1]) / fabs(photonData.LocalDirection[2]);
    const double inverseCosine = sqrt(1.0 + tanAngle * tanAngle);

    // check absorption
    //const int iMat = obj->Material;
    //const AMaterial * mat = AMaterialHub::getConstInstance()[iMat];
    const double absCoeff = _material->getAbsorptionCoefficient(photonData.WaveIndex); // mm-1
    //const double absCoeff = (photonData.WaveIndex == -1 ? AbsCoeff : _absCoeffSpectrumBinned[photonData.WaveIndex]); // mm-1
    const double photonPath = Length_mm * inverseCosine;
    const double absProb = 1.0 - exp( - absCoeff * photonPath);
    //qDebug() << "abs prob:" << absProb;
    if (ARandomHub::getInstance().uniform() < absProb) return false;

    // teleporting
    //qDebug() << photonData.LocalPosition[2];
    const double sign = photonData.LocalPosition[2] / fabs(photonData.LocalPosition[2]);
    photonData.LocalPosition[2] -= sign * 1e-9; // safity to be inside
    photonData.LocalPosition[2] = - photonData.LocalPosition[2]; // on the other side
    //qDebug() << photonData.LocalPosition[2];

    // time increase
    const double speed = _material->getSpeedOfLight(photonData.WaveIndex); // mm/ns
    const double deltaT = Length_mm * inverseCosine / speed;
    //qDebug() << "t0" << photonData.Time << "speed" << speed << "deltaT" << deltaT;
    photonData.Time += deltaT;

    //qDebug() << "->" << atan(photonData.LocalDirection[2] / sqrt(photonData.LocalDirection[0]*photonData.LocalDirection[0])+photonData.LocalDirection[1]*photonData.LocalDirection[1])*180.0/3.1415926;

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

QString APFM_ThinLens::updateRuntimeProperties(int)
{
    QString err = APFM_ThinLens::checkModel();
    if (!err.isEmpty()) return err;

    _FocalLengthBinned.clear();

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (WaveSet.Enabled)
    {
        if (!FocalLengthSpectrum_mm.empty())
            WaveSet.toStandardBins(FocalLengthSpectrum_mm, _FocalLengthBinned, AWaveResSettings::ExpandWithLastValues);
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

QString APFM_Filter::updateRuntimeProperties(int)
{
    QString err = APFM_Filter::checkModel();
    if (!err.isEmpty()) return err;

    _TransmissionBinned.clear();

    const AWaveResSettings & WaveSet = APhotonSimHub::getInstance().Settings.WaveSet;
    if (WaveSet.Enabled)
    {
        if (!TransmissionSpectrum.empty())
            WaveSet.toStandardBins(TransmissionSpectrum, _TransmissionBinned, AWaveResSettings::ExpandWithZero);
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
