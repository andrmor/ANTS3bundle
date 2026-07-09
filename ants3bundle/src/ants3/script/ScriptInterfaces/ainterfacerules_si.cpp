#include "ainterfacerules_si.h"
#include "ainterfacerulehub.h"
#include "alutinterfacerule.h"
#include "alutsurfacedata.h"
#include "alutsurfacegenerator.h"
#include "amaterialhub.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ascripthub.h"
#include "ajsontools.h"

#include <QFileInfo>
#include <QJsonObject>

AInterfaceRules_SI::AInterfaceRules_SI() :
    RuleHub(AInterfaceRuleHub::getInstance())
{
    Description = "Optical interface rules: generation of surface LUTs (\"DavisLUT\" rule) and rule assignment";

    Help["generateSurfaceLut"] = "Generates a surface LUT for the 'DavisLUT' interface rule by ray tracing over a 3D surface\n"
                                 "topography Z(x,y) (e.g. measured with AFM), see Roncali & Cherry, Phys.Med.Biol. 58 (2013) 2185.\n"
                                 "Arguments: heightmap file name, output LUT file name, object with parameters.\n"
                                 "Parameter keys (* = mandatory):\n"
                                 "  n1*: refractive index of the medium the photons arrive from (e.g. the crystal)\n"
                                 "  n2*: refractive index of the medium behind the surface\n"
                                 "  format: 'matrix' (default) = text file with a matrix of heights (rows <-> y, columns <-> x),\n"
                                 "          requires pixelSizeX and pixelSizeY; 'xyz' = text file with 3 columns (x y z) on a regular grid\n"
                                 "  pixelSizeX, pixelSizeY: pixel size for the 'matrix' format (same length units as the heights)\n"
                                 "  wavelength: [nm] recorded in the LUT metadata (default 420)\n"
                                 "  thetaBins (default 40), thetaOutBins (45), phiOutBins (36): LUT binning\n"
                                 "  photonsPerBin: photons per incidence angle bin (default 80000)\n"
                                 "  phiSteps: 0 (default) = random incidence azimuth, >0 = discrete azimuth steps\n"
                                 "  maxBounces (default 100), seed (0 = do not reseed), comment\n"
                                 "  reverseGeometry: true if the photons arrive from the medium ABOVE the heightmap surface (default false)\n"
                                 "  alsoReverse: true = also generate the LUT for the reverse direction (swapped n1/n2) and save it\n"
                                 "               to a file with '_reverse' appended to the name\n"
                                 "Returns an object with generation statistics.\n"
                                 "Note: the LUT is direction-specific, assign it only for the n1->n2 material pair!";
    Help["getLutInfo"]         = "Returns the metadata and binning info of the given surface LUT file as an object";
    Help["setLutMaterialRule"] = "Creates a 'DavisLUT' interface rule with the LUT loaded from the given file\n"
                                 "and sets it for the material pair (from, to). Materials are given by name.\n"
                                 "The LUT data are embedded in the config, the file is not needed afterwards.";
    Help["setLutVolumeRule"]   = "Creates a 'DavisLUT' interface rule with the LUT loaded from the given file\n"
                                 "and sets it for the volume pair (from, to). Volumes are given by name.";
    Help["clearMaterialRule"]  = "Removes the interface rule (of any type) defined for the material pair (from, to)";
    Help["clearVolumeRule"]    = "Removes the interface rule (of any type) defined for the volume pair (from, to)";
}

QVariantMap AInterfaceRules_SI::generateSurfaceLut(QString heightmapFile, QString outputLutFile, QVariantMap params)
{
    QVariantMap result;

    if (!params.contains("n1") || !params.contains("n2"))
    {
        abort("generateSurfaceLut: parameters 'n1' and 'n2' are mandatory");
        return result;
    }

    ALutSurfaceGenerator generator;
    generator.n1 = params["n1"].toDouble();
    generator.n2 = params["n2"].toDouble();
    if (params.contains("wavelength"))    generator.wavelength         = params["wavelength"].toDouble();
    if (params.contains("thetaBins"))     generator.thetaIncBins       = params["thetaBins"].toInt();
    if (params.contains("thetaOutBins"))  generator.thetaOutBins       = params["thetaOutBins"].toInt();
    if (params.contains("phiOutBins"))    generator.phiOutBins         = params["phiOutBins"].toInt();
    if (params.contains("photonsPerBin")) generator.photonsPerThetaBin = params["photonsPerBin"].toInt();
    if (params.contains("phiSteps"))      generator.phiSteps           = params["phiSteps"].toInt();
    if (params.contains("maxBounces"))    generator.maxBounces         = params["maxBounces"].toInt();
    if (params.contains("seed"))          generator.seed               = params["seed"].toInt();
    if (params.contains("comment"))       generator.comment            = params["comment"].toString();
    if (params.contains("reverseGeometry")) generator.reverseGeometry  = params["reverseGeometry"].toBool();

    const QString format = (params.contains("format") ? params["format"].toString() : "matrix");
    QString err;
    if (format == "matrix")
    {
        const double pixelSizeX = params["pixelSizeX"].toDouble();
        const double pixelSizeY = params["pixelSizeY"].toDouble();
        if (pixelSizeX <= 0 || pixelSizeY <= 0)
        {
            abort("generateSurfaceLut: 'matrix' format requires positive 'pixelSizeX' and 'pixelSizeY' parameters");
            return result;
        }
        err = generator.loadHeightmapMatrix(heightmapFile, pixelSizeX, pixelSizeY);
    }
    else if (format == "xyz") err = generator.loadHeightmapXYZ(heightmapFile);
    else
    {
        abort("generateSurfaceLut: 'format' should be 'matrix' or 'xyz'");
        return result;
    }
    if (!err.isEmpty())
    {
        abort("generateSurfaceLut: " + err);
        return result;
    }

    generator.progressCallback = [this](int percent) -> bool
    {
        AScriptHub::getInstance().reportProgress(percent, Lang);
        return !AScriptHub::isAborted(Lang);
    };

    auto runAndSave = [&generator, this](const QString & fileName) -> QVariantMap
    {
        QVariantMap summary;
        ALutSurfaceData data;
        QString err = generator.generate(data);
        if (!err.isEmpty())
        {
            abort("generateSurfaceLut: " + err);
            return summary;
        }
        QJsonObject json;
        data.writeToJson(json);
        if (!jstools::saveJsonToFile(json, fileName))
        {
            abort("generateSurfaceLut: cannot save LUT to file " + fileName);
            return summary;
        }
        summary["lutFile"]     = fileName;
        summary["n1"]          = generator.n1;
        summary["n2"]          = generator.n2;
        summary["meanBounces"] = generator.meanBounces();
        summary["anomalies"]   = (qlonglong)generator.anomalies();
        summary["wraps"]       = (qlonglong)generator.wraps();
        summary["gridSizeX"]   = data.GridSizeX;
        summary["gridSizeY"]   = data.GridSizeY;
        return summary;
    };

    result = runAndSave(outputLutFile);
    if (result.isEmpty()) return result;   // aborted

    if (params.contains("alsoReverse") && params["alsoReverse"].toBool())
    {
        std::swap(generator.n1, generator.n2);
        generator.reverseGeometry = !generator.reverseGeometry;

        const QFileInfo fileInfo(outputLutFile);
        const QString suffix = fileInfo.suffix();
        QString reverseName = fileInfo.path() + "/" + fileInfo.completeBaseName() + "_reverse";
        if (!suffix.isEmpty()) reverseName += "." + suffix;

        const QVariantMap reverseSummary = runAndSave(reverseName);
        if (reverseSummary.isEmpty()) return result;
        result["reverse"] = reverseSummary;
    }

    return result;
}

QVariantMap AInterfaceRules_SI::getLutInfo(QString lutFile)
{
    QVariantMap result;

    QJsonObject json;
    if (!jstools::loadJsonFromFile(json, lutFile))
    {
        abort("Cannot open or parse LUT file " + lutFile);
        return result;
    }
    ALutSurfaceData data;
    const QString err = data.readFromJson(json);
    if (!err.isEmpty())
    {
        abort(err);
        return result;
    }

    result["n1"]              = data.n1;
    result["n2"]              = data.n2;
    result["wavelength"]      = data.Wavelength;
    result["sourceHeightmap"] = data.SourceHeightmap;
    result["gridSizeX"]       = data.GridSizeX;
    result["gridSizeY"]       = data.GridSizeY;
    result["pixelSizeX"]      = data.PixelSizeX;
    result["pixelSizeY"]      = data.PixelSizeY;
    result["thetaIncBins"]    = data.ThetaIncBins;
    result["thetaOutBins"]    = data.ThetaOutBins;
    result["phiOutBins"]      = data.PhiOutBins;
    result["photonsPerBin"]   = data.PhotonsPerThetaBin;
    result["meanBounces"]     = data.MeanBounces;
    result["anomalyCount"]    = data.AnomalyCount;
    result["generationDate"]  = data.GenerationDate;
    result["comment"]         = data.Comment;
    return result;
}

ALutInterfaceRule * AInterfaceRules_SI::makeLutRule(int matFrom, int matTo, const QString & lutFile)
{
    ALutInterfaceRule * rule = new ALutInterfaceRule(matFrom, matTo);

    QString err = rule->loadLUT(lutFile);
    if (err.isEmpty()) err = rule->checkOverrideData();
    if (!err.isEmpty())
    {
        delete rule;
        abort(err);
        return nullptr;
    }

    const QString warning = rule->getMaterialConsistencyWarning();
    if (!warning.isEmpty())
        AScriptHub::getInstance().outputText("WARNING - " + warning, Lang);

    return rule;
}

void AInterfaceRules_SI::setLutMaterialRule(QString matFrom, QString matTo, QString lutFile)
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    const int iFrom = MatHub.findMaterial(matFrom);
    if (iFrom == -1)
    {
        abort("Material not found: " + matFrom);
        return;
    }
    const int iTo = MatHub.findMaterial(matTo);
    if (iTo == -1)
    {
        abort("Material not found: " + matTo);
        return;
    }
    if (iFrom == iTo)
    {
        abort("Materials 'from' and 'to' should be different");
        return;
    }

    ALutInterfaceRule * rule = makeLutRule(iFrom, iTo, lutFile);
    if (!rule) return;

    RuleHub.setMaterialRule(iFrom, iTo, rule);
    RuleHub.announceRulesChanged();
}

void AInterfaceRules_SI::setLutVolumeRule(QString volFrom, QString volTo, QString lutFile)
{
    // material indices are needed for the consistency check; taken from the current geometry
    // if the volumes exist (the rule itself is applied by volume names at simulation time)
    int iFrom = 0;
    int iTo   = 0;
    AGeoObject * world = AGeometryHub::getConstInstance().World;
    const AGeoObject * objFrom = world->findObjectByName(volFrom);
    if (objFrom) iFrom = objFrom->Material;
    const AGeoObject * objTo = world->findObjectByName(volTo);
    if (objTo) iTo = objTo->Material;

    ALutInterfaceRule * rule = makeLutRule(iFrom, iTo, lutFile);
    if (!rule) return;

    RuleHub.setVolumeRule(TString(volFrom.toLatin1().data()), TString(volTo.toLatin1().data()), rule);
    RuleHub.announceRulesChanged();
}

void AInterfaceRules_SI::clearMaterialRule(QString matFrom, QString matTo)
{
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    const int iFrom = MatHub.findMaterial(matFrom);
    if (iFrom == -1)
    {
        abort("Material not found: " + matFrom);
        return;
    }
    const int iTo = MatHub.findMaterial(matTo);
    if (iTo == -1)
    {
        abort("Material not found: " + matTo);
        return;
    }

    RuleHub.setMaterialRule(iFrom, iTo, nullptr);
    RuleHub.announceRulesChanged();
}

void AInterfaceRules_SI::clearVolumeRule(QString volFrom, QString volTo)
{
    RuleHub.removeVolumeRule(TString(volFrom.toLatin1().data()), TString(volTo.toLatin1().data()));
    RuleHub.announceRulesChanged();
}
