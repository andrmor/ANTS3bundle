#include "aphotonsimsettings.h"
#include "ajsontools.h"

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

void AWaveResSettings::toStandardBins(const std::vector<double> & wavelength, const std::vector<double> & value, std::vector<double> & binnedValue) const
{
    binnedValue.clear();

    double wave, binned;
    const int points = countNodes();
    for (int iP = 0; iP < points; iP++)
    {
        wave = From + Step * iP;
        if (wave <= wavelength.front()) binned = 0; // before November 2024: value.front();
        else
        {
            if (wave >= wavelength.back()) binned = 0; // before November 2024: value.back();
            else                           binned = getInterpolatedValue(wave, wavelength, value);
        }
        binnedValue.push_back(binned);
    }
}

void AWaveResSettings::toStandardBins(const std::vector<std::pair<double,double>> & waveAndData, std::vector<double> & binnedValue) const
{
    std::vector<double> wavVec, valVec;

    for (const auto & pair : waveAndData)
    {
        wavVec.push_back(pair.first);
        valVec.push_back(pair.second);
    }

    toStandardBins(wavVec, valVec, binnedValue);
}

void AWaveResSettings::toStandardBins(const std::vector<std::pair<double, std::complex<double>>> & waveReIm, std::vector<std::complex<double>> & reIm) const
{
    std::vector<double> wavelength, realValue, imagValue, realBinned, imagBinned;

    for (const auto & rec : waveReIm)
    {
        wavelength.push_back(rec.first);
        realValue.push_back(rec.second.real());
        imagValue.push_back(rec.second.imag());
    }

    toStandardBins(wavelength, realValue, realBinned);
    toStandardBins(wavelength, imagValue, imagBinned);

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
    json["CheckQeBeforeTracking"] = CheckQeBeforeTracking;
}

void APhotOptSettings::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "MaxPhotonTransitions",  MaxPhotonTransitions);
    jstools::parseJson(json, "CheckQeBeforeTracking", CheckQeBeforeTracking);
}

void APhotOptSettings::clear()
{
    MaxPhotonTransitions  = 500;
    CheckQeBeforeTracking = false;
}

// ---

void ASingleSettings::clearSettings()
{
    Position[0] = Position[1] = Position[2] = 0;
}

void ASingleSettings::writeToJson(QJsonObject &json) const
{
    QJsonObject js;
        QJsonArray ar;
        for (int i = 0; i < 3 ; i++) ar.push_back(Position[i]);
    json["Position"] = ar;
}

QString ASingleSettings::readFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "Position", ar);
    if (ok && ar.size() == 3)
        for (int i = 0; i < 3 ; i++) Position[i] = ar[i].toDouble();
    else return "Error in single photon bomb position data";
    return "";
}

// ---

void AFloodSettings::clearSettings()
{
    Number   = 100;
    Shape    = Rectangular;
    Xfrom    = -15.0;
    Xto      =  15.0;
    Yfrom    = -15.0;
    Yto      =  15.0;
    X0       = 0;
    Y0       = 0;
    OuterDiameter   = 300.0;
    InnerDiameter   = 0;
    Zmode    = Fixed;
    Zfixed   = 0;
    Zfrom    = 0;
    Zto      = 0;
}

void AFloodSettings::writeToJson(QJsonObject &json) const
{
    json["Number"]        = Number;
    json["Shape"]         = (Shape == Rectangular ? "rectangle" : "ring");
    json["Xfrom"]         = Xfrom;
    json["Xto"]           = Xto;
    json["Yfrom"]         = Yfrom;
    json["Yto"]           = Yto;
    json["CenterX"]       = X0;
    json["CenterY"]       = Y0;
    json["OuterDiameter"] = OuterDiameter;
    json["InnerDiameter"] = InnerDiameter;
    json["Zmode"]         = (Zmode == Fixed ? "fixed" : "range");
    json["Zfixed"]        = Zfixed;
    json["Zfrom"]         = Zfrom;
    json["Zto"]           = Zto;
}

QString AFloodSettings::readFromJson(const QJsonObject & json)
{
    clearSettings();

    jstools::parseJson(json, "Number", Number);

    QString shapeStr = "undefined";
    jstools::parseJson(json, "Shape", shapeStr);
    if      (shapeStr == "rectangle") Shape = Rectangular;
    else if (shapeStr == "ring")      Shape = Ring;
    else return "Unknown flood shape: " + shapeStr;

    jstools::parseJson(json, "Xfrom", Xfrom);
    jstools::parseJson(json, "Xto",   Xto);
    jstools::parseJson(json, "Yfrom", Yfrom);
    jstools::parseJson(json, "Yto",   Yto);

    jstools::parseJson(json, "CenterX", X0);
    jstools::parseJson(json, "CenterY", Y0);

    jstools::parseJson(json, "OuterDiameter", OuterDiameter);
    jstools::parseJson(json, "InnerDiameter", InnerDiameter);

    QString zStr = "undefined";
    jstools::parseJson(json, "Zmode", zStr);
    if      (zStr == "fixed") Zmode = Fixed;
    else if (zStr == "range") Zmode = Range;
    else return "Unknown Z mode for flood: " + zStr;

    jstools::parseJson(json, "Zfixed", Zfixed);
    jstools::parseJson(json, "Zfrom",  Zfrom);
    jstools::parseJson(json, "Zto",    Zto);

    return "";
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
    FileNameMonitors      = "PhotonMonitors.txt";

    SaveConfig            = false;
    FileNameConfig        = "Config_OpticalSim.json";

    PhotonLogSet.clear();
}

// ---

void APhotonSimSettings::writeToJson(QJsonObject & json, bool addRuntimeExport) const
{
    QJsonObject jsSim;
    // Wave
    {
        QJsonObject js;
        WaveSet.writeToJson(js);
        jsSim["WaveResolved"] = js;
    }
    // Max trans and QE check before trace
    {
        QJsonObject js;
        OptSet.writeToJson(js);
        jsSim["Optimization"] = js;
    }
    // Type
    {
        QString str;
        switch (SimType)
        {
        case EPhotSimType::PhotonBombs       : str = "bomb"; break;
        case EPhotSimType::FromEnergyDepo    : str = "depo"; break;
        case EPhotSimType::IndividualPhotons : str = "indi"; break;
        case EPhotSimType::FromLRFs          : str = "lrf";  break;
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

    // Wave
    {
        QJsonObject js;
        ok = jstools::parseJson(jsSim, "WaveResolved", js);
        if (!ok) return "Json does not contain wavelength-related settings!\n";
        WaveSet.readFromJson(js);
    }
    // Max trans and QE check before trace
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "Optimization", js);
        OptSet.readFromJson(js);
    }
    // Type
    {
        QString str = "undefined";
        jstools::parseJson(jsSim, "SimulationType", str);
        if      (str == "bomb") SimType = EPhotSimType::PhotonBombs;
        else if (str == "depo") SimType = EPhotSimType::FromEnergyDepo;
        else if (str == "indi") SimType = EPhotSimType::IndividualPhotons;
        else if (str == "lrf")  SimType = EPhotSimType::FromLRFs;
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
    SimType = EPhotSimType::PhotonBombs;

    BombSet.clear();
    DepoSet.clear();
    PhotFileSet.clear();

    WaveSet.clear();
    OptSet.clear();

    RunSet.clear();
}

// ---- Depo ----

void APhotonDepoSettings::clear()
{
    AFileSettingsBase::clear();

    Primary          = true;
    Secondary        = false;
}

void APhotonDepoSettings::doWriteToJson(QJsonObject &json) const
{
    json["Primary"]    = Primary;
    json["Secondary"]  = Secondary;
}

void APhotonDepoSettings::doReadFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "Primary",   Primary);
    jstools::parseJson(json, "Secondary", Secondary);
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
    for (int i=0; i<3; i++)
        ScanRecords[i] = APhScanRecord();
    ScanRecords[0].bEnabled = true;

    ScanRecords[1].DX = 0; ScanRecords[1].DY = 10.0;
    ScanRecords[2].DX = 0; ScanRecords[2].DZ = 10.0;
}

void AGridSettings::writeToJson(QJsonObject &json) const
{
    json["ScanX0"] = X0;
    json["ScanY0"] = Y0;
    json["ScanZ0"] = Z0;

    QJsonArray ar;
        for (int i = 0; i < 3; i++)
        {
            QJsonObject js;
                const APhScanRecord & r = ScanRecords[i];
                js["Enabled"]   = r.bEnabled;
                js["BiDirect"] = r.bBiDirect;
                js["Nodes"]     = r.Nodes;
                js["dX"]        = r.DX;
                js["dY"]        = r.DY;
                js["dZ"]        = r.DZ;
            ar.append(js);
        }
        json["AxesData"] = ar;
}

QString AGridSettings::readFromJson(const QJsonObject &json)
{
    clearSettings();

    jstools::parseJson(json, "ScanX0", X0);
    jstools::parseJson(json, "ScanY0", Y0);
    jstools::parseJson(json, "ScanZ0", Z0);

    QJsonArray ar;
    bool bOK = jstools::parseJson(json, "AxesData", ar);
    if (!bOK || ar.size() != 3) return "Bad format in photon sim grid settings";

    for (int i = 0; i < 3; i++)
    {
        QJsonObject js = ar[i].toObject();
        APhScanRecord & r = ScanRecords[i];
        jstools::parseJson(js, "Enabled",  r.bEnabled);
        jstools::parseJson(js, "BiDirect", r.bBiDirect);
        jstools::parseJson(js, "Nodes",    r.Nodes);
        jstools::parseJson(js, "dX",       r.DX);
        jstools::parseJson(js, "dY",       r.DY);
        jstools::parseJson(js, "dZ",       r.DZ);
    }

    ScanRecords[0].bEnabled = true;
    return "";
}

// ----

void APhotonAdvancedSettings::clear()
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

    bOnlyVolume    = false;
    Volume.clear();
    bOnlyMaterial    = false;
    Material.clear();
    MaxNodeAttempts = 1000;
}

void APhotonAdvancedSettings::writeToJson(QJsonObject & json) const
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

    // Skip bombs
    {
        QJsonObject js;
            js["EnableOnlyVol"]   = bOnlyVolume;
            js["Volume"]          = Volume;
            js["EnableOnlyMat"]   = bOnlyMaterial;
            js["Material"]        = Material;
            js["MaxNodeAttempts"] = MaxNodeAttempts;
        json["SkipBombs"] = js;
    }
}

void APhotonAdvancedSettings::readFromJson(const QJsonObject &json)
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

    // Skip bombs
    {
        QJsonObject js;
        jstools::parseJson(json, "SkipBombs", js);
        jstools::parseJson(js, "EnableOnlyVol",   bOnlyVolume);
        jstools::parseJson(js, "Volume",          Volume);
        jstools::parseJson(js, "EnableOnlyMat",   bOnlyMaterial);
        jstools::parseJson(js, "Material",        Material);
        jstools::parseJson(js, "MaxNodeAttempts", MaxNodeAttempts);
    }
}

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
