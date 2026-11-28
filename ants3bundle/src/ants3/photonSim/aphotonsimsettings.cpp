#include "aphotonsimsettings.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDebug>

#include <cmath>

void AWaveResSettings::writeToJson(QJsonObject & json) const
{
    json["Enabled"] = Enabled;

    json["From"]    = From;
    json["To"]      = To;
    json["Step"]    = Step;
}

void AWaveResSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Enabled", Enabled);

    jstools::parseJson(json, "From",    From);
    jstools::parseJson(json, "To",      To);
    jstools::parseJson(json, "Step",    Step);
}

void AWaveResSettings::clear()
{
    Enabled = false;
    From = 200.0;
    To   = 800.0;
    Step = 5.0;
}

int AWaveResSettings::countNodes() const
{
    if (Step == 0) return 1;
    return (To - From) / Step + 1;
}

double AWaveResSettings::toWavelength(int index) const
{
    return From + Step * index;
}

void AWaveResSettings::toWavelength(std::vector<std::pair<double, double>> & waveIndex_I_pairs) const
{
    for (auto & pair : waveIndex_I_pairs)
        pair.first = toWavelength(pair.first);
}

int AWaveResSettings::toIndex(double wavelength) const
{
    if (!Enabled) return -1;

    if (wavelength < From) return -1;
    if (wavelength > To)   return -1;

    int iwave = round( (wavelength - From) / Step );

    if (iwave < 0) return -1;

    const int numNodes = countNodes();
    if (iwave >= numNodes) return -1;

    return iwave;
}

int AWaveResSettings::toIndexFast(double wavelength) const
{
    return round( (wavelength - From) / Step );
}

void AWaveResSettings::toStandardBins(const std::vector<double> & wavelength, const std::vector<double> & value, std::vector<double> & binnedValue, EConverterOptions options) const
{
    binnedValue.clear();

    double wave, binned;
    const int points = countNodes();
    for (int iP = 0; iP < points; iP++)
    {
        wave = From + Step * iP;
        if (wave < wavelength.front())
        {
            if (options == ExpandWithZero) binned = 0;
            else                           binned = value.front();
        }
        else
        {
            if (wave > wavelength.back())
            {
                if (options == ExpandWithZero) binned = 0;
                else                           binned = value.back();
            }
            else binned = getInterpolatedValue(wave, wavelength, value);
        }
        binnedValue.push_back(binned);
    }
}

void AWaveResSettings::toStandardBins(const std::vector<std::pair<double,double>> & waveAndData, std::vector<double> & binnedValue, EConverterOptions options) const
{
    std::vector<double> wavVec, valVec;

    for (const auto & pair : waveAndData)
    {
        wavVec.push_back(pair.first);
        valVec.push_back(pair.second);
    }

    toStandardBins(wavVec, valVec, binnedValue, options);
}

void AWaveResSettings::toStandardBins(const std::vector<std::pair<double, std::complex<double>>> & waveReIm, std::vector<std::complex<double>> & reIm, EConverterOptions options) const
{
    std::vector<double> wavelength, realValue, imagValue, realBinned, imagBinned;

    for (const auto & rec : waveReIm)
    {
        wavelength.push_back(rec.first);
        realValue.push_back(rec.second.real());
        imagValue.push_back(rec.second.imag());
    }

    toStandardBins(wavelength, realValue, realBinned, options);
    toStandardBins(wavelength, imagValue, imagBinned, options);

    reIm.clear();
    for (size_t i = 0; i < realBinned.size(); i++)
        reIm.push_back( {realBinned[i], imagBinned[i]} );
}

void AWaveResSettings::getWavelengthBins(std::vector<double> & wavelength) const
{
    wavelength.clear();
    const int points = countNodes();
    for (int iP = 0; iP < points; iP++)
        wavelength.push_back(From + Step * iP);
}

std::vector<double> AWaveResSettings::getVectorOfIndexes() const
{
    const int nodes = countNodes();
    std::vector<double> indexes(nodes);
    for (int i = 0; i < nodes; i++) indexes[i] = i;
    return indexes;
}

double AWaveResSettings::getInterpolatedValue(double val, const std::vector<double> & X, const std::vector<double> & F)
{
    if (X.size() == 0) return 0;
    if (X.size() == 1) return F.front();

    if (val <= X.front()) return F.front();
    if (val >= X.back())  return F.back();

    std::vector<double>::const_iterator it;
    it = std::lower_bound(X.begin(), X.end(), val);
    int index = it - X.begin();
    if (index < 1) return F.front();

    const double & F_Less = F[index-1];
    const double & F_More = F[index];
    const double & X_Less = X[index-1];
    const double & X_More = X[index];

    if (X_Less == X_More) return F_More;
    return F_Less + (F_More - F_Less) * (val - X_Less) / (X_More - X_Less);
}

double AWaveResSettings::getInterpolatedValue(double val, const std::vector<std::pair<double, double>> & F)
{
    if (F.size() == 0) return 0;
    if (F.size() == 1) return F.front().second;

    if (val <= F.front().first) return F.front().second;
    if (val >= F.back().first)  return F.back().second;

    std::vector<std::pair<double, double>>::const_iterator it;
    it = std::lower_bound(F.begin(), F.end(), std::pair<double,double>{val,0},
                          [](const std::pair<double, double> & lh, const std::pair<double, double> & rh)
                          {
                            return (lh.first < rh.first);
                          });
    int index = it - F.begin();
    if (index < 1) return F.front().second;

    const double & F_Less = F[index-1].second;
    const double & F_More = F[index].second;
    const double & X_Less = F[index-1].first;
    const double & X_More = F[index].first;

    if (X_Less == X_More) return F_More;
    return F_Less + (F_More - F_Less) * (val - X_Less) / (X_More - X_Less);
}

// ---

void APhotonsPerBombSettings::clearSettings()
{
    Mode        = Constant;
    FixedNumber = 10;
    UniformMin  = 10;
    UniformMax  = 12;
    NormalMean  = 100.0;
    NormalSigma = 10.0;
    CustomDist.clear();
}

void APhotonsPerBombSettings::writeToJson(QJsonObject & json) const
{
    QString str;
    switch (Mode)
    {
    case EBombPhNumber::Constant : str = "constant"; break;
    case EBombPhNumber::Poisson  : str = "poisson";  break;
    case EBombPhNumber::Uniform  : str = "uniform";  break;
    case EBombPhNumber::Normal   : str = "normal";   break;
    case EBombPhNumber::Custom   : str = "custom";   break;
    }
    json["Mode"] = str;

    json["FixedNumber"] = FixedNumber;
    json["UniformMin"]  = UniformMin;
    json["UniformMax"]  = UniformMax;
    json["NormalMean"]  = NormalMean;
    json["NormalSigma"] = NormalSigma;
    json["PoissonMean"] = PoissonMean;

    QJsonArray ar;
        for (const auto & pair : CustomDist)
        {
            QJsonArray el;
                el << pair.first << pair.second;
            ar.push_back(el);
        }
    json["CustomDist"] = ar;
}

QString APhotonsPerBombSettings::readFromJson(const QJsonObject & json)
{
    clearSettings();

    QString str = "undefined";
    jstools::parseJson(json, "Mode", str);

    if      (str == "constant") Mode = EBombPhNumber::Constant;
    else if (str == "poisson")  Mode = EBombPhNumber::Poisson;
    else if (str == "uniform")  Mode = EBombPhNumber::Uniform;
    else if (str == "normal")   Mode = EBombPhNumber::Normal;
    else if (str == "custom")   Mode = EBombPhNumber::Custom;
    else return QString("Unknown photons per bomb mode: " + str);

    jstools::parseJson(json, "FixedNumber", FixedNumber);
    jstools::parseJson(json, "UniformMin",  UniformMin);
    jstools::parseJson(json, "UniformMax",  UniformMax);
    jstools::parseJson(json, "NormalMean",  NormalMean);
    jstools::parseJson(json, "NormalSigma", NormalSigma);
    jstools::parseJson(json, "PoissonMean", PoissonMean);

    QJsonArray ar;
    jstools::parseJson(json, "CustomDist", ar);
    CustomDist.reserve(ar.size());
    for (int i = 0; i < ar.size(); i++)
    {
        QJsonArray el = ar[i].toArray();
        int x = el[0].toInt();
        double y = el[1].toDouble();
        CustomDist.push_back({x, y});
    }

    return "";
}

// ---

void APhotOptSettings::writeToJson(QJsonObject &json) const
{
    json["MaxPhotonTransitions"]  = MaxPhotonTransitions;

    QString str;
    switch (TracingMode)
    {
        case Normal        : str = "Normal";  break;
        case CheckQeBefore : str = "CheckQE"; break;
        case LRF           : str = "LRF";     break;
    }
    json["TracingMode"] = str;

    // LRF-based signal generation
    {
        QJsonObject js;
            js["PhotonsPerNode"] = LRF_photonsPerNode;
            js["PhotoElectrons"] = LRF_photoElectrons;
        json["LRF"] = js;
    }
}

void APhotOptSettings::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "MaxPhotonTransitions",  MaxPhotonTransitions);

    QString str;
    bool ok = jstools::parseJson(json, "TracingMode", str);
    if (ok)
    {
        if      (str == "Normal")  TracingMode = Normal;
        else if (str == "CheckQE") TracingMode = CheckQeBefore;
        else if (str == "LRF")     TracingMode = LRF;
        else
        {
            TracingMode = Normal;
            qWarning() << "Bad option in TracingMode";
        }
    }

    // LRF-based signal generation
    {
        QJsonObject js;
        if (jstools::parseJson(json, "LRF", js))
        {
            jstools::parseJson(js, "PhotonsPerNode", LRF_photonsPerNode);
            jstools::parseJson(js, "PhotoElectrons", LRF_photoElectrons);
            // !!!*** error control
        }
    }
}

void APhotOptSettings::clear()
{
    MaxPhotonTransitions  = 500;
    TracingMode = Normal;
}

// ---

void ASingleSettings::clearSettings()
{
    for (size_t i = 0; i < 3; i++)
    {
        Position[i] = 0;
        PositionStr[i].clear();
    }
}

void ASingleSettings::writeToJson(QJsonObject & json) const
{
    {
        QJsonArray ar;
            for (size_t i = 0; i < 3 ; i++) ar.push_back(Position[i]);
        json["Position"] = ar;
    }

    {
        QJsonArray ar;
            for (size_t i = 0; i < 3 ; i++) ar.push_back(PositionStr[i]);
        json["PositionStr"] = ar;
    }
}

QString ASingleSettings::readFromJson(const QJsonObject & json)
{
    {
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "Position", ar);
        if (ok && ar.size() == 3)
            for (int i = 0; i < 3 ; i++) Position[i] = ar[i].toDouble();
        else return "Error in single photon bomb position data (double)";
    }

    {
        QJsonArray ar;
        bool ok = jstools::parseJson(json, "PositionStr", ar);
        if (ok && ar.size() == 3)
            for (int i = 0; i < 3 ; i++) PositionStr[i] = ar[i].toString();
        else return "Error in single photon bomb position data (string)";
    }
    return "";
}

#include "ageoconsts.h"
void ASingleSettings::updateGeoConstRelatedSimProperties()
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    QString errorStr;

    for (size_t i = 0; i < 3; i++)
    {
        if (PositionStr[i].isEmpty()) continue;

        Position[i] = 0;
        bool ok = GC.updateDoubleParameter(errorStr, PositionStr[i], Position[i], false, false, false);
        //if (!ok) qWarning() << "Error in Photon Bomb single X" << errorStr;
        if (!ok)
        {
            const QString msg = QString("Error in Photon Bomb single position (coordinate index [%1]): %2").arg(i).arg(errorStr);
            qWarning().noquote() << msg;
            AErrorHub::addQError(msg);
        }
    }
}

QString ASingleSettings::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (PositionStr[0].contains(nameRegExp)) return "Photon simulation->Photon bombs->Single->X position";
    if (PositionStr[1].contains(nameRegExp)) return "Photon simulation->Photon bombs->Single->Y position";
    if (PositionStr[2].contains(nameRegExp)) return "Photon simulation->Photon bombs->Single->Z position";
    return "";
}

void ASingleSettings::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    for (size_t i = 0; i < 3; i++)
        PositionStr[i].replace(nameRegExp, newName);
}

// ---

void updateParameter(QString & str, double & val, const QString & message, bool bForbidZero = false, bool bForbidNegative = false)
{
    if (str.isEmpty()) return;

    const AGeoConsts & GC = AGeoConsts::getConstInstance();
    val = 0;
    QString errorStr;
    bool ok = GC.updateDoubleParameter(errorStr, str, val, bForbidZero, bForbidNegative, false);
    if (!ok)
    {
        const QString msg = QString("Error in Photon sim->Photon Bombs->%1: %2").arg(message, errorStr);
        qWarning().noquote() << msg;
        AErrorHub::addQError(msg);
    }
}

// --- AFloodProfile ---

void AFloodProfile::clear()
{
    *this = AFloodProfile();
}

QString AFloodProfile::typeToString(EType type)
{
    switch (type)
    {
    case Uniform     : return "uniform";
    case Exponential : return "exponential";
    case Gauss       : return "gauss";
    case Formula     : return "formula";
    case Table       : return "table";
    }
    return "uniform";
}

void AFloodProfile::writeToJson(QJsonObject & json, bool radial) const
{
    json["Type"] = typeToString(Type);

    switch (Type)
    {
    case Uniform :
        break;
    case Exponential :
        json["AttenuationLength"] = AttenuationLength;  json["AttenuationLengthStr"] = AttenuationLengthStr;
        if (!radial) json["Direction"] = (Direction == Plus ? "+" : "-");
        break;
    case Gauss :
        if (!radial) { json["Mean"] = Mean;  json["MeanStr"] = MeanStr; }
        json["Sigma"] = Sigma;  json["SigmaStr"] = SigmaStr;
        break;
    case Formula :
        json["Formula"] = FormulaText;
        json["SamplingBins"] = SamplingBins;
        break;
    case Table :
        {
            QJsonArray ar;
            jstools::writeDPairVectorToArray(Data, ar);
            json["Data"] = ar;
            json["TableMode"] = (TableMode == Points ? "points" : "bins");
        }
        break;
    }

    if (radial && (Type == Exponential || Type == Formula || Type == Table))
        json["RadialMeaning"] = (RadialMeaning == Density ? "density" : "distribution");
}

QString AFloodProfile::readFromJson(const QJsonObject & json)
{
    clear();

    QString typeStr = "uniform";
    jstools::parseJson(json, "Type", typeStr);
    if      (typeStr == "uniform")     Type = Uniform;
    else if (typeStr == "exponential") Type = Exponential;
    else if (typeStr == "gauss")       Type = Gauss;
    else if (typeStr == "formula")     Type = Formula;
    else if (typeStr == "table")       Type = Table;
    else return "Unknown profile type: " + typeStr;

    jstools::parseJson(json, "AttenuationLength", AttenuationLength);  jstools::parseJson(json, "AttenuationLengthStr", AttenuationLengthStr);

    QString dirStr = "+";
    jstools::parseJson(json, "Direction", dirStr);
    if      (dirStr == "+") Direction = Plus;
    else if (dirStr == "-") Direction = Minus;
    else return "Unknown Direction (use \"+\" or \"-\"): " + dirStr;

    jstools::parseJson(json, "Mean",  Mean);   jstools::parseJson(json, "MeanStr",  MeanStr);
    jstools::parseJson(json, "Sigma", Sigma);  jstools::parseJson(json, "SigmaStr", SigmaStr);

    jstools::parseJson(json, "Formula", FormulaText);
    jstools::parseJson(json, "SamplingBins", SamplingBins);

    QJsonArray ar;
    if (jstools::parseJson(json, "Data", ar))
        if (!jstools::readDPairVectorFromArray(ar, Data)) return "Bad format of the table Data (expected [[x, value], ...])";

    QString modeStr = "points";
    jstools::parseJson(json, "TableMode", modeStr);
    if      (modeStr == "points") TableMode = Points;
    else if (modeStr == "bins")   TableMode = Bins;
    else return "Unknown TableMode (use \"points\" or \"bins\"): " + modeStr;

    QString meanStr = "density";
    jstools::parseJson(json, "RadialMeaning", meanStr);
    if      (meanStr == "density")      RadialMeaning = Density;
    else if (meanStr == "distribution") RadialMeaning = Distribution;
    else return "Unknown RadialMeaning (use \"density\" or \"distribution\"): " + meanStr;

    return "";
}

void AFloodProfile::updateGeoConstRelatedSimProperties(const QString & name)
{
    updateParameter(AttenuationLengthStr, AttenuationLength, "Flood->" + name + "->AttenuationLength");
    updateParameter(MeanStr,              Mean,              "Flood->" + name + "->Mean");
    updateParameter(SigmaStr,             Sigma,             "Flood->" + name + "->Sigma");
}

QString AFloodProfile::isGeoConstInUse(const QRegularExpression & nameRegExp, const QString & name) const
{
    const QString prefix = "Photon simulation->Photon bombs->Flood->" + name + "->";
    if (AttenuationLengthStr.contains(nameRegExp)) return prefix + "Attenuation length";
    if (MeanStr.contains(nameRegExp))              return prefix + "Mean";
    if (SigmaStr.contains(nameRegExp))             return prefix + "Sigma";
    if (FormulaText.contains(nameRegExp))          return prefix + "Formula";
    return "";
}

void AFloodProfile::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    AttenuationLengthStr.replace(nameRegExp, newName);
    MeanStr.replace(nameRegExp, newName);
    SigmaStr.replace(nameRegExp, newName);
    FormulaText.replace(nameRegExp, newName);
}

// --- AFloodSettings ---

QString AFloodSettings::shapeToString(EShape shape)
{
    switch (shape)
    {
    case Box      : return "box";
    case Cylinder : return "cylinder";
    case Sphere   : return "sphere";
    }
    return "box";
}

QString AFloodSettings::axisToString(EAxis axis)
{
    switch (axis)
    {
    case AxisX : return "X";
    case AxisY : return "Y";
    case AxisZ : return "Z";
    }
    return "Z";
}

void AFloodSettings::clearSettings()
{
    Number = 100;
    Shape  = Box;
    Axis   = AxisZ;

    X0 = 0;  X0Str.clear();
    Y0 = 0;  Y0Str.clear();
    Z0 = 0;  Z0Str.clear();

    SizeX = 30.0;  SizeXStr.clear();
    SizeY = 30.0;  SizeYStr.clear();
    SizeZ = 0;     SizeZStr.clear();

    Length        = 0;      LengthStr.clear();
    OuterDiameter = 300.0;  OuterDiameterStr.clear();
    InnerDiameter = 0;      InnerDiameterStr.clear();

    ProfileX.clear();
    ProfileY.clear();
    ProfileZ.clear();
    ProfileRadial.clear();
    ProfileAxial.clear();

    _Runtime.clear();
}

void AFloodSettings::writeToJson(QJsonObject & json) const
{
    json["Number"]        = Number;
    json["Shape"]         = shapeToString(Shape);
    json["Axis"]          = axisToString(Axis);

    json["CenterX"]       = X0;             json["CenterXStr"]       = X0Str;
    json["CenterY"]       = Y0;             json["CenterYStr"]       = Y0Str;
    json["CenterZ"]       = Z0;             json["CenterZStr"]       = Z0Str;

    json["SizeX"]         = SizeX;          json["SizeXStr"]         = SizeXStr;
    json["SizeY"]         = SizeY;          json["SizeYStr"]         = SizeYStr;
    json["SizeZ"]         = SizeZ;          json["SizeZStr"]         = SizeZStr;

    json["Length"]        = Length;         json["LengthStr"]        = LengthStr;
    json["OuterDiameter"] = OuterDiameter;  json["OuterDiameterStr"] = OuterDiameterStr;
    json["InnerDiameter"] = InnerDiameter;  json["InnerDiameterStr"] = InnerDiameterStr;

    auto writeProfile = [&json](const AFloodProfile & p, const QString & key, bool radial)
    {
        QJsonObject js;
        p.writeToJson(js, radial);
        json[key] = js;
    };

    switch (Shape)
    {
    case Box :
        writeProfile(ProfileX, "ProfileX", false);
        writeProfile(ProfileY, "ProfileY", false);
        writeProfile(ProfileZ, "ProfileZ", false);
        break;
    case Cylinder :
        writeProfile(ProfileRadial, "ProfileRadial", true);
        writeProfile(ProfileAxial,  "ProfileAxial",  false);
        break;
    case Sphere :
        writeProfile(ProfileRadial, "ProfileRadial", true);
        break;
    }
}

QString AFloodSettings::readFromJson(const QJsonObject & json)
{
    clearSettings();

    jstools::parseJson(json, "Number", Number);

    QString shapeStr = "undefined";
    jstools::parseJson(json, "Shape", shapeStr);

    if (shapeStr == "rectangle" || shapeStr == "ring")
        return readLegacyJson(json, shapeStr);

    if      (shapeStr == "box")      Shape = Box;
    else if (shapeStr == "cylinder") Shape = Cylinder;
    else if (shapeStr == "sphere")   Shape = Sphere;
    else return "Unknown flood shape: " + shapeStr;

    static const QStringList legacyKeys = {"Xfrom", "Xto", "Yfrom", "Yto", "Zfrom", "Zto", "Zmode", "Zfixed"};
    for (const QString & key : legacyKeys)
        if (json.contains(key) || json.contains(key + "Str"))
            return QString("Flood: key \"%1\" belongs to the old format (shape rectangle/ring). "
                           "For shape %2 use CenterX/Y/Z and SizeX/Y/Z (or Length).").arg(key, shapeStr);

    QString axisStr = "Z";
    jstools::parseJson(json, "Axis", axisStr);
    if      (axisStr == "X") Axis = AxisX;
    else if (axisStr == "Y") Axis = AxisY;
    else if (axisStr == "Z") Axis = AxisZ;
    else return "Unknown flood Axis (use \"X\", \"Y\" or \"Z\"): " + axisStr;

    jstools::parseJson(json, "CenterX", X0);   jstools::parseJson(json, "CenterXStr", X0Str);
    jstools::parseJson(json, "CenterY", Y0);   jstools::parseJson(json, "CenterYStr", Y0Str);
    jstools::parseJson(json, "CenterZ", Z0);   jstools::parseJson(json, "CenterZStr", Z0Str);

    jstools::parseJson(json, "SizeX", SizeX);  jstools::parseJson(json, "SizeXStr", SizeXStr);
    jstools::parseJson(json, "SizeY", SizeY);  jstools::parseJson(json, "SizeYStr", SizeYStr);
    jstools::parseJson(json, "SizeZ", SizeZ);  jstools::parseJson(json, "SizeZStr", SizeZStr);

    jstools::parseJson(json, "Length",        Length);         jstools::parseJson(json, "LengthStr",        LengthStr);
    jstools::parseJson(json, "OuterDiameter", OuterDiameter);  jstools::parseJson(json, "OuterDiameterStr", OuterDiameterStr);
    jstools::parseJson(json, "InnerDiameter", InnerDiameter);  jstools::parseJson(json, "InnerDiameterStr", InnerDiameterStr);

    auto readProfile = [&json](AFloodProfile & p, const QString & key) -> QString
    {
        QJsonObject js;
        if (!jstools::parseJson(json, key, js)) return "";   // absent: uniform
        const QString err = p.readFromJson(js);
        return (err.isEmpty() ? "" : "Flood->" + key + ": " + err);
    };

    for (auto & rec : std::vector<std::pair<AFloodProfile*, QString>>{ {&ProfileX, "ProfileX"}, {&ProfileY, "ProfileY"}, {&ProfileZ, "ProfileZ"},
                                                                     {&ProfileRadial, "ProfileRadial"}, {&ProfileAxial, "ProfileAxial"} })
    {
        const QString err = readProfile(*rec.first, rec.second);
        if (!err.isEmpty()) return err;
    }

    return "";
}

// conversion of a legacy from/to pair into centre and size; see the spec, section 3.4
static QString floodNumToStr(double v) {return QString::number(v, 'g', 17);}

static bool floodIsNegationOf(const QString & a, const QString & b) // both without spaces
{
    return (a == "-" + b) || (a == "-(" + b + ")");
}

static void floodConvertPair(double A, const QString & AStr, double B, const QString & BStr,
                             double & center, QString & centerStr, double & size, QString & sizeStr)
{
    center = 0.5 * (A + B);
    size   = std::fabs(B - A);
    centerStr.clear();
    sizeStr.clear();
    if (AStr.isEmpty() && BStr.isEmpty()) return;

    const QString a = (AStr.isEmpty() ? floodNumToStr(A) : AStr);
    const QString b = (BStr.isEmpty() ? floodNumToStr(B) : BStr);
    QString as = a; as.remove(' ');
    QString bs = b; bs.remove(' ');

    if (floodIsNegationOf(as, bs) || floodIsNegationOf(bs, as))
    {
        center = 0;
        const QString & pos = (B >= 0 ? b : a);
        sizeStr = "2*(" + pos + ")";
        return;
    }

    centerStr = "((" + a + ")+(" + b + "))/2";
    sizeStr   = (B >= A ? "(" + b + ")-(" + a + ")" : "(" + a + ")-(" + b + ")");
}

QString AFloodSettings::readLegacyJson(const QJsonObject & json, const QString & shapeStr)
{
    double Xfrom = -15.0, Xto = 15.0, Yfrom = -15.0, Yto = 15.0;
    QString XfromStr, XtoStr, YfromStr, YtoStr;
    jstools::parseJson(json, "Xfrom", Xfrom);   jstools::parseJson(json, "XfromStr", XfromStr);
    jstools::parseJson(json, "Xto",   Xto);     jstools::parseJson(json, "XtoStr",   XtoStr);
    jstools::parseJson(json, "Yfrom", Yfrom);   jstools::parseJson(json, "YfromStr", YfromStr);
    jstools::parseJson(json, "Yto",   Yto);     jstools::parseJson(json, "YtoStr",   YtoStr);

    double CenterX = 0, CenterY = 0;
    QString CenterXStr, CenterYStr;
    jstools::parseJson(json, "CenterX", CenterX);  jstools::parseJson(json, "CenterXStr", CenterXStr);
    jstools::parseJson(json, "CenterY", CenterY);  jstools::parseJson(json, "CenterYStr", CenterYStr);

    jstools::parseJson(json, "OuterDiameter", OuterDiameter);  jstools::parseJson(json, "OuterDiameterStr", OuterDiameterStr);
    jstools::parseJson(json, "InnerDiameter", InnerDiameter);  jstools::parseJson(json, "InnerDiameterStr", InnerDiameterStr);

    QString zStr = "undefined";
    jstools::parseJson(json, "Zmode", zStr);
    if (zStr != "fixed" && zStr != "range") return "Unknown Z mode for flood: " + zStr;

    double Zfixed = 0, Zfrom = 0, Zto = 0;
    QString ZfixedStr, ZfromStr, ZtoStr;
    jstools::parseJson(json, "Zfixed", Zfixed);  jstools::parseJson(json, "ZfixedStr", ZfixedStr);
    jstools::parseJson(json, "Zfrom",  Zfrom);   jstools::parseJson(json, "ZfromStr",  ZfromStr);
    jstools::parseJson(json, "Zto",    Zto);     jstools::parseJson(json, "ZtoStr",    ZtoStr);

    // box sizes are converted for both legacy shapes, so switching the shape in the GUI keeps sensible values
    double cx, cy;
    QString cxStr, cyStr;
    floodConvertPair(Xfrom, XfromStr, Xto, XtoStr, cx, cxStr, SizeX, SizeXStr);
    floodConvertPair(Yfrom, YfromStr, Yto, YtoStr, cy, cyStr, SizeY, SizeYStr);

    double zSize = 0;
    QString zSizeStr;
    if (zStr == "fixed")
    {
        Z0 = Zfixed;  Z0Str = ZfixedStr;    // saved Zfrom/Zto are ignored
    }
    else floodConvertPair(Zfrom, ZfromStr, Zto, ZtoStr, Z0, Z0Str, zSize, zSizeStr);

    if (shapeStr == "rectangle")
    {
        Shape = Box;
        X0 = cx;  X0Str = cxStr;           // saved CenterX/Y belong to the ring page: ignored
        Y0 = cy;  Y0Str = cyStr;
        SizeZ = zSize;  SizeZStr = zSizeStr;
    }
    else
    {
        Shape = Cylinder;
        Axis  = AxisZ;
        X0 = CenterX;  X0Str = CenterXStr;
        Y0 = CenterY;  Y0Str = CenterYStr;
        Length = zSize;  LengthStr = zSizeStr;
    }

    return "";
}

void AFloodSettings::updateGeoConstRelatedSimProperties()
{
    updateParameter(X0Str, X0, "Flood->Position X");
    updateParameter(Y0Str, Y0, "Flood->Position Y");
    updateParameter(Z0Str, Z0, "Flood->Position Z");

    updateParameter(SizeXStr, SizeX, "Flood->Size X", false, true);
    updateParameter(SizeYStr, SizeY, "Flood->Size Y", false, true);
    updateParameter(SizeZStr, SizeZ, "Flood->Size Z", false, true);

    updateParameter(LengthStr,        Length,        "Flood->Length",         false, true);
    updateParameter(OuterDiameterStr, OuterDiameter, "Flood->Outer diameter", true,  true);
    updateParameter(InnerDiameterStr, InnerDiameter, "Flood->Inner diameter", false, true);

    ProfileX.updateGeoConstRelatedSimProperties("ProfileX");
    ProfileY.updateGeoConstRelatedSimProperties("ProfileY");
    ProfileZ.updateGeoConstRelatedSimProperties("ProfileZ");
    ProfileRadial.updateGeoConstRelatedSimProperties("ProfileRadial");
    ProfileAxial.updateGeoConstRelatedSimProperties("ProfileAxial");
}

QString AFloodSettings::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    const QString prefix = "Photon simulation->Photon bombs->Flood->";
    if (X0Str.contains(nameRegExp)) return prefix + "Position X";
    if (Y0Str.contains(nameRegExp)) return prefix + "Position Y";
    if (Z0Str.contains(nameRegExp)) return prefix + "Position Z";

    if (SizeXStr.contains(nameRegExp)) return prefix + "Size X";
    if (SizeYStr.contains(nameRegExp)) return prefix + "Size Y";
    if (SizeZStr.contains(nameRegExp)) return prefix + "Size Z";

    if (LengthStr.contains(nameRegExp))        return prefix + "Length";
    if (OuterDiameterStr.contains(nameRegExp)) return prefix + "Outer diameter";
    if (InnerDiameterStr.contains(nameRegExp)) return prefix + "Inner diameter";

    QString str;
    str = ProfileX.isGeoConstInUse(nameRegExp, "ProfileX");           if (!str.isEmpty()) return str;
    str = ProfileY.isGeoConstInUse(nameRegExp, "ProfileY");           if (!str.isEmpty()) return str;
    str = ProfileZ.isGeoConstInUse(nameRegExp, "ProfileZ");           if (!str.isEmpty()) return str;
    str = ProfileRadial.isGeoConstInUse(nameRegExp, "ProfileRadial"); if (!str.isEmpty()) return str;
    str = ProfileAxial.isGeoConstInUse(nameRegExp, "ProfileAxial");   if (!str.isEmpty()) return str;

    return "";
}

void AFloodSettings::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    X0Str.replace(nameRegExp, newName);
    Y0Str.replace(nameRegExp, newName);
    Z0Str.replace(nameRegExp, newName);

    SizeXStr.replace(nameRegExp, newName);
    SizeYStr.replace(nameRegExp, newName);
    SizeZStr.replace(nameRegExp, newName);

    LengthStr.replace(nameRegExp, newName);
    OuterDiameterStr.replace(nameRegExp, newName);
    InnerDiameterStr.replace(nameRegExp, newName);

    ProfileX.replaceGeoConstName(nameRegExp, newName);
    ProfileY.replaceGeoConstName(nameRegExp, newName);
    ProfileZ.replaceGeoConstName(nameRegExp, newName);
    ProfileRadial.replaceGeoConstName(nameRegExp, newName);
    ProfileAxial.replaceGeoConstName(nameRegExp, newName);
}

QString AFloodSettings::updateRuntimeProperties(QString * warnings)
{
    return _Runtime.configure(*this, warnings);
}

bool AFloodSettings::generatePosition(double * r) const
{
    return _Runtime.generatePosition(r);
}

// ---

void APhotonBombsSettings::writeToJson(QJsonObject & json) const
{
    {
        QJsonObject js;
        PhotonsPerBomb.writeToJson(js);
        json["PhotonsPerBomb"] = js;
    }

    // GenerationMode
    {
        QString str;
        switch (GenerationMode)
        {
        case EBombGen::Single : str = "single"; break;
        case EBombGen::Grid   : str = "grid";   break;
        case EBombGen::Flood  : str = "flood";  break;
        case EBombGen::File   : str = "file";   break;
        }
        json["GenerationMode"] = str;
    }

    // Particular generation modes
    // Single
    {
        QJsonObject js;
        SingleSettings.writeToJson(js);
        json["Single"] = js;
    }
    // Grid
    {
        QJsonObject js;
        GridSettings.writeToJson(js);
        json["Grid"] = js;
    }
    //Flood
    {
        QJsonObject js;
        FloodSettings.writeToJson(js);
        json["Flood"] = js;
    }
    //File
    {
        QJsonObject js;
        BombFileSettings.writeToJson(js);
        json["File"] = js;
    }

    //Advanced settings
    {
        QJsonObject js;
        AdvancedSettings.writeToJson(js);
        json["Advanced"] = js;
    }
}

QString APhotonBombsSettings::readFromJson(const QJsonObject & json)
{
    // PhotonNumberMode
    {
        QJsonObject js;
        jstools::parseJson(json, "PhotonsPerBomb", js);
        QString ErrorString = PhotonsPerBomb.readFromJson(js);
        if (!ErrorString.isEmpty()) return ErrorString;
    }

    // GenerationMode
    {
        QString str;
        jstools::parseJson(json, "GenerationMode",  str);

        if      (str == "single") GenerationMode = EBombGen::Single;
        else if (str == "grid")   GenerationMode = EBombGen::Grid;
        else if (str == "flood")  GenerationMode = EBombGen::Flood;
        else if (str == "file")   GenerationMode = EBombGen::File;
        else
        {
            GenerationMode = EBombGen::Single;
            return QString("Unknown GenerationMode: %1 -> replacing with 'Single'").arg(str);
        }
    }

    // Particular generation modes
    // Single
    {
        QJsonObject js;
        jstools::parseJson(json, "Single", js);
        QString ErrorString = SingleSettings.readFromJson(js);
        if (!ErrorString.isEmpty()) return ErrorString;
    }
    // Grid
    {
        QJsonObject js;
        jstools::parseJson(json, "Grid", js);
        QString ErrorString = GridSettings.readFromJson(js);
        if (!ErrorString.isEmpty()) return ErrorString;
    }
    //Flood
    {
        QJsonObject js;
        jstools::parseJson(json, "Flood", js);
        QString ErrorString = FloodSettings.readFromJson(js);
        if (!ErrorString.isEmpty()) return ErrorString;
    }
    //File
    {
        QJsonObject js;
        jstools::parseJson(json, "File", js);
        BombFileSettings.readFromJson(js);
    }

    //Advanced settings
    {
        QJsonObject js;
        jstools::parseJson(json, "Advanced", js);
        AdvancedSettings.readFromJson(js);
    }

    return "";
}

void APhotonBombsSettings::clear()
{
    GenerationMode = EBombGen::Single;
    PhotonsPerBomb.clearSettings();

    SingleSettings.clearSettings();
    GridSettings.clearSettings();
    FloodSettings.clearSettings();
    BombFileSettings.clear();

    AdvancedSettings.clear();
}

void APhotonBombsSettings::updateGeoConstRelatedSimProperties()
{
    SingleSettings.updateGeoConstRelatedSimProperties();
    GridSettings.  updateGeoConstRelatedSimProperties();
    FloodSettings. updateGeoConstRelatedSimProperties();
}

QString APhotonBombsSettings::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    QString str;
    str = SingleSettings.isGeoConstInUse(nameRegExp); if (!str.isEmpty()) return str;
    str = GridSettings.  isGeoConstInUse(nameRegExp); if (!str.isEmpty()) return str;
    str = FloodSettings. isGeoConstInUse(nameRegExp); if (!str.isEmpty()) return str;
    return "";
}

void APhotonBombsSettings::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    SingleSettings.replaceGeoConstName(nameRegExp, newName);
    GridSettings.  replaceGeoConstName(nameRegExp, newName);
    FloodSettings. replaceGeoConstName(nameRegExp, newName);
}

// ---

void APhotSimRunSettings::writeToJson(QJsonObject & json, bool addRuntimeExport) const
{
    json["Seed"]                  = Seed;

    if (addRuntimeExport)
    {
        json["EventFrom"]             = EventFrom;
        json["EventTo"]               = EventTo;
    }

    json["OutputDirectory"]       = OutputDirectory;
    json["BinaryFormat"]          = BinaryFormat;

    json["FileNameReceipt"]       = FileNameReceipt;

    json["SaveSensorSignals"]     = SaveSensorSignals;
    json["FileNameSensorSignals"] = FileNameSensorSignals;

    json["SaveSensorLog"]         = SaveSensorLog;
    json["FileNameSensorLog"]     = FileNameSensorLog;
    json["SensorLogTime"]         = SensorLogTime;
    json["SensorLogXY"]           = SensorLogXY;
    json["SensorLogAngle"]        = SensorLogAngle;
    json["SensorLogWave"]         = SensorLogWave;

    json["SavePhotonBombs"]       = SavePhotonBombs;
    json["FileNamePhotonBombs"]   = FileNamePhotonBombs;

    json["SaveTracks"]            = SaveTracks;
    json["MaxTracks"]             = MaxTracks;
    json["FileNameTracks"]        = FileNameTracks;

    json["SaveStatistics"]        = SaveStatistics;
    json["FileNameStatistics"]    = FileNameStatistics;
    json["UpperTimeLimit"]        = UpperTimeLimit;

    json["SaveMonitors"]          = SaveMonitors;
    json["FileNameMonitors"]      = FileNameMonitors;
    json["SaveMonitorLog"]        = SaveMonitorLog;
    json["FileNameMonitorLog"]    = FileNameMonitorLog;

    json["SaveConfig"]            = SaveConfig;
    json["FileNameConfig"]        = FileNameConfig;

    {
        QJsonObject js;
        PhotonLogSet.writeToJson(js);
        json["PhotonLog"] = js;
    }
}

void APhotSimRunSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Seed",                  Seed);

    jstools::parseJson(json, "EventFrom",             EventFrom);
    jstools::parseJson(json, "EventTo",               EventTo);

    jstools::parseJson(json, "OutputDirectory",       OutputDirectory);
    jstools::parseJson(json, "BinaryFormat",          BinaryFormat);

    jstools::parseJson(json, "FileNameReceipt",       FileNameReceipt);

    jstools::parseJson(json, "SaveSensorSignals",     SaveSensorSignals);
    jstools::parseJson(json, "FileNameSensorSignals", FileNameSensorSignals);

    jstools::parseJson(json, "SaveSensorLog",         SaveSensorLog);
    jstools::parseJson(json, "FileNameSensorLog",     FileNameSensorLog);
    jstools::parseJson(json, "SensorLogTime",         SensorLogTime);
    jstools::parseJson(json, "SensorLogXY",           SensorLogXY);
    jstools::parseJson(json, "SensorLogAngle",        SensorLogAngle);
    jstools::parseJson(json, "SensorLogWave",         SensorLogWave);

    jstools::parseJson(json, "SavePhotonBombs",       SavePhotonBombs);
    jstools::parseJson(json, "FileNamePhotonBombs",   FileNamePhotonBombs);

    jstools::parseJson(json, "SaveTracks",            SaveTracks);
    jstools::parseJson(json, "MaxTracks",             MaxTracks);
    jstools::parseJson(json, "FileNameTracks",        FileNameTracks);

    jstools::parseJson(json, "SaveStatistics",        SaveStatistics);
    jstools::parseJson(json, "FileNameStatistics",    FileNameStatistics);
    jstools::parseJson(json, "UpperTimeLimit",        UpperTimeLimit);

    jstools::parseJson(json, "SaveMonitors",          SaveMonitors);
    jstools::parseJson(json, "FileNameMonitors",      FileNameMonitors);
    jstools::parseJson(json, "SaveMonitorLog",        SaveMonitorLog);
    jstools::parseJson(json, "FileNameMonitorLog",    FileNameMonitorLog);

    jstools::parseJson(json, "SaveConfig",            SaveConfig);
    jstools::parseJson(json, "FileNameConfig",        FileNameConfig);

    {
        QJsonObject js;
        jstools::parseJson(json, "PhotonLog", js);
        PhotonLogSet.readFromJson(js);
    }
}

void APhotSimRunSettings::clear()
{
    Seed = 0;

    EventFrom = 0;
    EventTo   = 0;

    OutputDirectory.clear();
    BinaryFormat = false;

    SaveSensorSignals     = false;
    FileNameSensorSignals = "SensorSignals.txt";

    SaveSensorLog         = false;
    FileNameSensorLog     = "SensorLog.dat";
    SensorLogTime         = true;
    SensorLogXY           = false;
    SensorLogAngle        = false;
    SensorLogWave         = false;

    SavePhotonBombs       = false;
    FileNamePhotonBombs   = "PhotonBombs.txt";

    SaveTracks            = true;
    MaxTracks             = 1000;
    FileNameTracks        = "PhotonTracks.txt";

    SaveStatistics        = true;
    FileNameStatistics    = "PhotonStatistics.json";
    UpperTimeLimit        = 100;

    SaveMonitors          = false;
    FileNameMonitors      = "PhotonMonitors.json";
    SaveMonitorLog        = false;
    FileNameMonitorLog    = "PhotonMonitorLog.dat";

    SaveConfig            = false;
    FileNameConfig        = "Config_OpticalSim.json";

    PhotonLogSet.clear();
}

// ---

void APhotonSimSettings::writeToJson(QJsonObject & json, bool addRuntimeExport) const
{
    QJsonObject jsSim;

    // Scint
    {
        QJsonObject js;
            js["Primary"]   = PrimaryScint;
            js["Secondary"] = SecondaryScint;
        jsSim["Scintillation"] = js;
    }

    // Wave
    {
        QJsonObject js;
        WaveSet.writeToJson(js);
        jsSim["WaveResolved"] = js;
    }

    // Max transitions
    {
        QJsonObject js;
        OptSet.writeToJson(js);
        jsSim["Optimization"] = js;
    }

    // Photon generation overrides
    {
        QJsonObject js;
        PhGenOverrideSet.writeToJson(js);
        jsSim["PhGenOverrides"] = js;
    }

    // Type
    {
        QString str;
        switch (SimType)
        {
        case EPhotSimType::PhotonBombs       : str = "bomb"; break;
        case EPhotSimType::FromEnergyDepo    : str = "depo"; break;
        case EPhotSimType::IndividualPhotons : str = "indi"; break;
        }
        jsSim["SimulationType"] = str;
    }

    // Particular modes
    {
        QJsonObject js;
        BombSet.writeToJson(js);
        jsSim["PhotonBombs"] = js;
    }
    {
        QJsonObject js;
        DepoSet.writeToJson(js);
        jsSim["Deposition"] = js;
    }
    {
        QJsonObject js;
        PhotFileSet.writeToJson(js);
        jsSim["PhotonFile"] = js;
    }
    //Run
    {
        QJsonObject js;
        RunSet.writeToJson(js, addRuntimeExport);
        jsSim["Run"] = js;
    }

    json["PhotonSim"] = jsSim;
}

QString APhotonSimSettings::readFromJson(const QJsonObject & json)
{
    QJsonObject jsSim;
    bool ok = jstools::parseJson(json, "PhotonSim", jsSim);
    if (!ok) return "Json does not contain photon sim settings!\n";

    // Scint
    {
        QJsonObject js;
        ok = jstools::parseJson(jsSim, "Scintillation", js);
        if (ok)
        {
            jstools::parseJson(js, "Primary", PrimaryScint);
            jstools::parseJson(js, "Secondary", SecondaryScint);
        }
        else
        {
            // old system, compatibility
            QJsonObject js;
            ok = jstools::parseJson(jsSim, "Deposition", js);
            if (ok)
            {
                jstools::parseJson(js, "Primary",   PrimaryScint);
                jstools::parseJson(js, "Secondary", SecondaryScint);
            }
        }
    }

    // Wave
    {
        QJsonObject js;
        ok = jstools::parseJson(jsSim, "WaveResolved", js);
        if (!ok) return "Json does not contain wavelength-related settings!\n";
        WaveSet.readFromJson(js);
    }

    // Max transitions
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "Optimization", js);
        OptSet.readFromJson(js);
    }

    // Photon generation overrides
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "PhGenOverrides", js);
        PhGenOverrideSet.readFromJson(js);
    }

    // Type
    {
        QString str = "undefined";
        jstools::parseJson(jsSim, "SimulationType", str);
        if      (str == "bomb") SimType = EPhotSimType::PhotonBombs;
        else if (str == "depo") SimType = EPhotSimType::FromEnergyDepo;
        else if (str == "indi") SimType = EPhotSimType::IndividualPhotons;
        else
        {
            SimType = EPhotSimType::PhotonBombs;
            return QString("Unknown photon simulation mode: %1").arg(str);
        }
    }

    // Particular modes
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "PhotonBombs", js);
        BombSet.readFromJson(js);
    }
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "Deposition", js);
        DepoSet.readFromJson(js);
    }
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "PhotonFile", js);
        PhotFileSet.readFromJson(js);
    }

    //Run
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "Run", js);
        RunSet.readFromJson(js);
    }

    return "";
}

void APhotonSimSettings::clear()
{
    PrimaryScint = true;
    SecondaryScint = false;

    SimType = EPhotSimType::PhotonBombs;

    BombSet.clear();
    DepoSet.clear();
    PhotFileSet.clear();

    WaveSet.clear();
    OptSet.clear();

    PhGenOverrideSet.clear();

    RunSet.clear();
}

// ---- Depo ----

void APhotonDepoSettings::clear()
{
    AFileSettingsBase::clear();

    //Primary          = true;
    //Secondary        = false;
}

void APhotonDepoSettings::doWriteToJson(QJsonObject &json) const
{
    //json["Primary"]    = Primary;
    //json["Secondary"]  = Secondary;
}

void APhotonDepoSettings::doReadFromJson(const QJsonObject &json)
{
    //jstools::parseJson(json, "Primary",   Primary);
    //jstools::parseJson(json, "Secondary", Secondary);
}

// ----

AGridSettings::AGridSettings()
{
    clearSettings();
}

int AGridSettings::getNumEvents() const
{
    int num = ScanRecords[0].Nodes;
    if (ScanRecords[1].bEnabled) num *= ScanRecords[1].Nodes;
    if (ScanRecords[2].bEnabled) num *= ScanRecords[2].Nodes;
    return num;
}

void AGridSettings::clearSettings()
{
    for (size_t i = 0; i < 3; i++)
        ScanRecords[i] = APhScanRecord();
    ScanRecords[0].bEnabled = true;
    ScanRecords[1].DX = 0; ScanRecords[1].DY = 10.0;
    ScanRecords[2].DX = 0; ScanRecords[2].DZ = 10.0;

    X0 = 0; X0Str.clear();
    Y0 = 0; Y0Str.clear();
    Z0 = 0; Z0Str.clear();
}

void AGridSettings::writeToJson(QJsonObject & json) const
{
    json["ScanX0"] = X0;    json["ScanX0Str"] = X0Str;
    json["ScanY0"] = Y0;    json["ScanY0Str"] = Y0Str;
    json["ScanZ0"] = Z0;    json["ScanZ0Str"] = Z0Str;

    QJsonArray ar;
        for (size_t i = 0; i < 3; i++)
        {
            QJsonObject js;
                const APhScanRecord & r = ScanRecords[i];
                js["Enabled"]   = r.bEnabled;
                js["BiDirect"] = r.bBiDirect;
                js["Nodes"]     = r.Nodes;
                js["dX"]        = r.DX;     js["dXStr"] = r.DXStr;
                js["dY"]        = r.DY;     js["dYStr"] = r.DYStr;
                js["dZ"]        = r.DZ;     js["dZStr"] = r.DZStr;
            ar.append(js);
        }
    json["AxesData"] = ar;
}

QString AGridSettings::readFromJson(const QJsonObject & json)
{
    clearSettings();

    jstools::parseJson(json, "ScanX0", X0);     jstools::parseJson(json, "ScanX0Str", X0Str);
    jstools::parseJson(json, "ScanY0", Y0);     jstools::parseJson(json, "ScanY0Str", Y0Str);
    jstools::parseJson(json, "ScanZ0", Z0);     jstools::parseJson(json, "ScanZ0Str", Z0Str);

    QJsonArray ar;
    bool bOK = jstools::parseJson(json, "AxesData", ar);
    if (!bOK || ar.size() != 3) return "Bad format in photon sim grid settings";

    for (size_t i = 0; i < 3; i++)
    {
        QJsonObject js = ar[i].toObject();
        APhScanRecord & r = ScanRecords[i];
        jstools::parseJson(js, "Enabled",  r.bEnabled);
        jstools::parseJson(js, "BiDirect", r.bBiDirect);
        jstools::parseJson(js, "Nodes",    r.Nodes);
        jstools::parseJson(js, "dX",       r.DX);       jstools::parseJson(js, "dXStr", r.DXStr);
        jstools::parseJson(js, "dY",       r.DY);       jstools::parseJson(js, "dYStr", r.DYStr);
        jstools::parseJson(js, "dZ",       r.DZ);       jstools::parseJson(js, "dZStr", r.DZStr);
    }

    ScanRecords[0].bEnabled = true;
    return "";
}

void AGridSettings::updateGeoConstRelatedSimProperties()
{
    updateParameter(X0Str, X0, "X origin");
    updateParameter(Y0Str, Y0, "Y origin");
    updateParameter(Z0Str, Z0, "Z origin");

    for (size_t i = 0; i < 3; i++)
    {
        APhScanRecord & sr = ScanRecords[i];
        updateParameter(sr.DXStr, sr.DX, QString("X step [%1]").arg(i));
        updateParameter(sr.DYStr, sr.DY, QString("Y step [%1]").arg(i));
        updateParameter(sr.DZStr, sr.DZ, QString("Z step [%1]").arg(i));
    }
}

QString AGridSettings::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (X0Str.contains(nameRegExp)) return "Photon simulation->Photon bombs->Grid->X origin";
    if (Y0Str.contains(nameRegExp)) return "Photon simulation->Photon bombs->Grid->Y origin";
    if (Z0Str.contains(nameRegExp)) return "Photon simulation->Photon bombs->Grid->Z origin";

    for (size_t i = 0; i < 3; i++)
    {
        const APhScanRecord & sr = ScanRecords[i];
        if (sr.DXStr.contains(nameRegExp)) return QString("Photon simulation->Photon bombs->Grid->X step [%0])").arg(i);
        if (sr.DYStr.contains(nameRegExp)) return QString("Photon simulation->Photon bombs->Grid->Y step [%0])").arg(i);
        if (sr.DZStr.contains(nameRegExp)) return QString("Photon simulation->Photon bombs->Grid->Z step [%0])").arg(i);
    }

    return "";
}

void AGridSettings::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    X0Str.replace(nameRegExp, newName);
    Y0Str.replace(nameRegExp, newName);
    Z0Str.replace(nameRegExp, newName);

    for (APhScanRecord & sr : ScanRecords)
    {
        sr.DXStr.replace(nameRegExp, newName);
        sr.DYStr.replace(nameRegExp, newName);
        sr.DZStr.replace(nameRegExp, newName);
    }
}

// ----

void APhotonBombAdvancedSettings::clear()
{
    bOnlyVolume    = false;
    Volume.clear();
    bOnlyMaterial    = false;
    Material.clear();
    MaxNodeAttempts = 1000;
}

void APhotonBombAdvancedSettings::writeToJson(QJsonObject & json) const
{
    QJsonObject js;
        js["EnableOnlyVol"]   = bOnlyVolume;
        js["Volume"]          = Volume;
        js["EnableOnlyMat"]   = bOnlyMaterial;
        js["Material"]        = Material;
        js["MaxNodeAttempts"] = MaxNodeAttempts;
    json["SkipBombs"] = js;
}

void APhotonBombAdvancedSettings::readFromJson(const QJsonObject & json)
{
    clear();

    QJsonObject js;
    jstools::parseJson(json, "SkipBombs", js);
        jstools::parseJson(js, "EnableOnlyVol",   bOnlyVolume);
        jstools::parseJson(js, "Volume",          Volume);
        jstools::parseJson(js, "EnableOnlyMat",   bOnlyMaterial);
        jstools::parseJson(js, "Material",        Material);
        jstools::parseJson(js, "MaxNodeAttempts", MaxNodeAttempts);
}

// ----

void APhGenOverrideSettings::clear()
{
    DirectionMode = Isotropic;
    DirDX         = 0;
    DirDY         = 0;
    DirDZ         = 1.0;
    ConeAngle     = 10.0;

    bFixWave      = false;
    FixedWavelength  = 550.0;

    bFixDecay     = false;
    DecayTime  = 5.0; // in ns
}

void APhGenOverrideSettings::writeToJson(QJsonObject & json) const
{
    // Direction
    {
        QJsonObject js;
        QString DirStr;
        switch (DirectionMode)
        {
        case Isotropic : DirStr = "Isotropic"; break;
        case Fixed     : DirStr = "Fixed";     break;
        case Cone      : DirStr = "Cone";     break;
        }
        js["DirectionMode"] = DirStr;

        //js["DirDX"] = DirDX;
        //js["DirDY"] = DirDY;
        //js["DirDZ"] = DirDZ;
        QJsonArray ar;
        ar << DirDX << DirDY << DirDZ;
        js["DirectionVector"] = ar;

        js["ConeAngle"] = ConeAngle;
        json["Direction"] = js;
    }

    // Wave index
    {
        QJsonObject js;
        js["Enabled"] = bFixWave;
        js["FixedWavelength"] = FixedWavelength;
        json["Wave"] = js;
    }

    // Decay time
    {
        QJsonObject js;
        js["Enabled"]   = bFixDecay;
        js["DecayTime"] = DecayTime;
        json["Time"] = js;
    }
}

void APhGenOverrideSettings::readFromJson(const QJsonObject &json)
{
    clear();

    // Direction
    {
        QJsonObject js;
        jstools::parseJson(json, "Direction", js);

        QString DirStr;
        jstools::parseJson(js, "DirectionMode", DirStr);
        if      (DirStr == "Fixed") DirectionMode = Fixed;
        else if (DirStr == "Cone")  DirectionMode = Cone;
        else                        DirectionMode = Isotropic;

        //jstools::parseJson(js, "DirDX",     DirDX);
        //jstools::parseJson(js, "DirDY",     DirDY);
        //jstools::parseJson(js, "DirDZ",     DirDZ);
        QJsonArray ar;
        if ( jstools::parseJson(js, "DirectionVector", ar) && ar.size() == 3)
        {
            DirDX = ar[0].toDouble();
            DirDY = ar[1].toDouble();
            DirDZ = ar[2].toDouble();
        }
        if (DirDX == 0 && DirDY == 0 && DirDZ == 0) DirDZ = 1.0;

        jstools::parseJson(js, "ConeAngle", ConeAngle);
    }

    // Wave index
    {
        QJsonObject js;
        jstools::parseJson(json, "Wave", js);
        jstools::parseJson(js, "Enabled",   bFixWave);
        jstools::parseJson(js, "FixedWavelength", FixedWavelength);
    }

    // Decay time
    {
        QJsonObject js;
        jstools::parseJson(json, "Time", js);
        jstools::parseJson(js, "Enabled",   bFixDecay);
        jstools::parseJson(js, "DecayTime", DecayTime);
    }
}

// ----

void APhotonLogSettings::writeToJson(QJsonObject & json) const
{
    json["Enabled"]  = Enabled;
    json["FileName"] = FileName;

    json["MaxNumber"] = MaxNumber;

    {
        QJsonArray ar;
        for (int pr : MustNotInclude_Processes) ar.push_back(pr);
        json["MustNotInclude_Processes"] = ar;
    }

    {
        QJsonArray ar;
        for (int pr : MustInclude_Processes) ar.push_back(pr);
        json["MustInclude_Processes"] = ar;
    }

    {
        QJsonArray ar;
        for (const TString & vol : MustNotInclude_Volumes) ar.push_back(QString(vol.Data()));
        json["MustNotInclude_Volumes"] = ar;
    }

    {
        QJsonArray ar;
        for (const AVolumeIndexPair & pair : MustInclude_Volumes)
        {
            QJsonArray el;
                el.push_back(QString(pair.Volume.Data()));
                el.push_back(pair.Index);
            ar.push_back(el);
        }
        json["MustInclude_Volumes"] = ar;
    }
}

void APhotonLogSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Enabled", Enabled);
    jstools::parseJson(json, "FileName", FileName);

    jstools::parseJson(json, "MaxNumber", MaxNumber);

    MustNotInclude_Processes.clear();
    {
        QJsonArray ar;
        jstools::parseJson(json, "MustNotInclude_Processes", ar);
        for (int i = 0; i < ar.size(); i++)
            MustNotInclude_Processes.emplace(ar[i].toInt());
    }

    MustInclude_Processes.clear();
    {
        QJsonArray ar;
        jstools::parseJson(json, "MustInclude_Processes", ar);
        for (int i = 0; i < ar.size(); i++)
            MustInclude_Processes.push_back(ar[i].toInt());
    }

    MustNotInclude_Volumes.clear();
    {
        QJsonArray ar;
        jstools::parseJson(json, "MustNotInclude_Volumes", ar);
        for (int i = 0; i < ar.size(); i++)
            MustNotInclude_Volumes.emplace(ar[i].toString().toLatin1().data());
    }

    MustInclude_Volumes.clear();
    {
        QJsonArray ar;
        jstools::parseJson(json, "MustInclude_Volumes", ar);
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonArray el = ar[i].toArray();
            if (el.size() == 2) // !!!*** error handling
            {
                TString vol = el[0].toString().toLatin1().data();
                int index = el[1].toInt();
                MustInclude_Volumes.push_back({vol, index});
            }
        }
    }
}

void APhotonLogSettings::clear()
{
    Enabled  = false;
    FileName = "PhotonLog.txt";

    MustNotInclude_Processes.clear();
    MustInclude_Processes.clear();
    MustNotInclude_Volumes.clear();
    MustInclude_Volumes.clear();
}
