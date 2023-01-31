#include "ahist_si.h"
#include "ascriptobjstore.h"
#include "arootobjcollection.h"
#include "arootgraphrecord.h"
#include "aroothistrecord.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

#include "TObject.h"
#include "TH1D.h"
#include "TH1.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH2.h"
#include "TF2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TF1.h"
#include "TFile.h"
#include "TKey.h"

//----------------- HIST  -----------------
AHist_SI::AHist_SI()
    : Hists(AScriptObjStore::getInstance().Hists)
{
    Description = "CERN ROOT histograms";

    Help["FitGauss"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
                    "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
    Help["FitGaussWithInit"] = "Fit histogram with a Gaussian. The returned result (is successful) contains an array [Constant,Mean,Sigma,ErrConstant,ErrMean,ErrSigma]"
                            "\nInitialParValues is an array of initial parameters of the values [Constant,Mean,Sigma]"
                            "\nOptional 'options' parameter is directly forwarded to TH1::Fit()";
}

//AHist_SI::AHist_SI(const AHist_SI &other) :
//    AScriptInterface(other),
//    TmpHub(other.TmpHub) {}

void AHist_SI::new1D(QString histName, int bins, double start, double stop)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH1D* hist = new TH1D("", histName.toLatin1().data(), bins, start, stop);
    ARootHistRecord* rec = new ARootHistRecord(hist, histName, "TH1D");

    bool bOK = Hists.append(histName, rec, AbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + histName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::new2D(QString histName, int binsX, double startX, double stopX,  int binsY, double startY, double stopY)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH2D* hist = new TH2D("", histName.toLatin1().data(), binsX, startX, stopX, binsY, startY, stopY);
    ARootHistRecord* rec = new ARootHistRecord(hist, histName, "TH2D");

    bool bOK = Hists.append(histName, rec, AbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + histName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::new3D(QString histName, int binsX, double startX, double stopX, int binsY, double startY, double stopY, int binsZ, double startZ, double stopZ)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    TH3D * hist = new TH3D("", histName.toLatin1().data(), binsX, startX, stopX, binsY, startY, stopY, binsZ, startZ, stopZ);
    ARootHistRecord* rec = new ARootHistRecord(hist, histName, "TH3D");

    bool bOK = Hists.append(histName, rec, AbortIfExists);
    if (!bOK)
    {
        delete rec;
        abort("Histogram " + histName+" already exists!");
    }
    else
    {
        hist->GetYaxis()->SetTitleOffset(1.30f);
    }
}

void AHist_SI::setXCustomLabels(QString histName, QVariantList textLabels)
{
    if (!bGuiThread)
    {
        abort("Cannot perform this operation in non-GUI thread!");
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else
    {
        std::vector<QString> lab;
        for (int i = 0; i < textLabels.size(); i++)
            lab.push_back( textLabels[i].toString() );
        r->setXLabels(lab);
    }
}

void AHist_SI::setTitle(QString histName, QString title)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setTitle(title);
}

void AHist_SI::setAxisTitles(QString histName, QString x_Title, QString y_Title, QString z_Title)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setAxisTitles(x_Title, y_Title, z_Title);
}

void AHist_SI::setLineProperties(QString histName, int lineColor, int lineStyle, int lineWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setLineProperties(lineColor, lineStyle, lineWidth);
}

void AHist_SI::setMarkerProperties(QString histName, int markerColor, int markerStyle, double markerSize)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setMarkerProperties(markerColor, markerStyle, markerSize);
}

void AHist_SI::setFillColor(QString histName, int color)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setFillColor(color);
}

void AHist_SI::setMaximum(QString histName, double max)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setMax(max);
}

void AHist_SI::setMinimum(QString histName, double min)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setMin(min);
}

double AHist_SI::getMaximum(QString histName)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return 0;
    }
    return r->getMaximum();
}

double AHist_SI::getMinimum(QString histName)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return 0;
    }
    return r->getMinimum();
}

void AHist_SI::setXDivisions(QString histName, int primary, int secondary, int tertiary, bool canOptimize)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setXDivisions(primary, secondary, tertiary, canOptimize);
}

void AHist_SI::setYDivisions(QString histName, int primary, int secondary, int tertiary, bool canOptimize)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setYDivisions(primary, secondary, tertiary, canOptimize);
}

void AHist_SI::setXLabelProperties(QString histName, double size, double offset)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setXLabelProperties(size, offset);
}

void AHist_SI::setYLabelProperties(QString histName, double size, double offset)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->setYLabelProperties(size, offset);
}

void AHist_SI::fill(QString histName, double val, double weight)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r || r->getType() != "TH1D")
        abort("1D histogram " + histName + " not found!");
    else
        r->fill1D(val, weight);
}

void AHist_SI::fill(QString histName, double x, double y, double weight)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r || r->getType() != "TH2D")
        abort("2D histogram " + histName + " not found!");
    else
        r->fill2D(x, y, weight);
}

void AHist_SI::fill(QString histName, double x, double y, double z, double weight)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r || r->getType() != "TH3D")
        abort("3D histogram " + histName + " not found!");
    else
        r->fill3D(x, y, z, weight);
}

void AHist_SI::fillArr(QString histName, QVariantList array)
{
    if (array.empty()) return;

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) return;

    const int numPoints = array.size();
    const QVariantList first = array.front().toList();
    const int numDim = first.size();

    if (r->getType() == "TH1D")
    {
        std::vector<double> x(numPoints), w(numPoints);
        if (numDim == 0)
        {
            bool ok;
            for (int i = 0; i < numPoints; i++)
            {
                x[i] = array[i].toDouble(&ok);
                w[i] = 1.0;
                if (!ok)
                {
                    abort("Format error in 1D array for 1D histogram");
                    return;
                }
            }
        }
        else if (numDim == 2)
        {
            bool ok1, ok2;
            for (int i = 0; i < numPoints; i++)
            {
                const QVariantList element = array[i].toList();
                if (element.size() != 2)
                {
                    abort("Format error in 2D array for 1D histogram");
                    return;
                }
                x[i] = element[0].toDouble(&ok1);
                w[i] = element[1].toDouble(&ok2);
                if (!ok1 || !ok2)
                {
                    abort("Format error in 2D array for 1D histogram");
                    return;
                }
            }
        }
        else
        {
            abort("fillArr method for 1D histogram should have as the second argument an array of values (assuming all weights=1) or an array of [x,weight] arrays");
            return;
        }
        r->fill1D(x, w);
    }
    else if (r->getType() == "TH2D")
    {
        std::vector<double> x(numPoints), y(numPoints), w(numPoints);
        if (numDim == 2)
        {
            bool ok1, ok2;
            for (int i = 0; i < numPoints; i++)
            {
                const QVariantList element = array[i].toList();
                if (element.size() != 2)
                {
                    abort("Format error in 2D array for 2D histogram");
                    return;
                }
                x[i] = element[0].toDouble(&ok1);
                y[i] = element[1].toDouble(&ok2);
                w[i] = 1.0;
                if (!ok1 || !ok2)
                {
                    abort("Format error in 2D array for 2D histogram");
                    return;
                }
            }
        }
        else if (numDim == 3)
        {
            bool ok1, ok2, ok3;
            for (int i = 0; i < numPoints; i++)
            {
                const QVariantList element = array[i].toList();
                if (element.size() != 3)
                {
                    abort("Format error in 3D array for 2D histogram");
                    return;
                }
                x[i] = element[0].toDouble(&ok1);
                y[i] = element[1].toDouble(&ok2);
                w[i] = element[2].toDouble(&ok3);
                if (!ok1 || !ok2 || !ok3)
                {
                    abort("Format error in 3D array for 2D histogram");
                    return;
                }
            }
        }
        else
        {
            abort("fillArr method for 2D histogram should have as the second argument an array of [x,y] values (assuming all weights=1) or an array of [x,y,weight] arrays");
            return;
        }
        r->fill2D(x, y, w);
    }
    else if (r->getType() == "TH3D")
    {
        std::vector<double> x(numPoints), y(numPoints), z(numPoints), w(numPoints);
        if (numDim == 3)
        {
            bool ok1, ok2, ok3;
            for (int i = 0; i < numPoints; i++)
            {
                const QVariantList element = array[i].toList();
                if (element.size() != 3)
                {
                    abort("Format error in 3D array for 3D histogram");
                    return;
                }
                x[i] = element[0].toDouble(&ok1);
                y[i] = element[1].toDouble(&ok2);
                z[i] = element[2].toDouble(&ok3);
                w[i] = 1.0;
                if (!ok1 || !ok2 || !ok3)
                {
                    abort("Format error in 3D array for 3D histogram");
                    return;
                }
            }
        }
        else if (numDim == 4)
        {
            bool ok1, ok2, ok3, ok4;
            for (int i = 0; i < numPoints; i++)
            {
                const QVariantList element = array[i].toList();
                if (element.size() != 3)
                {
                    abort("Format error in 4D array for 3D histogram");
                    return;
                }
                x[i] = element[0].toDouble(&ok1);
                y[i] = element[1].toDouble(&ok2);
                z[i] = element[2].toDouble(&ok3);
                w[i] = element[3].toDouble(&ok4);
                if (!ok1 || !ok2 || !ok3 || !ok4)
                {
                    abort("Format error in 4D array for 3D histogram");
                    return;
                }
            }
        }
        else
        {
            abort("fillArr method for 3D histogram should have as the second argument an array of [x,y,z] values (assuming all weights=1) or an array of [x,y,z,weight] arrays");
            return;
        }
        r->fill3D(x, y, z, w);
    }
    else abort("Histogram " + histName + " not found!");
}

void AHist_SI::fillArr(QString histName, QVariantList array1, QVariantList array2)
{
    if (array1.empty()) return;

    const int numPoints = array1.size();
    if (array2.size() != numPoints)
    {
        abort("fillArr: mismatch in the array sizes for histogram " + histName);
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) return;

    if (r->getType() == "TH1D")
    {
        std::vector<double> x(numPoints), w(numPoints);
        bool ok1, ok2;
        for (int i = 0; i < numPoints; i++)
        {
            x[i] = array1[i].toDouble(&ok1);
            w[i] = array2[i].toDouble(&ok2);
            if (!ok1 || !ok2)
            {
                if (!ok1) abort("fillArr: Format error in first array for histogram " + histName);
                else      abort("fillArr: Format error in second array for histogram " + histName);
                return;
            }
        }
        r->fill1D(x, w);
    }
    else if (r->getType() == "TH2D")
    {
        std::vector<double> x(numPoints), y(numPoints), w(numPoints);
        bool ok1, ok2;
        for (int i = 0; i < numPoints; i++)
        {
            x[i] = array1[i].toDouble(&ok1);
            y[i] = array2[i].toDouble(&ok2);
            w[i] = 1.0;
            if (!ok1 || !ok2)
            {
                if (!ok1) abort("fillArr: Format error in first array for histogram " + histName);
                else      abort("fillArr: Format error in second array for histogram " + histName);
                return;
            }
        }
        r->fill2D(x, y, w);
    }
    else abort("fillArr with two array arguments: applicable only to 1D / 2D histograms (not valid for " + histName + ")");
}

void AHist_SI::fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3)
{
    if (array1.empty()) return;

    const int numPoints = array1.size();
    if (array2.size() != numPoints || array3.size() != numPoints)
    {
        abort("fillArr: mismatch in the array sizes for histogram " + histName);
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) return;

    if (r->getType() == "TH2D")
    {
        std::vector<double> x(numPoints), y(numPoints), w(numPoints);
        bool ok1, ok2, ok3;
        for (int i = 0; i < numPoints; i++)
        {
            x[i] = array1[i].toDouble(&ok1);
            y[i] = array2[i].toDouble(&ok2);
            w[i] = array3[i].toDouble(&ok3);
            if (!ok1 || !ok2 || !ok3)
            {
                if      (!ok1) abort("fillArr: Format error in first array for histogram " + histName);
                else if (!ok2) abort("fillArr: Format error in second array for histogram " + histName);
                else           abort("fillArr: Format error in third array for histogram " + histName);
                return;
            }
        }
        r->fill2D(x, y, w);
    }
    else if (r->getType() == "TH3D")
    {
        std::vector<double> x(numPoints), y(numPoints), z(numPoints), w(numPoints);
        bool ok1, ok2, ok3;
        for (int i = 0; i < numPoints; i++)
        {
            x[i] = array1[i].toDouble(&ok1);
            y[i] = array2[i].toDouble(&ok2);
            z[i] = array3[i].toDouble(&ok3);
            w[i] = 1.0;
            if (!ok1 || !ok2 || !ok3)
            {
                if      (!ok1) abort("fillArr: Format error in first array for histogram " + histName);
                else if (!ok2) abort("fillArr: Format error in second array for histogram " + histName);
                else           abort("fillArr: Format error in third array for histogram " + histName);
                return;
            }
        }
        r->fill3D(x, y, z, w);
    }
    else abort("fillArr with three array arguments: applicable only to 2D / 3D histograms (not valid for " + histName + ")");
}

void AHist_SI::fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3, QVariantList array4)
{
    if (array1.empty()) return;

    const int numPoints = array1.size();
    if (array2.size() != numPoints || array3.size() != numPoints || array4.size() != numPoints)
    {
        abort("fillArr: mismatch in the array sizes for histogram " + histName);
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) return;

    if (r->getType() == "TH3D")
    {
        std::vector<double> x(numPoints), y(numPoints), z(numPoints), w(numPoints);
        bool ok1, ok2, ok3, ok4;
        for (int i = 0; i < numPoints; i++)
        {
            x[i] = array1[i].toDouble(&ok1);
            y[i] = array2[i].toDouble(&ok2);
            z[i] = array3[i].toDouble(&ok3);
            w[i] = array4[i].toDouble(&ok4);
            if (!ok1 || !ok2 || !ok3 || !ok4)
            {
                if      (!ok1) abort("fillArr: Format error in first array for histogram " + histName);
                else if (!ok2) abort("fillArr: Format error in second array for histogram " + histName);
                else if (!ok3) abort("fillArr: Format error in third array for histogram " + histName);
                else           abort("fillArr: Format error in forth array for histogram " + histName);
                return;
            }
        }
        r->fill3D(x, y, z, w);
    }
    else abort("fillArr with four array arguments: applicable only to 3D histograms (not valid for " + histName + ")");
}

void AHist_SI::smooth(QString histName, int times)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else
    {
        r->smooth(times);
        if (bGuiThread) emit requestDraw(0, "", true); //to update
    }
}

void AHist_SI::smear(QString histName, double sigma)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else
    {
        if (r->getType() != "TH1D")
        {
            abort("Smear is implemented only for TH1D");
            return;
        }
        r->smear(sigma);
        if (bGuiThread) emit requestDraw(0, "", true); //to update
    }
}

void AHist_SI::applyMedianFilter(QString histName, int span)
{
    applyMedianFilter(histName, span, -1);
}

void AHist_SI::applyMedianFilter(QString histName, int spanLeft, int spanRight)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
        abort("Histogram " + histName + " not found!");
    else
    {
        bool ok = r->medianFilter(spanLeft, spanRight);
        if (!ok) abort("Failed - median filter is currently implemented only for 1D histograms (TH1)");
    }
}

void AHist_SI::divideByHistogram(QString histName, QString histToDivideWith)
{
    ARootHistRecord * r1 = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    ARootHistRecord * r2 = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histToDivideWith));
    if (!r1)
    {
        abort("Histogram " + histName + " not found!");
        return;
    }
    if (!r2)
    {
        abort("Histogram " + histToDivideWith + " not found!");
        return;
    }
    else
    {
        bool bOK = r1->divide(r2);
        if (!bOK) abort("Histogram division failed: " + histName + " by " + histToDivideWith);
    }
}

QVariantList AHist_SI::fitGauss(QString histName, QString options)
{
    QVariantList vl;
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return vl;
    }

    if (!r->getType().startsWith("TH1"))
    {
        abort("Gaussian fit is implemented only for 1D histograms");
        return vl;
    }

    std::vector<double> vec = r->fitGauss(options);

    if (!vec.empty() && bGuiThread) emit requestDraw(0, "", true); //to update

    for (double d : vec) vl << d;
    return vl;
}

QVariantList AHist_SI::fitGaussWithInit(QString histName, QVariantList initialParameterValues, QString options)
{
    QVariantList vl;
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return vl;
    }

    std::vector<double> in(3);
    if (initialParameterValues.size() != 3)
    {
        abort("fitGaussWithInit: initialParameterValues argument has to be an array of Scaling, Mean and Sigma values");
        return vl;
    }
    bool ok;
    for (int i = 0; i < 3; i++)
    {
        in[i] = initialParameterValues[i].toDouble(&ok);
        if (!ok)
        {
            abort("fitGaussWithInit: initialParameterValues argument has to be an array of Scaling, Mean and Sigma values");
            return vl;
        }
    }

    std::vector<double> vec = r->fitGaussWithInit(in, options);

    if (!vec.empty() && bGuiThread) emit requestDraw(0, "", true); //to update

    for (double d : vec) vl << d;
    return vl;
}

QVariantList AHist_SI::findPeaks(QString histName, double sigma, double threshold)
{
    QVariantList vl;

    if (threshold <= 0 || threshold >= 1.0)
    {
        abort("hist.FindPeaks() : Threshold should be larger than 0 and less than 1.0");
        return vl;
    }
    if (sigma <= 0)
    {
        abort("hist.FindPeaks() : Sigma should be positive");
        return vl;
    }
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return vl;
    }

    const std::vector<double> vec = r->findPeaks(sigma, threshold);
    for (const double & d : vec) vl << d;
    return vl;
}

int AHist_SI::getNumberEntries(QString histName)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return 1.0;
    }
    return r->getEntries();
}

void AHist_SI::setNumberEntries(QString histName, int numEntries)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else r->setEntries(numEntries);
}

double AHist_SI::getIntegral(QString histName, bool multiplyByBinWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return 1.0;
    }
    else return r->getIntegral(multiplyByBinWidth);
}

double AHist_SI::getRandom(QString histName)
{
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return 0;
    }
    return r->GetRandom();
}

QVariantList AHist_SI::getRandomArray(QString histName, int numRandoms)
{
    QVariantList vl;

    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return vl;
    }

    std::vector<double> vec = r->GetRandomMultiple(numRandoms);
    for (const double & d : vec) vl.append(d);
    return vl;
}

QVariantList AHist_SI::getStatistics(QString histName)
{
    QVariantList vl;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
        abort("Histogram " + histName + " not found!");
    else
    {
        int num;
        std::vector<double> mean = {0, 0};
        std::vector<double> std = {0, 0};
        r->GetStatistics(num, mean, std);
        if (r->getType() == "TH1D")
            vl << num << mean[0] << std[0];
        else
        {
            vl << num;
            QVariantList m; m << mean[0] << mean[1]; vl.push_back(m);
            QVariantList s; s << std[0]  << std[1];  vl.push_back(s);
        }
    }
    return vl;
}

void AHist_SI::scaleIntegral(QString histName, double scaleIntegralTo, bool dividedByBinWidth)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->Scale(scaleIntegralTo, dividedByBinWidth);
}

void AHist_SI::save(QString histName, QString fileName)
{
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else    r->save(fileName);
}

void AHist_SI::load(QString histName, QString fileName, QString histNameInFile)
{
    if (!bGuiThread)
    {
        abort("Threads cannot load histograms!");
        return;
    }

    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (r && AbortIfExists)
    {
        abort("Histogram " + histName + " already exists!");
        return;
    }

    TFile* f = new TFile(fileName.toLatin1());
    if (!f)
    {
        abort("File not found or cannot be opened: " + fileName);
        return;
    }

    const int numKeys = f->GetListOfKeys()->GetEntries();
    qDebug() << "File contains" << numKeys << "TKeys";

    ARootHistRecord * rec = nullptr;
    bool bFound = false;
    for (int i=0; i<numKeys; i++)
    {
        TKey *key = (TKey*)f->GetListOfKeys()->At(i);
        QString Type = key->GetClassName();
        QString Name = key->GetName();
        qDebug() << i << Type << Name;

        if (!histNameInFile.isEmpty() && Name != histNameInFile) continue;
        bFound = true;

        if (Type == "TH1D")
        {
            TH1D * hist = (TH1D*)key->ReadObj();
            rec = new ARootHistRecord(hist, histName, "TH1D");
            hist->GetYaxis()->SetTitleOffset(1.30f);
            break;
        }
        //else if (Type=="TProfile") p = (TProfile*)key->ReadObj();
        //else if (Type=="TProfile2D") p = (TProfile2D*)key->ReadObj();
        else if (Type == "TH2D")
        {
            TH2D * hist = (TH2D*)key->ReadObj();
            rec = new ARootHistRecord(hist, histName, "TH2D");
            //hist->GetYaxis()->SetTitleOffset(1.30f);
            break;
        }
    }
    f->Close();
    delete f;

    if (!rec)
    {
        if (!histNameInFile.isEmpty() && !bFound)
            abort("Histogram with name " + histNameInFile + " not found in file " + fileName);
        else
            abort("Error loading histogram.\nNote that currently supported histogram types are TH1D and TH2D");
    }
    else
    {
        bool bOK = Hists.append(histName, rec, false);
        if (!bOK)
        {
            delete rec;
            abort("Load histogram from file " + fileName + " failed!");
        }
    }
}

bool AHist_SI::remove(QString histName)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return false;
    }

    return Hists.remove(histName);
}

void AHist_SI::removeAll()
{
    if (!bGuiThread) abort("Threads cannot create/delete/draw histograms!");
    else             Hists.clear();
}

void AHist_SI::draw(QString HistName, QString options)
{
    if (!bGuiThread)
    {
        abort("Threads cannot create/delete/draw histograms!");
        return;
    }

    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(HistName));
    if (!r)
        abort("Histogram " + HistName + " not found!");
    else
    {
        if (options.isEmpty())
        {
            if      (r->getType() == "TH1D") options = "hist";
            else if (r->getType() == "TH2D") options = "colz";
            else                             options = ""; // !!!*** can be better?
        }

        TObject * copy = r->GetObject()->Clone(r->GetObject()->GetName());
        emit requestDraw(copy, options, true);
    }
}

QVariantList AHist_SI::getContent(QString histName)
{
    QVariantList vl;
    ARootHistRecord* r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r)
    {
        abort("Histogram " + histName + " not found!");
        return vl;
    }

    if (r->is1D())
    {
        std::vector<double> x, w;
        bool ok = r->getContent1D(x, w);
        if (ok)
        {
            for (size_t i = 0; i < x.size(); i++)
            {
                QVariantList el;
                el << x[i] << w[i];
                vl.push_back(el);
            }
        }
    }
    else if (r->is2D())
    {
        std::vector<double> x, y, w;
        bool ok = r->getContent2D(x, y, w);
        if (ok)
        {
            for (size_t i = 0; i < x.size(); i++)
            {
                QVariantList el;
                el << x[i] << y[i] << w[i];
                vl.push_back(el);
            }
        }
    }
    else if (r->is3D())
    {
        std::vector<double> x, y, z, w;
        bool ok = r->getContent3D(x, y, z, w);
        if (ok)
        {
            for (size_t i = 0; i < x.size(); i++)
            {
                QVariantList el;
                el << x[i] << y[i] << z[i] << w[i];
                vl.push_back(el);
            }
        }
    }
    else abort("getContent method is currently implemented only for TH1D, TH2D and TH3D histograms");

    return vl;
}

double AHist_SI::getNumberUnderflows(QString histName)
{
    double val = 0;
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else
    {
        if (!r->getUnderflow(val))
            abort("Failed to get undeflow - the method is curretly implemented only for TH1");
    }
    return val;
}

double AHist_SI::getNumberOverflows(QString histName)
{
    double val = 0;
    ARootHistRecord * r = dynamic_cast<ARootHistRecord*>(Hists.getRecord(histName));
    if (!r) abort("Histogram " + histName + " not found!");
    else
    {
        if (!r->getOverflow(val))
            abort("Failed to get overflow - the method is curretly implemented only for TH1");
    }
    return val;
}

