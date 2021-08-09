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

void APhotonBombsSettings::writeToJson(QJsonObject & json) const
{
    // PhotonNumberMode
    {
        QString str;
        switch (PhotonNumberMode)
        {
        case EBombPhNumber::Constant : str = "constant"; break;
        case EBombPhNumber::Poisson  : str = "poisson";  break;
        case EBombPhNumber::Uniform  : str = "uniform";  break;
        case EBombPhNumber::Normal   : str = "normal";   break;
        case EBombPhNumber::Custom   : str = "custom";   break;
        }
        json["PhotonNumberMode"] = str;
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
        case EBombGen::Script : str = "script"; break;
        }
        json["GenerationMode"] = str;
    }

    // Particular generation modes
    // Single
    {
        QJsonObject js;
            QJsonArray ar;
            for (int i = 0; i < 3 ; i++) ar.push_back(Position[i]);
            js["Position"] = ar;
        json["SingleMode"] = js;
    }
    // Grid

}

QString APhotonBombsSettings::readFromJson(const QJsonObject & json)
{
    // PhotonNumberMode
    {
        QString str = "undefined";
        jstools::parseJson(json, "PhotonNumberMode", str);

        if      (str == "constant") PhotonNumberMode = EBombPhNumber::Constant;
        else if (str == "poisson")  PhotonNumberMode = EBombPhNumber::Poisson;
        else if (str == "uniform")  PhotonNumberMode = EBombPhNumber::Uniform;
        else if (str == "normal")   PhotonNumberMode = EBombPhNumber::Normal;
        else if (str == "custom")   PhotonNumberMode = EBombPhNumber::Custom;
        else
        {
            PhotonNumberMode = EBombPhNumber::Constant;
            return QString("Unknown PhotonNumberMode: %1 -> replacing with 'Constant'").arg(str);
        }
    }

    // GenerationMode
    {
        QString str;
        jstools::parseJson(json, "GenerationMode",  str);

        if      (str == "single") GenerationMode = EBombGen::Single;
        else if (str == "grid")   GenerationMode = EBombGen::Grid;
        else if (str == "flood")  GenerationMode = EBombGen::Flood;
        else if (str == "file")   GenerationMode = EBombGen::File;
        else if (str == "script") GenerationMode = EBombGen::Script;
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
        jstools::parseJson(json, "SingleMode", js);
            QJsonArray ar;
            bool ok = jstools::parseJson(js, "Position", ar);
            if (ok && ar.size() == 3)
                for (int i = 0; i < 3 ; i++) Position[i] = ar[i].toDouble();
            else
            {
                qWarning() << "Error in loading Position for Single mode";
                for (int i = 0; i < 3 ; i++) Position[i] = 0;
            }
    }
    // Grid


    return "";
}

// ---

void APhotSimRunSettings::writeToJson(QJsonObject & json) const
{
    json["EventFrom"]             = EventFrom;
    json["EventTo"]               = EventTo;

    json["SaveSensorSignals"]     = SaveSensorSignals;
    json["FileNameSensorSignals"] = FileNameSensorSignals;

    json["SavePhotonBombs"]       = SavePhotonBombs;
    json["FileNamePhotonBombs"]   = FileNamePhotonBombs;

    json["SaveTracks"]            = SaveTracks;
    json["MaxTracks"]             = MaxTracks;
    json["FileNameTracks"]        = FileNameTracks;

    json["SavePhotonLog"]         = SavePhotonLog;
    json["FileNamePhotonLog"]     = FileNamePhotonLog;
}

void APhotSimRunSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "EventFrom",             EventFrom);
    jstools::parseJson(json, "EventTo",               EventTo);

    jstools::parseJson(json, "SaveSensorSignals",     SaveSensorSignals);
    jstools::parseJson(json, "FileNameSensorSignals", FileNameSensorSignals);

    jstools::parseJson(json, "SavePhotonBombs",       SavePhotonBombs);
    jstools::parseJson(json, "FileNamePhotonBombs",   FileNamePhotonBombs);

    jstools::parseJson(json, "SaveTracks",            SaveTracks);
    jstools::parseJson(json, "MaxTracks",             MaxTracks);
    jstools::parseJson(json, "FileNameTracks",        FileNameTracks);
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
    if (!ok) return "Json does not contain photon sim settings!";

    // Wave
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "WaveResolved", js);
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
            return QString("Unknown photon simulation mode: %1 -> setting to 'bombs'").arg(str);
        }
    }
    // Particular modes
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "PhotonBombs", js);
        BombSet.readFromJson(js);
    }
    //Run
    {
        QJsonObject js;
        jstools::parseJson(jsSim, "Run", js);
        RunSet.readFromJson(js);
    }

    return "";
}
