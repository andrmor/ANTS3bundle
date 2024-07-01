#include "amath_si.h"
#include "arandomhub.h"

#include <QDebug>
#include <QVector>

#include <cmath>

AMath_SI::AMath_SI() :
    RandomHub(ARandomHub::getInstance())
{
    Description = "Basic and advanced math";

    Help["random"] = "Return a random number between 0 and 1.\nGenerator respects the seed set by SetSeed method of the sim module!";
    Help["gauss"] = "Return a random value sampled from Gaussian distribution with the given mean and sigma";
    Help["poisson"] = "Return a random value sampled from Poisson distribution with the given mean";
    Help["maxwell"] = "Return a random value sampled from Maxwell distribution with the given Sqrt(kT/M)";
    Help["exponential"] = "Return a random value sampled from exponential decay with the given decay time";

    Help["interpolateToRegulareArray"] = "Convert an arbitrary array (can be unsorted) of double [x,y] pairs to a constant bin_size array. "
                                         "The input arguments defines the number of bins and the range (lower and upper boundaries) of the output array. "
                                         "The result is an array of [Xbin, Ybin] pairs, where Xbin is the middle of the bin and Ybin is linear-interpolated value. "
                                         "The array is sorted (increasing values of Xbin)";

    Help["fit1D"] = "Fit the array of [x,y] pairs using the provided TFormula of Cern ROOT.\n"
                  "Optional startParValues arguments can hold array of initial parameter values.\n"
                  "Returned value depends on the extendedOutput argument (false by default),\n"
                  "false: array of parameter values; true: array of [value, error] for each parameter";

    Help["getAnglesBetween3DVectors"] = "Caluculate angles in radians between two or three 3D vectors.\n"
                                        "For the case of two vectors, the method returns an array containing one element: the angle between the vectors;\n"
                                        "For the case of three vectors, the method returns an array of three angles: between 1-2, 2-3, and 3-1.";

    Help["generateDirectionIsotropic"] = "Return [Vx,Vy,Vz] unit vector sampled from isotripic distribution";
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

#include "avector.h"
QVariantList AMath_SI::getAnglesBetween3DVectors(QVariantList arrayOfVectors)
{
    QVariantList res;
    if (arrayOfVectors.length() == 2)
    {
        QVariantList vl1 = arrayOfVectors[0].toList();
        QVariantList vl2 = arrayOfVectors[1].toList();
        if (vl1.length() != 3 || vl2.length() != 3)
        {
            abort("math.getAnglesBetween3DVectors() argument must be an array of two or three 3D vectors");
            return res;
        }
        AVector3 v1, v2;
        for (int i = 0; i < 3; i++)
        {
            v1[i] = vl1[i].toDouble();
            v2[i] = vl2[i].toDouble();
        }
        res.push_back(v1.angle(v2));
    }
    else if (arrayOfVectors.length() == 3)
    {
        QVariantList vl1 = arrayOfVectors[0].toList();
        QVariantList vl2 = arrayOfVectors[1].toList();
        QVariantList vl3 = arrayOfVectors[2].toList();
        if (vl1.length() != 3 || vl2.length() != 3  || vl3.length() != 3)
        {
            abort("math.getAnglesBetween3DVectors() argument must be an array of two or three 3D vectors");
            return res;
        }
        AVector3 v1, v2, v3;
        for (int i = 0; i < 3; i++)
        {
            v1[i] = vl1[i].toDouble();
            v2[i] = vl2[i].toDouble();
            v3[i] = vl3[i].toDouble();
        }
        res.push_back(v1.angle(v2));
        res.push_back(v2.angle(v3));
        res.push_back(v3.angle(v1));
    }
    else abort("math.getAnglesBetween3DVectors() argument must be an array of two or three 3D vectors");
    return res;
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

#include "ahistogram.h"
QVariantList AMath_SI::interpolateToRegulareArray(QVariantList arrayOfPairs, int numBins, double from, double to)
{
    QVariantList res;
    if (numBins < 2)
    {
        abort("interpolateToRegulareArray(): minimum numBins is 2");
        return res;
    }
    if (to <= from)
    {
        abort("interpolateToRegulareArray(): the value of 'to' should be larger than the value of 'from'");
        return res;
    }
    const size_t arraySize = arrayOfPairs.size();
    if (arraySize < 1)
    {
        abort("interpolateToRegulareArray(): input array cannot be empty");
        return res;
    }

    std::vector<std::pair<double,double>> dist;
    dist.resize(arraySize);
    for (size_t iBin = 0; iBin < arraySize; iBin++)
    {
        const QVariantList el = arrayOfPairs[iBin].toList();
        if (el.size() != 2)
        {
            abort("interpolateToRegulareArray(): input array should contain pairs of doubles");
            return res;
        }
        dist[iBin] = {el[0].toDouble(), el[1].toDouble()};
    }

    std::sort(dist.begin(), dist.end(), [](const auto & lhs, const auto & rhs){return (lhs.first < rhs.first);});
    //qDebug() << dist;

    const double step = (to - from) / numBins;
    size_t positionInDist = 0;
    for (int iBin = 0; iBin < numBins; iBin++)
    {
        const double pos = from + iBin * step;
        while (dist[positionInDist].first < pos)
            positionInDist++;

        double val;
        if (dist[positionInDist].first == pos) val = dist[positionInDist].second; // exact match
        else
        {
            // need to interpolate

            const double interpolationFactor = (pos - dist[positionInDist-1].first) / (dist[positionInDist].first - dist[positionInDist-1].first);
            val = AHistogram1D::interpolateHere(dist[positionInDist-1].second, dist[positionInDist].second, interpolationFactor);
        }

        QVariantList thisPair;
        thisPair << pos + 0.5*step << val;
        res.push_back(thisPair);
    }
    return res;
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
