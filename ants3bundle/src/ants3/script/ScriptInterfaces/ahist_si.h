#ifndef AHIST_SI_H
#define AHIST_SI_H

#include "ascriptinterface.h"

#include <QString>
#include <QVariantList>

class TObject;
class ARootObjCollection;

class AHist_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AHist_SI();
    //AHist_SI(const AHist_SI& other);
    //~AHist_SI(){}

    //bool           InitOnRun() override {}
    //bool           IsMultithreadCapable() const override {return true;}

    AScriptInterface * cloneBase() const {return new AHist_SI();}

public slots:
    void           new1D(QString histName, int bins, double start, double stop);
    void           new2D(QString histName, int binsX, double startX, double stopX,
                                           int binsY, double startY, double stopY);
    void           new3D(QString histName, int binsX, double startX, double stopX,
                                           int binsY, double startY, double stopY,
                                           int binsZ, double startZ, double stopZ);

    void           fill(QString histName, double val, double weight);
    void           fill(QString histName, double x, double y, double weight);
    void           fill(QString histName, double x, double y, double z, double weight);

    void           fillArr(QString histName, QVariantList array);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3, QVariantList array4);

    void           draw(QString HistName, QString options = "");

    void           setTitle(QString histName, QString title);
    void           setAxisTitles(QString histName, QString x_Title, QString y_Title, QString z_Title = "");

    void           setLineProperties(QString histName, int lineColor, int lineStyle, int lineWidth);
    void           setMarkerProperties(QString histName, int markerColor, int markerStyle, double markerSize);
    void           setFillColor(QString histName, int color);

    void           setMinimum(QString histName, double min);
    void           setMaximum(QString histName, double max);
    double         getMinimum(QString histName);
    double         getMaximum(QString histName);

    void           setXDivisions(QString histName, int primary, int secondary, int tertiary, bool canOptimize);
    void           setYDivisions(QString histName, int primary, int secondary, int tertiary, bool canOptimize);
    void           setXLabelProperties(QString histName, double size, double offset);
    void           setYLabelProperties(QString histName, double size, double offset);
    void           setXCustomLabels(QString histName, QVariantList textLabels);

    QVariantList   getContent(QString histName);
    int            getNumberEntries(QString histName);
    double         getNumberUnderflows(QString histName); // !!!*** 2D
    double         getNumberOverflows(QString histName);  // !!!*** 2D
    double         getIntegral(QString histName, bool multiplyByBinWidth = false);

    double         getRandom(QString histName);
    QVariantList   getRandomArray(QString histName, int numRandoms);

    QVariantList   getStatistics(QString histName); // num mean std, for 2D mean and std are vectors of [x,y]
    void           setNumberEntries(QString histName, int numEntries);

    void           scaleIntegral(QString histName, double scaleIntegralTo, bool dividedByBinWidth = false);
    void           divideByHistogram(QString histName, QString histToDivideWith);

    void           applyMedianFilter(QString histName, int span);
    void           applyMedianFilter(QString histName, int spanLeft, int spanRight);
    void           smooth(QString histName, int times);
    void           smear(QString histName, double sigma); // !!!*** not general! work on the algorithm

    QVariantList   fitGauss(QString histName, QString options = "");
    QVariantList   fitGaussWithInit(QString histName, QVariantList initialParameterValues, QString options = "");

    QVariantList   findPeaks(QString histName, double sigma, double threshold);

    void           save(QString histName, QString fileName);
    void           load(QString histName, QString fileName, QString histNameInFile = "");

    bool           remove(QString histName);
    void           removeAll();

    void           configureAbortIfAlreadyExists(bool flag) {AbortIfExists = flag;}

signals:
    void           requestDraw(TObject * obj, QString options, bool fFocus);

private:
    ARootObjCollection & Hists;

    bool           AbortIfExists = false;

};

#endif // AHIST_SI_H
