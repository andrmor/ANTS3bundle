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

int AWaveResSettings::countNodes() const
{
    if (Step == 0) return 1;
    return (To - From) / Step + 1;
}

double AWaveResSettings::toWavelength(int index) const
{
    return From + Step * index;
}

int AWaveResSettings::toIndex(double wavelength) const
{
    if (!Enabled) return -1;

    int iwave = round( (wavelength - From) / Step );

    if (iwave < 0) iwave = 0;
    const int numNodes = countNodes();
    if (iwave >= numNodes) iwave = numNodes - 1;
    return iwave;
}

int AWaveResSettings::toIndexFast(double wavelength) const
{
    return (wavelength - From) / Step;
}

void AWaveResSettings::toStandardBins(const QVector<double>* sp_x, const QVector<double>* sp_y, QVector<double>* y) const
{
    y->clear();

    double xx, yy;
    const int points = countNodes();
    for (int iP = 0; iP < points; iP++)
    {
        xx = From + Step * iP;
        if (xx <= sp_x->at(0)) yy = sp_y->at(0);
        else
        {
            if (xx >= sp_x->at(sp_x->size()-1)) yy = sp_y->at(sp_x->size()-1);
            else
            {
                //general case
                yy = getInterpolatedValue(xx, sp_x, sp_y); //reusing interpolation function
                if (yy<0) yy = 0; // !!!*** is it needed?
            }
        }
        y->append(yy);
    }
}

double AWaveResSettings::getInterpolatedValue(double val, const QVector<double> *X, const QVector<double> *F) const
{
    if (val < X->first())
    {
        //  qWarning()<<"Interpolation: value is out of the data range:"<<val<< " < " << X->first();
        return F->first();
    }
    if (val > X->last())
    {
        //  qWarning()<<"Interpolation: value is out of the data range:"<<val<< " > " << X->last();
        return F->last();
    }

    QVector<double>::const_iterator it;
    it = std::lower_bound(X->begin(), X->end(), val);
    int index = X->indexOf(*it);
    //      qDebug()<<"energy:"<<energy<<"index"<<index;//<<*it;
    if (index < 1)
    {
        //qWarning()<<"Interpolation: value out (or on the border) of the interaction data range!";
        return F->first();
    }

    double Less = F->at(index-1);
    double More = F->at(index);
    //      qDebug()<<" Less/More"<<Less<<More;
    double EnergyLess = X->at(index-1);
    double EnergyMore = X->at(index);
    //      qDebug()<<" Energy Less/More"<<EnergyLess<<EnergyMore;

    double InterpolationValue;
    if (EnergyLess == EnergyMore) InterpolationValue = More;
    else                          InterpolationValue = Less + (More-Less)*(val-EnergyLess)/(EnergyMore-EnergyLess);
    //      qDebug()<<"energy / interValue"<<energy<<InteractValue;
    return InterpolationValue;
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
        for (const ADPair & pair : CustomDist)
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
        double x = el[0].toDouble();
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

// ---

void APhotSimRunSettings::writeToJson(QJsonObject & json) const
{
    json["Seed"]                  = Seed;

    json["EventFrom"]             = EventFrom;
    json["EventTo"]               = EventTo;

    json["OutputDirectory"]       = OutputDirectory;

    json["SaveSensorSignals"]     = SaveSensorSignals;
    json["FileNameSensorSignals"] = FileNameSensorSignals;

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

    /*
    json["SavePhotonLog"]         = SavePhotonLog;
    json["FileNamePhotonLog"]     = FileNamePhotonLog;
    json["UpperTimeLimit"]        = UpperTimeLimit;
    */
}

void APhotSimRunSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Seed",                  Seed);

    jstools::parseJson(json, "EventFrom",             EventFrom);
    jstools::parseJson(json, "EventTo",               EventTo);

    jstools::parseJson(json, "OutputDirectory",       OutputDirectory);

    jstools::parseJson(json, "SaveSensorSignals",     SaveSensorSignals);
    jstools::parseJson(json, "FileNameSensorSignals", FileNameSensorSignals);

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
}

// ---

void APhotonSimSettings::writeToJson(QJsonObject & json) const
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
        RunSet.writeToJson(js);
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
    WaveIndex  = -1;

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
            js["DirDX"] = DirDX;
            js["DirDY"] = DirDY;
            js["DirDZ"] = DirDZ;
            js["ConeAngle"] = ConeAngle;
        json["Direction"] = js;
    }

    // Wave index
    {
        QJsonObject js;
            js["Enabled"]   = bFixWave;
            js["WaveIndex"] = WaveIndex;
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

        jstools::parseJson(js, "DirDX",     DirDX);
        jstools::parseJson(js, "DirDY",     DirDY);
        jstools::parseJson(js, "DirDZ",     DirDZ);
        jstools::parseJson(js, "ConeAngle", ConeAngle);
    }

    // Wave index
    {
        QJsonObject js;
        jstools::parseJson(json, "Wave", js);
        jstools::parseJson(js, "Enabled",   bFixWave);
        jstools::parseJson(js, "WaveIndex", WaveIndex);
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
