#include "amath_si.h"
#include "arandomhub.h"

#include <QDebug>
#include <QVector>

#include <cmath>

AMath_SI::AMath_SI() :
    RandomHub(ARandomHub::getInstance())
{
    Description = "Expanded math module; implemented using std and CERN ROOT functions";

    Help["random"] = "Returns a random number between 0 and 1.\nGenerator respects the seed set by SetSeed method of the sim module!";
    Help["gauss"] = "Returns a random value sampled from Gaussian distribution with mean and sigma given by the user";
    Help["poisson"] = "Returns a random value sampled from Poisson distribution with mean given by the user";
    Help["maxwell"] = "Returns a random value sampled from maxwell distribution with Sqrt(kT/M) given by the user";
    Help["exponential"] = "Returns a random value sampled from exponential decay with decay time given by the user";

    Help["fit1D"] = "Fits the array of [x,y] points using the provided TFormula of Cern ROOT.\n"
                  "Optional startParValues arguments can hold array of initial parameter values.\n"
                  "Returned value depends on the extendedOutput argument (false by default),\n"
                  "false: array of parameter values; true: array of [value, error] for each parameter";
}

double AMath_SI::abs(double val)
{
    return std::abs(val);
}

double AMath_SI::acos(double val)
{
    return std::acos(val);
}

double AMath_SI::asin(double val)
{
    return std::asin(val);
}

double AMath_SI::atan(double val)
{
    return std::atan(val);
}

double AMath_SI::atan2(double y, double x)
{
    return std::atan2(y, x);
}

double AMath_SI::ceil(double val)
{
    return std::ceil(val);
}

double AMath_SI::cos(double val)
{
    return std::cos(val);
}

double AMath_SI::cosh(double val)
{
    return std::cosh(val);
}

double AMath_SI::sinh(double val)
{
    return std::sinh(val);
}

double AMath_SI::exp(double val)
{
    return std::exp(val);
}

double AMath_SI::floor(double val)
{
    return std::floor(val);
}

double AMath_SI::log(double val)
{
    return std::log(val);
}

double AMath_SI::log10(double val)
{
    return std::log10(val);
}

double AMath_SI::max(double val1, double val2)
{
    return std::max(val1, val2);
}

double AMath_SI::min(double val1, double val2)
{
    return std::min(val1, val2);
}

double AMath_SI::pow(double val, double power)
{
    return std::pow(val, power);
}

double AMath_SI::sin(double val)
{
    return std::sin(val);
}

double AMath_SI::sqrt(double val)
{
    return std::sqrt(val);
}

double AMath_SI::tan(double val)
{
    return std::tan(val);
}

double AMath_SI::round(double val)
{
    int f = std::floor(val);
    if (val > 0)
    {
        if (val - f < 0.5) return f;
        else return f+1;
    }
    else
    {
        if (val - f < 0.5 ) return f;
        else return f+1;
    }
}

double AMath_SI::random()
{
    return RandomHub.uniform();
}

double AMath_SI::gauss(double mean, double sigma)
{
    return RandomHub.gauss(mean, sigma);
}

double AMath_SI::poisson(double mean)
{
    return RandomHub.poisson(mean);
}

double AMath_SI::maxwell(double a)
{
    double v2 = 0;
    for (int i=0; i<3; i++)
    {
        double v = RandomHub.gauss(0, a);
        v *= v;
        v2 += v;
    }
    return std::sqrt(v2);
}

double AMath_SI::exponential(double tau)
{
    return RandomHub.exp(tau);
}

QVariantList AMath_SI::generateDirectionIsotropic()
{
    //Sphere function of CERN ROOT
    double a = 0, b = 0, r2 = 1.0;
    while (r2 > 0.25)
    {
        a  = RandomHub.uniform() - 0.5;
        b  = RandomHub.uniform() - 0.5;
        r2 = a*a + b*b;
    }
    double scale = 8.0 * sqrt(0.25 - r2);

    QVariantList v;
    v.push_back(a * scale);
    v.push_back(b * scale);
    v.push_back(-1.0 + 8.0 * r2);
    return v;
}

#include "TFormula.h"
#include "TF1.h"
#include "TGraph.h"
#include "QVariant"
#include "QVariantList"
#include "TFitResult.h"
#include "TFitResultPtr.h"
//QVariantList AMath_SI::fit1D(QVariantList array, QString tformula, QVariantList range, QVariantList startParValues, bool extendedOutput)
QVariantList AMath_SI::fit1D(QVariantList array, QString tformula, QVariantList startParValues, bool extendedOutput)
{
    QVariantList res;
    TFormula * f = new TFormula("", tformula.toLocal8Bit().data());
    if (!f || !f->IsValid())
    {
        delete f;
        abort("Cannot create TFormula");
        return res;
    }
    int numPars = f->GetNpar();
    //qDebug() << "TFormula accepted, #par = " << numPars;

    bool bParVals = false;
    QVector<double> ParValues;
    bool ok1, ok2;
    if (!startParValues.isEmpty())
    {
        if (startParValues.size() != numPars)
        {
            delete f;
            abort("Mismatch in the number of parameters for provided initial values");
            return res;
        }
        for (int i=0; i<startParValues.size(); i++)
        {
            double v = startParValues[i].toDouble(&ok1);
            if (!ok1)
            {
                delete f;
                abort("Format error in range");
                return res;
            }
            ParValues << v;
        }
        bParVals = true;
        //qDebug() << "Initial values:" << ParValues;
    }

    //bool bRange = false;
    double from = 0;
    double to = 1.0;
    /*
    if (!range.isEmpty())
    {
        if (range.size() != 2)
        {
            delete f;
            abort("Range should contain start and stop values");
            return res;
        }
        from = range[0].toDouble(&ok1);
        to   = range[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            delete f;
            abort("Format error in range");
            return res;
        }
        bRange = true;
        //qDebug() << "Fixed range:" << from << to;
    }
    */

    const int arSize = array.size();
    //qDebug() << "Data size:"<< arSize;
    if (arSize == 0)
    {
        delete f;
        abort("Array is empty!");
        return res;
    }
    QVector<double> xx; xx.reserve(arSize);
    QVector<double> yy; yy.reserve(arSize);
    //qDebug() << "Vectors are initialized";

    for (int i=0; i<arSize; i++)
    {
        QVariantList el = array[i].toList();
        if (el.size() != 2)
        {
            delete f;
            abort("array argument must contain arrays of [x,val]!");
            return res;
        }

        xx << el[0].toDouble(&ok1);
        yy << el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            delete f;
            abort("Format error in data");
            return res;
        }
    }

    TGraph g(arSize, xx.data(), yy.data());
    //qDebug() << "Graph created";

    TF1  * f1 = new TF1("f1", tformula.toLocal8Bit().data(), from, to);
    //qDebug() << "TF1 created" << f1;

    if (bParVals)
    {
        for (int i=0; i<numPars; i++) f1->SetParameter(i, ParValues[i]);
        //qDebug() << "Init par values are set!";
    }

    TString opt = "SQN"; //(bRange ? "SR" : "S"); //https://root.cern.ch/root/htmldoc/guides/users-guide/FittingHistograms.html
    TFitResultPtr fr = g.Fit(f1, opt, "");

    //qDebug() << "Fit done!";

    if ((int)fr->NTotalParameters() != numPars)
    {
        delete f1;
        delete f;
        abort("Bad number of parameters in fit result");
        return res;
    }

    if (extendedOutput)
    {
        for (int i=0; i<numPars; i++)
        {
            QVariantList el;
            el << fr->Value(i) << fr->ParError(i);
            res.push_back(el);
        }
        //res.push_back(fr->Chi2());
    }
    else
    {
        for (int i=0; i<numPars; i++) res << fr->Value(i);
    }

    delete f1;
    delete f;
    return res;
}

#include "TVirtualFFT.h"
QVariantList AMath_SI::fft(QVariantList array, int maxN)
{
    QVariantList res;

    const int arSize = array.size();

    std::vector<double> input(arSize);
    bool ok = true;
    for (int i = 0; i < arSize; i++)
    {
        input[i] = array[i].toDouble(&ok);
        if (!ok)
        {
            abort("FFT input should be an array of doubles");
            return res;
        }
    }

    int N = arSize; // does not accept const below
    TVirtualFFT * fftr2c = TVirtualFFT::FFT(1, &N, "R2C");
    fftr2c->SetPoints(input.data());

    fftr2c->Transform();

    double re, im;
    const int limN = ( maxN == -1 ? arSize : maxN );
    for (int i = 0; i < limN; i++)
    {
        if (i < arSize)
        {
            fftr2c->GetPointComplex(i, re, im);
            res.push_back(QVariantList{re, im});
        }
        else res.push_back(QVariantList{0,0});
    }

    delete fftr2c;

    return res;
}

QVariantList AMath_SI::fftMulti(QVariantList arrayOfArrays, int maxN)
{
    QVariantList res;

    int N;
    //TVirtualFFT * fftr2c = TVirtualFFT::FFT(1, &N, "R2C"); // apparently it is not reusable class :(
    std::vector<double> input;

    const int numAr = arrayOfArrays.size();
    for (int iArray = 0; iArray < numAr; iArray++)
    {
        QVariantList array = arrayOfArrays[iArray].toList();
        const int arSize = array.size();

        input.resize(arSize);
        bool ok = true;
        for (int i = 0; i < arSize; i++)
        {
            input[i] = array[i].toDouble(&ok);
            if (!ok)
            {
                abort("FFT input should be an array of doubles");
                return res;
            }
        }

        N = arSize;
        TVirtualFFT * fftr2c = TVirtualFFT::FFT(1, &N, "R2C");
        fftr2c->SetPoints(input.data());

        fftr2c->Transform();

        QVariantList thisRes;
        double re, im;
        const int limN = ( maxN == -1 ? arSize : maxN );
        for (int i = 0; i < limN; i++)
        {
            if (i < arSize)
            {
                fftr2c->GetPointComplex(i, re, im);
                thisRes.push_back(QVariantList{re, im});
            }
            else thisRes.push_back(QVariantList{0,0});
        }
        res.push_back(thisRes);
        delete fftr2c;
    }

    //delete fftr2c;
    return res;
}

#include "vformula.h"
double AMath_SI::evalFormula(QString formula, QVariantList varNames, QVariantList varValues)
{
    VFormula p1;

    std::vector<std::string> names;
    for (int i = 0; i < varNames.size(); i++) names.push_back(std::string(varNames[i].toString().toLatin1()));
    p1.setVariableNames(names);

    bool ok = p1.parse(formula.toLatin1().data());
    if (!ok)
    {
        abort("VFormula parse error!\n" + QString(p1.ErrorString.data()));
        return 0;
    }

    VFormula p(p1);

    ok = p.validate();
    if (!ok)
    {
        abort("VFormula validation error!\n" + QString(p.ErrorString.data()));
        return 0;
    }

    std::vector<double> values;
    for (int i = 0; i < varValues.size(); i++) values.push_back(varValues[i].toDouble());

    double res = p.eval(values);

    if (!p.ErrorString.empty())
    {
        abort("VFormula eval error!\n" + QString(p.ErrorString.data()));
        return 0;
    }

    return res;
}
