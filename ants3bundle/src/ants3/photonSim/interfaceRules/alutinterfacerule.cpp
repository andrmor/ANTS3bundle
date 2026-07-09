#include "alutinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QDebug>

#include <cmath>

ALutInterfaceRule::ALutInterfaceRule(int MatFrom, int MatTo) :
    AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Polished;   // the LUT itself represents the rough surface
}

AInterfaceRule::EInterfaceRuleResult ALutInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    // NormalVector points along the photon propagation direction (see APhotonTracer::tryInterfaceRule)
    double cosTi = Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2];
    if      (cosTi > 1.0) cosTi = 1.0;
    else if (cosTi < 0)   cosTi = 0;
    const double thetaInc = acos(cosTi) * 180.0 / M_PI;

    const int iBin = Data.selectThetaBin(thetaInc, RandomHub.uniform());

    bool reflected;
    const double rnd = RandomHub.uniform();
    const double probReflection = Data.getReflectionProbability(iBin);
    if      (rnd < probReflection)                                        reflected = true;
    else if (rnd < probReflection + Data.getTransmissionProbability(iBin)) reflected = false;
    else
    {
        Status = Absorption;
        return Absorbed;
    }

    double thetaOut, phiOut; // [deg]
    const bool ok = Data.sampleOutgoing(reflected, iBin,
                                        RandomHub.uniform(), RandomHub.uniform(), RandomHub.uniform(),
                                        thetaOut, phiOut);
    if (!ok)
    {
        // not possible for a valid LUT: non-zero probability implies non-empty distribution
        Status = Error;
        return _Error_;
    }

    // Local frame: ex is the tangential component of the photon direction (defines the incidence
    // plane, phiOut = 0 is the specular azimuth), ey = N x ex.
    // The same frame convention is used when the LUT is generated (see ALutSurfaceGenerator).
    double ex[3];
    const double sinTi = sqrt(1.0 - cosTi*cosTi);
    if (sinTi < 1e-6)
    {
        // normal incidence: the incidence plane is not defined, the LUT distribution is
        // azimuthally symmetric here, so any unit vector perpendicular to the normal can be used
        double a[3] = {0, 0, 0};
        if      (fabs(NormalVector[0]) <= fabs(NormalVector[1]) && fabs(NormalVector[0]) <= fabs(NormalVector[2])) a[0] = 1.0;
        else if (fabs(NormalVector[1]) <= fabs(NormalVector[2]))                                                   a[1] = 1.0;
        else                                                                                                       a[2] = 1.0;
        ex[0] = a[1]*NormalVector[2] - a[2]*NormalVector[1];
        ex[1] = a[2]*NormalVector[0] - a[0]*NormalVector[2];
        ex[2] = a[0]*NormalVector[1] - a[1]*NormalVector[0];
        const double norm = sqrt(ex[0]*ex[0] + ex[1]*ex[1] + ex[2]*ex[2]);
        for (int i = 0; i < 3; i++) ex[i] /= norm;
    }
    else
    {
        for (int i = 0; i < 3; i++) ex[i] = (Photon->v[i] - cosTi*NormalVector[i]) / sinTi;
    }

    double ey[3];
    ey[0] = NormalVector[1]*ex[2] - NormalVector[2]*ex[1];
    ey[1] = NormalVector[2]*ex[0] - NormalVector[0]*ex[2];
    ey[2] = NormalVector[0]*ex[1] - NormalVector[1]*ex[0];

    const double thetaRad = thetaOut * M_PI / 180.0;
    const double phiRad   = phiOut   * M_PI / 180.0;
    const double sinT = sin(thetaRad);
    const double tx = sinT * cos(phiRad);
    const double ty = sinT * sin(phiRad);
    const double tn = cos(thetaRad) * (reflected ? -1.0 : 1.0);

    for (int i = 0; i < 3; i++)
        Photon->v[i] = tx*ex[i] + ty*ey[i] + tn*NormalVector[i];

    // renormalize: the tracer takes this direction as-is (no rough-surface re-run),
    // and even a floating-point overshoot of |v| slightly above 1 makes a downstream
    // acos(normal . v) at a near-normal sensor hit return NaN
    const double norm = sqrt(Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2]);
    if (norm > 0)
        for (int i = 0; i < 3; i++) Photon->v[i] /= norm;

    if (reflected)
    {
        Status = LobeReflection;
        return Back;
    }
    else
    {
        Status = Transmission;
        return Forward;
    }
}

void ALutInterfaceRule::initializeWaveResolved()
{
    // safety net: normally the runtime data are already prepared by doCheckOverrideData()
    if (Data.isLoaded() && !Data.isRuntimeReady())
    {
        const QString err = Data.buildRuntime();
        if (!err.isEmpty()) qWarning() << "DavisLUT rule:" << err;
    }
}

QString ALutInterfaceRule::loadLUT(const QString & fileName)
{
    QJsonObject json;
    if (!jstools::loadJsonFromFile(json, fileName))
        return "Cannot open or parse LUT file " + fileName;

    const QString err = Data.readFromJson(json);
    if (!err.isEmpty())
    {
        Data.clear();
        return err;
    }

    LutFileName = fileName;
    return "";
}

QString ALutInterfaceRule::getMaterialConsistencyWarning() const
{
    if (!Data.isLoaded()) return "";

    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    if (MatFrom < 0 || MatFrom >= MatHub.countMaterials()) return "";
    if (MatTo   < 0 || MatTo   >= MatHub.countMaterials()) return "";

    auto refIndexAtLutWavelength = [this, &MatHub](int iMat)
    {
        const AMaterial * mat = MatHub[iMat];
        if (!mat->RefIndex_Wave.empty())
            return AWaveResSettings::getInterpolatedValue(Data.Wavelength, mat->RefIndex_Wave);
        return mat->RefIndex;
    };

    QString warn;
    const double nFrom = refIndexAtLutWavelength(MatFrom);
    if (fabs(nFrom - Data.n1) > 0.01 * Data.n1)
        warn += QString("LUT was generated for n1 = %1, but the 'from' material has refractive index %2 at %3 nm\n")
                .arg(Data.n1).arg(nFrom).arg(Data.Wavelength);
    const double nTo = refIndexAtLutWavelength(MatTo);
    if (fabs(nTo - Data.n2) > 0.01 * std::max(Data.n2, 1e-10))
        warn += QString("LUT was generated for n2 = %1, but the 'to' material has refractive index %2 at %3 nm\n")
                .arg(Data.n2).arg(nTo).arg(Data.Wavelength);

    return warn;
}

QString ALutInterfaceRule::getReportLine() const
{
    if (!Data.isLoaded()) return "LUT not loaded";
    return QString("n %1->%2, %3 angle bins").arg(Data.n1).arg(Data.n2).arg(Data.ThetaIncBins);
}

QString ALutInterfaceRule::getLongReportLine() const
{
    QString s = "--> Davis LUT <--\n";
    if (!Data.isLoaded()) return s + "LUT not loaded!";
    s += QString("LUT for n1 = %1 -> n2 = %2 at %3 nm\n").arg(Data.n1).arg(Data.n2).arg(Data.Wavelength);
    s += QString("Binning: %1 incidence angle bins, out %2 theta x %3 phi\n").arg(Data.ThetaIncBins).arg(Data.ThetaOutBins).arg(Data.PhiOutBins);
    if (!Data.SourceHeightmap.isEmpty()) s += QString("Source heightmap: %1\n").arg(Data.SourceHeightmap);
    const QString warn = getMaterialConsistencyWarning();
    if (!warn.isEmpty()) s += "WARNING:\n" + warn;
    return s;
}

QString ALutInterfaceRule::getDescription() const
{
    return "Look-up-table based model of light interaction with a rough optical surface\n"
           "(E.Roncali and S.R.Cherry, Phys.Med.Biol. 58 (2013) 2185).\n"
           "The LUT is generated offline by ray tracing over a 3D surface topography\n"
           "(e.g. measured with AFM): use the 'rules' script unit, generateSurfaceLut() method.\n"
           "For each incidence angle the LUT provides the probabilities of reflection and\n"
           "transmission and the angular distributions of the outgoing photons.\n"
           "Multiple micro-reflections, shadowing and masking are included in the LUT itself,\n"
           "so this rule does not use the 'rough surface' add-on.\n"
           "\n"
           "Note that the LUT is generated for fixed refractive indices n1 (before the\n"
           "interface) and n2 (after) at a fixed wavelength: the rule applies the same LUT\n"
           "to photons of all wavelengths.\n"
           "The LUT is valid only for the n1->n2 direction of travel, so the 'Symmetric'\n"
           "option is not available: to define the rule for the reverse direction,\n"
           "generate a LUT with swapped refractive indices.";
}

void ALutInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    json["LutFile"] = LutFileName;
    if (Data.isLoaded())
    {
        QJsonObject jsLut;
        Data.writeToJson(jsLut);
        json["LUT"] = jsLut;
    }
}

bool ALutInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "LutFile", LutFileName);

    QJsonObject jsLut;
    if (!jstools::parseJson(json, "LUT", jsLut))
    {
        Data.clear();
        return true;   // rule without loaded LUT is a valid (not yet configured) state
    }

    const QString err = Data.readFromJson(jsLut);
    if (!err.isEmpty())
    {
        qWarning() << "DavisLUT rule:" << err;
        Data.clear();
        return false;
    }
    return true;
}

QString ALutInterfaceRule::doCheckOverrideData()
{
    if (Symmetric)
        return "DavisLUT rule is direction-specific (n1->n2 is baked into the LUT):\n"
               "'Symmetric' option is not supported, generate a LUT with swapped refractive\n"
               "indices and assign it to the reverse direction explicitly";

    if (!Data.isLoaded()) return "Surface LUT is not loaded";

    const QString err = Data.buildRuntime();
    if (!err.isEmpty()) return err;

    const QString warn = getMaterialConsistencyWarning();
    if (!warn.isEmpty()) qWarning() << "DavisLUT rule:" << warn;

    return "";
}
