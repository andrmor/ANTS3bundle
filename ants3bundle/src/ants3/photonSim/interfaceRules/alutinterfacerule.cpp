#include "alutinterfacerule.h"
#include "ageomeshhandler.h"
#include "ahistogram.h"
#include "arandomhub.h"
#include "aphoton.h"

#include "TVector3.h"

#include <QDebug>

ALutInterfaceRule::ALutInterfaceRule(int MatFrom, int MatTo) :
    AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Polished;
}
AInterfaceRule::EInterfaceRuleResult ALutInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    double cosTheta = Photon->v[0] * NormalVector[0] + Photon->v[1] * NormalVector[1] + Photon->v[2] * NormalVector[2];
    double theta = std::acos(cosTheta);
    size_t iInciThetaBin = getClosestInboundThetaIndex(theta * 180.0 / 3.1415926535);
    /*
    qDebug() << "ph:" << Photon->v[0] << Photon->v[1] << Photon->v[2];
    qDebug() << "norm:" << NormalVector[0] << NormalVector[1] << NormalVector[2];
    qDebug() << theta * 180.0 / 3.1415926535 << "--> bin" << iInciThetaBin;
    */

    // get process
    const std::array<double,3> & relAbsRefTr = _AbsRefTransVsTheta[iInciThetaBin];
    double sum = relAbsRefTr[0] + relAbsRefTr[1] + relAbsRefTr[2];
    double val = ARandomHub::getInstance().uniform() * sum;

    if (val < relAbsRefTr[0])
    {
        // absorption triggered
        Status = Absorption;
        return Absorbed;
    }

    if (val < relAbsRefTr[0] + relAbsRefTr[1])
    {
        // reflection triggered
        int iTriangle = _HistReflections[iInciThetaBin]->getRandomBin();
        const AGeoMeshHandler::Triangle & tri = _MeshReflections->triangles[iTriangle];

        std::array<double, 3> pointOnSphereInTriangle;
        generateRandomPointInTriangle(_MeshReflections->vertices[tri[0]],
                                      _MeshReflections->vertices[tri[1]],
                                      _MeshReflections->vertices[tri[2]],
                                      pointOnSphereInTriangle);

        TVector3 normalGlobal  (-NormalVector[0], -NormalVector[1], -NormalVector[2]);
        TVector3 photInciGlobal(Photon->v[0],    Photon->v[1],    Photon->v[2]);
        TVector3 photOutLocal  (pointOnSphereInTriangle[0], pointOnSphereInTriangle[1], pointOnSphereInTriangle[2]);
        TVector3 photOutGlobal;
        reflectedLocalToGlobal(normalGlobal, photInciGlobal, photOutLocal, photOutGlobal);

        Photon->v[0] = photOutGlobal[0];
        Photon->v[1] = photOutGlobal[1];
        Photon->v[2] = photOutGlobal[2];

        Status = LobeReflection;
        return Back;
    }

    // else transmission
    int iTriangle = _HistTransmissions[iInciThetaBin]->getRandomBin();
    //qDebug() << "Random triang #" << iTriangle;
    const AGeoMeshHandler::Triangle & tri = _MeshTransmission->triangles[iTriangle];
    //qDebug() << "Triangle vertice indeces:" << tri;
    //qDebug() << "Vertices:" << _MeshTransmission->vertices[tri[0]] << _MeshTransmission->vertices[tri[1]]<< _MeshTransmission->vertices[tri[2]];

    std::array<double, 3> pointOnSphereInTriangle;
    generateRandomPointInTriangle(_MeshTransmission->vertices[tri[0]],
                                  _MeshTransmission->vertices[tri[1]],
                                  _MeshTransmission->vertices[tri[2]],
                                  pointOnSphereInTriangle);
    //qDebug() << "  random point in triang-->" << pointOnSphereInTriangle;

    TVector3 normalGlobal  (-NormalVector[0], -NormalVector[1], -NormalVector[2]);
    TVector3 photInciGlobal(Photon->v[0],    Photon->v[1],    Photon->v[2]);
    TVector3 photOutLocal  (pointOnSphereInTriangle[0], -pointOnSphereInTriangle[1], -pointOnSphereInTriangle[2]); // minus! rotated around X axis

    //qDebug() << "N:" << normalGlobal[0] << normalGlobal[1] << normalGlobal[2];
    //qDebug() << "A:" << photInciGlobal[0] << photInciGlobal[1] << photInciGlobal[2];
    //qDebug() << "B_local:" << photOutLocal[0] << photOutLocal[1] << photOutLocal[2];

    TVector3 photOutGlobal;
    reflectedLocalToGlobal(normalGlobal, photInciGlobal, photOutLocal, photOutGlobal);
    //qDebug() << "result:" << photOutGlobal[0] << photOutGlobal[1] << photOutGlobal[2];

    Photon->v[0] = photOutGlobal[0];
    Photon->v[1] = photOutGlobal[1];
    Photon->v[2] = photOutGlobal[2];
    Status = Transmission;
    return Forward;
}

QString ALutInterfaceRule::getReportLine() const
{
    int size = DataReflection.size();
    if (size == 0) size = DataTransmission.size();
    QString txt = "--> LUT <--\n";
    txt += QString("Angle of inceidence bins: %0").arg(size);
    return txt;
}

QString ALutInterfaceRule::getLongReportLine() const
{
    int size = DataReflection.size();
    if (size == 0) size = DataTransmission.size();
    QString txt = "--> LUT <--\n";
    txt += QString("Angle of inceidence bins: %0").arg(size);
    return txt;
}

QString ALutInterfaceRule::getDescription() const
{
    return ""; // !!!***
}

QString ALutInterfaceRule::loadLUT(const QJsonObject & json)
{
    bool ok = doReadFromJson(json);
    if (!ok) return "Cannot process loaded json";

    // !!!*** check data
    return "";
}

QString ALutInterfaceRule::check()
{
    return doCheckOverrideData();
}

#include "ajsontools.h"
void ALutInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    // abs
    {
        QJsonArray ar;
            jstools::writeDPairVectorToArray(DataAbsorption, ar);
        json["Absorption"] = ar;
    }

    // Reflection
    {
        QJsonArray ar;
            jstools::writeDVectorOfDPairVectorToArray(DataReflection, ar);
        json["Reflection"] = ar;
    }

    // Transmission
    {
        QJsonArray ar;
        jstools::writeDVectorOfDPairVectorToArray(DataTransmission, ar);
        json["Transmission"] = ar;
    }
}

bool ALutInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    DataAbsorption.clear();
    DataReflection.clear();
    DataTransmission.clear();

    // abs
    {
        QJsonArray ar;
        jstools::parseJson(json, "Absorption", ar);
        jstools::readDPairVectorFromArray(ar, DataAbsorption);
    }

    // Reflection
    {
        QJsonArray ar;
        jstools::parseJson(json, "Reflection", ar);
        jstools::readDVectorOfDPairVectorfromArray(ar, DataReflection);
    }

    // Transmission
    {
        QJsonArray ar;
        jstools::parseJson(json, "Transmission", ar);
        jstools::readDVectorOfDPairVectorfromArray(ar, DataTransmission);
    }

    return true;
}

QString ALutInterfaceRule::doCheckOverrideData()
{
    _GloballyNoReflection   = DataReflection.empty();
    _GloballyNoTransmission = DataTransmission.empty();
    _GloballyNoAbsorption   = DataAbsorption.empty();

    _NumberIncidentAngleBins = DataReflection.size();
    if (_NumberIncidentAngleBins == 0)
        _NumberIncidentAngleBins = DataTransmission.size();
    else if (!_GloballyNoTransmission && DataTransmission.size() != _NumberIncidentAngleBins)
        return "LUT interface rule: Mismatch in trans and ref data sizes by angle";
    if (_NumberIncidentAngleBins == 0)
        return "LUT interface rule: both reflection and transmission data cannot be empty";

    if (!_GloballyNoAbsorption && DataAbsorption.size() != _NumberIncidentAngleBins)
        return "LUT interface rule: Mismatch in trans/ref and abs data sizes by angle";

    _AbsRefTransVsTheta.resize(_NumberIncidentAngleBins);
    _HistReflections   = std::vector<AHistogram1D*>(_NumberIncidentAngleBins, nullptr);
    _HistTransmissions = std::vector<AHistogram1D*>(_NumberIncidentAngleBins, nullptr);

    size_t meshSizeReflection = 0;
    size_t meshSizeTransmission = 0;
    for (size_t iInciTheta = 0; iInciTheta < _NumberIncidentAngleBins; iInciTheta++)
    {
        double probAbs = 0;
        if (!_GloballyNoAbsorption) probAbs = DataAbsorption[iInciTheta].second;

        double probRef = 0;
        if (!_GloballyNoReflection)
        {
            if (meshSizeReflection == 0) meshSizeReflection = DataReflection[iInciTheta].second.size();
            else if (meshSizeReflection != DataReflection[iInciTheta].second.size())
                return "Lut interface rule: not consistent mesh data size for reflection data";

            if (meshSizeReflection > 0)
            {
                AHistogram1D * h = new AHistogram1D(meshSizeReflection, 0, meshSizeReflection);
                for (size_t i = 0; i < meshSizeReflection; i++)
                {
                    probRef += DataReflection[iInciTheta].second[i];
                    h->fill(i+0.01, DataReflection[iInciTheta].second[i]);
                }
                h->initRandomGenerator();
                _HistReflections[iInciTheta] = h;
            }
        }

        double probTrans = 0;
        if (!_GloballyNoTransmission)
        {
            if (meshSizeTransmission == 0) meshSizeTransmission = DataTransmission[iInciTheta].second.size();
            else if (meshSizeTransmission != DataTransmission[iInciTheta].second.size())
                return "Lut interface rule: not consistent mesh data size for transmission data";

            if (meshSizeTransmission > 0)
            {
                AHistogram1D * h = new AHistogram1D(meshSizeTransmission, 0, meshSizeTransmission);
                for (size_t i = 0; i < meshSizeTransmission; i++)
                {
                    probTrans += DataTransmission[iInciTheta].second[i];
                    h->fill(i+0.01, DataTransmission[iInciTheta].second[i]);
                }
                h->initRandomGenerator();
                _HistTransmissions[iInciTheta] = h;
            }
        }

        _AbsRefTransVsTheta[iInciTheta] = {probAbs, probRef, probTrans};
    }

    if (!_GloballyNoReflection)
    {
        if (meshSizeReflection == 0)
            return "LUT interface rule: All reflection data arrays are empty";

        _MeshReflections = new AGeoMeshHandler();
        _MeshReflections->buildHemisphereMesh(meshSizeReflection);
    }

    if (!_GloballyNoTransmission)
    {
        if (meshSizeTransmission == 0)
            return "LUT interface rule: All transmission data arrays are empty";

        _MeshTransmission = new AGeoMeshHandler();
        _MeshTransmission->buildHemisphereMesh(meshSizeTransmission);
    }

    std::vector<std::pair<double,std::vector<double>>> * data = nullptr;
    if (!_GloballyNoReflection) data = &DataTransmission;
    else data = &DataReflection; // one of these is required
    _DefinedIncidentThetaValues.resize(_NumberIncidentAngleBins);
    //QString present;
    for (size_t i = 0; i < _NumberIncidentAngleBins; i++)
    {
        _DefinedIncidentThetaValues[i] = data->at(i).first;
        //present += QString("%0 ").arg(DataReflection[i].first);
    }
    //qDebug() << "Present:" << present << _DefinedIncidentThetaValues;

    return "";
}

void ALutInterfaceRule::generateRandomPointInTriangle(const std::array<double, 3> & A, const std::array<double, 3> & B, const std::array<double, 3> & C,
                                                      std::array<double, 3> & result)
{
    /*
    qDebug() << A << B << C;
    double u = ARandomHub::getInstance().uniform();
    double v = ARandomHub::getInstance().uniform();
    double s = std::sqrt(u);

    result[0] = (1.0 - s) * A[0] + s * (1.0 - v) * B[0] + s * v * C[0];
    result[1] = (1.0 - s) * A[1] + s * (1.0 - v) * B[1] + s * v * C[1];
    result[2] = (1.0 - s) * A[2] + s * (1.0 - v) * B[2] + s * v * C[2];
    */

    double u = ARandomHub::getInstance().uniform();
    double v = ARandomHub::getInstance().uniform();

    // Fold points outside the triangle back in (reflect across the diagonal)
    if (u + v > 1.0)
    {
        u = 1.0 - u;
        v = 1.0 - v;
    }

    for (int i = 0; i < 3; ++i)
    {
        double edge1 = B[i] - A[i];
        double edge2 = C[i] - A[i];
        result[i] = A[i] + edge1 * u + edge2 * v;
    }
}

// N photInGlobal  photOutLocal photOutGlobal
void ALutInterfaceRule::reflectedLocalToGlobal(const TVector3 & nHat, const TVector3 & aHat, const TVector3 & B_local, TVector3 & photOutGlobal)
{
    // In-plane (tangential) component of A relative to the normal
    TVector3 t = aHat - nHat * aHat.Dot(nHat);
    double tNorm = t.Mag();

    if (tNorm < 1e-9)
    {
        // A is (anti)parallel to N -> normal incidence, x_local undefined.
        // Pick an arbitrary tangent direction instead.
        TVector3 arbitrary = (std::fabs(nHat[0]) < 0.9) ? TVector3{1.0, 0, 0} : TVector3{0, 1.0, 0};
        t = arbitrary - nHat * arbitrary.Dot(nHat);
        tNorm = t.Mag();
    }

    TVector3 xLocal = t * (1.0 / tNorm);
    TVector3 zLocal = nHat;
    TVector3 yLocal = zLocal.Cross(xLocal);

    // B_local is arbitrary (not tied to A) -> full 3-component transform
    photOutGlobal = xLocal * B_local[0] + yLocal * B_local[1] + zLocal * B_local[2];
}

size_t ALutInterfaceRule::getClosestInboundThetaIndex(double theta_deg)
{
    auto it = std::lower_bound(_DefinedIncidentThetaValues.begin(), _DefinedIncidentThetaValues.end(), theta_deg);

    // D is <= all elements, so the first element is closest
    if (it == _DefinedIncidentThetaValues.begin()) return 0;

    // D is > all elements, so the last element is closest
    if (it == _DefinedIncidentThetaValues.end()) return _DefinedIncidentThetaValues.size() - 1;

    // D is between v[i-1] and v[i]; compare which is closer
    double after  = *it;
    double before = *(it - 1);

    return (after - theta_deg < theta_deg - before)
               ? static_cast<size_t>(it - _DefinedIncidentThetaValues.begin())
               : static_cast<size_t>(it - _DefinedIncidentThetaValues.begin() - 1);
}
