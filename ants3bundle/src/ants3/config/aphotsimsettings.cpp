#include "aphotsimsettings.h"
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
        json["SingleMode"] = ar;
    }
    // Grid

}

void APhotonBombsSettings::readFromJson(const QJsonObject & json)
{

}

// ---

APhotSimSettings & APhotSimSettings::getInstance()
{
    static APhotSimSettings instance;
    return instance;
}

const APhotSimSettings &APhotSimSettings::getConstInstance()
{
    return getInstance();
}

void APhotSimSettings::writeToJson(QJsonObject & json) const
{
    {
        QJsonObject js;
        WaveSet.writeToJson(js);
        json["WaveResolved"] = js;
    }

    {
        QJsonObject js;
        OptSet.writeToJson(js);
        json["Optimization"] = js;
    }

}

void APhotSimSettings::readFromJson(const QJsonObject & json)
{
    {
        QJsonObject js;
        jstools::parseJson(json, "WaveResolved", js);
        WaveSet.readFromJson(js);
    }

    {
        QJsonObject js;
        jstools::parseJson(json, "Optimization", js);
        OptSet.readFromJson(js);
    }

}
