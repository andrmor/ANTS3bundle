#include "amonitorconfig.h"
#include "ajsontools.h"
#include "ageoconsts.h"

#include <QJsonObject>
#include <QDebug>

void AMonitorConfig::writeToJson(QJsonObject &json) const
{
   json["shape"] = shape;
   json["size1"] = size1;
   json["size2"] = size2;
   json["dz"] = dz;

   if (!str2size1.isEmpty()) json["str2size1"] = str2size1;
   if (!str2size2.isEmpty()) json["str2size2"] = str2size2;

   json["PhotonOrParticle"] = PhotonOrParticle;
   json["bUpper"] = bUpper;
   json["bLower"] = bLower;
   json["bStopTracking"] = bStopTracking;

   json["Particle"]   = Particle;
   json["bPrimary"]   = bPrimary;
   json["bSecondary"] = bSecondary;
   json["bDirect"]    = bDirect;
   json["bIndirect"]  = bIndirect;

   json["xbins"] = xbins;
   json["ybins"] = ybins;

   json["timeBins"] = timeBins;
   json["timeFrom"] = timeFrom;
   json["timeTo"] = timeTo;

   json["angleBins"] = angleBins;
   json["angleFrom"] = angleFrom;
   json["angleTo"] = angleTo;

   json["waveBins"] = waveBins;
   json["waveFrom"] = waveFrom;
   json["waveTo"] = waveTo;

   json["energyBins"] = energyBins;
   json["energyFrom"] = energyFrom;
   json["energyTo"] = energyTo;
   json["energyUnitsInHist"] = energyUnitsInHist;
}

void AMonitorConfig::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "shape", shape);
    jstools::parseJson(json, "size1", size1);
    jstools::parseJson(json, "size2", size2);
    jstools::parseJson(json, "dz", dz);

    str2size1.clear();
    jstools::parseJson(json, "str2size1", str2size1);
    str2size2.clear();
    jstools::parseJson(json, "str2size2", str2size2);

    jstools::parseJson(json, "PhotonOrParticle", PhotonOrParticle);
    jstools::parseJson(json, "bUpper", bUpper);
    jstools::parseJson(json, "bLower", bLower);
    jstools::parseJson(json, "bStopTracking", bStopTracking);

    jstools::parseJson(json, "Particle",   Particle);
    jstools::parseJson(json, "bPrimary",   bPrimary);
    jstools::parseJson(json, "bSecondary", bSecondary);
    jstools::parseJson(json, "bDirect",    bDirect);
    jstools::parseJson(json, "bIndirect",  bIndirect);

    jstools::parseJson(json, "xbins", xbins);
    jstools::parseJson(json, "ybins", ybins);

    jstools::parseJson(json, "timeBins", timeBins);
    jstools::parseJson(json, "timeFrom", timeFrom);
    jstools::parseJson(json, "timeTo", timeTo);

    jstools::parseJson(json, "angleBins", angleBins);
    jstools::parseJson(json, "angleFrom", angleFrom);
    jstools::parseJson(json, "angleTo", angleTo);

    jstools::parseJson(json, "waveBins", waveBins);
    jstools::parseJson(json, "waveFrom", waveFrom);
    jstools::parseJson(json, "waveTo", waveTo);

    jstools::parseJson(json, "energyBins", energyBins);
    jstools::parseJson(json, "energyFrom", energyFrom);
    jstools::parseJson(json, "energyTo", energyTo);
    jstools::parseJson(json, "energyUnitsInHist", energyUnitsInHist);
}

QString AMonitorConfig::updateFromGeoConstants()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2size1, size1); if (!ok) return errorStr;
    ok = GC.updateDoubleParameter(errorStr, str2size2, size2); if (!ok) return errorStr;
    return "";
}
