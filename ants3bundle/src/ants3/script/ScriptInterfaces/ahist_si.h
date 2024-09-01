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
    void           new1D(QString histName, QVariantList binStartArray);
    void           new2D(QString histName, int binsX, double startX, double stopX,
                                           int binsY, double startY, double stopY);
    void           new2D(QString histName, QVariantList xBinStartArray, QVariantList yBinStartArray);
    void           new3D(QString histName, int binsX, double startX, double stopX,
                                           int binsY, double startY, double stopY,
                                           int binsZ, double startZ, double stopZ);

    void           clone(QString histName, QString cloneName);

    void           fill(QString histName, double val);
    void           fill(QString histName, double val, double weight);
    void           fill(QString histName, double x, double y, double weight);
    void           fill(QString histName, double x, double y, double z, double weight);

    void           fillArr(QString histName, QVariantList array);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3);
    void           fillArr(QString histName, QVariantList array1, QVariantList array2, QVariantList array3, QVariantList array4);

    void           draw(QString HistName);
    void           draw(QString HistName, QString options);

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

    QVariantList   getData(QString histName);
    QVariantList   getStatistics(QString histName); // num mean std, for 2D mean and std are vectors of [x,y]
    int            getNumberEntries(QString histName);
    void           setNumberEntries(QString histName, int numEntries);
    double         getNumberUnderflows(QString histName); // !!!*** 2D
    double         getNumberOverflows(QString histName);  // !!!*** 2D
    double         getIntegral(QString histName);
    double         getIntegral_multiplyByBinWidth(QString histName);

    double         getRandom(QString histName);
    QVariantList   getRandom(QString histName, int numRandoms);

    void           scaleIntegralTo(QString histName, double scaleIntegralTo, bool dividedByBinWidth = false);
    void           scaleMaxTo(QString histName, double max);

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
