#ifndef AHIST_SI_H
#define AHIST_SI_H

#include "ascriptinterface.h"

#include <QString>
#include <QVariant>
#include <QVariantList>

class AScriptObjStore;
class TObject;

class AHist_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AHist_SI();
    //AHist_SI(const AHist_SI& other);
    //~AHist_SI(){}

    //bool           InitOnRun() override {}
    //bool           IsMultithreadCapable() const override {return true;}

public slots:
    void           new1D(QString HistName, int bins, double start, double stop);
    void           new2D(QString HistName, int binsX, double startX, double stopX,   int binsY, double startY, double stopY);
    void           new3D(QString HistName, int binsX, double startX, double stopX,
                           int binsY, double startY, double stopY,
                           int binsZ, double startZ, double stopZ);

    //void           configureAbortIfAlreadyExists(bool flag) {bAbortIfExists = flag;}

    void           fill(QString HistName, double val, double weight);
    void           fill(QString HistName, double x, double y, double weight);
    void           fill(QString HistName, double x, double y, double z, double weight);

    void           FillArr(QString HistName, QVariantList XY_Array);
    void           FillArr(QString HistName, const QVariantList X_Array, const QVariantList Y_Array);
    void           Fill2DArr(QString HistName, QVariantList arrayXYW);

    void           draw(QString HistName, QString options = "");

    void           SetXCustomLabels(const QString &HistName, QVariantList Labels);

    void           SetTitle(const QString& HistName, const QString& Title);
    void           SetTitles(const QString& HistName, QString X_Title, QString Y_Title, QString Z_Title = "");

    void           SetNumberOfEntries(const QString& HistName, int numEntries);
    void           SetLineProperties(const QString& HistName, int LineColor, int LineStyle, int LineWidth);
    void           SetMarkerProperties(const QString& HistName, int MarkerColor, int MarkerStyle, double MarkerSize);
    void           SetFillColor(const QString& HistName, int Color);

    void           setMaximum(QString HistName, double max);
    void           setMinimum(QString HistName, double min);

    void           SetXDivisions(const QString& HistName, int primary, int secondary, int tertiary, bool canOptimize);
    void           SetYDivisions(const QString& HistName, int primary, int secondary, int tertiary, bool canOptimize);
    void           SetXLabelProperties(const QString& HistName, double size, double offset);
    void           SetYLabelProperties(const QString& HistName, double size, double offset);

    void           Divide(const QString& HistName, const QString& HistToDivideWith);

    int            GetNumberOfEntries(const QString& HistName);
    QVariantList   GetContent(const QString& HistName);
    double         GetUnderflowBin(const QString& HistName);
    double         GetOverflowBin(const QString& HistName);
    double         GetIntegral(const QString& HistName, bool MultiplyByBinWidth = false);

    double         getMaximum(QString HistName);

    double         getRandom(QString HistName);
    QVariantList   getRandomMultiple(QString HistName, int numRandoms);

    QVariantList   getStatistics(QString HistName); // num mean std, for 2D mean and std are vectors of [x,y]

    void           Smooth(const QString& HistName, int times);
    void           Smear(const QString& HistName, double sigma);

    void           ApplyMedianFilter(const QString& HistName, int span);
    void           ApplyMedianFilter(const QString& HistName, int spanLeft, int spanRight);

    const QVariant FitGauss(const QString& HistName, const QString options = "");
    const QVariant FitGaussWithInit(const QString& HistName, const QVariant InitialParValues, const QString options = "");

    QVariantList   findPeaks(const QString& HistName, double sigma, double threshold);

    void           Scale(const QString& HistName, double ScaleIntegralTo, bool DividedByBinWidth = false);

    void           Save(const QString& HistName, const QString &fileName);
    void           Load(const QString& HistName, const QString &fileName, const QString histNameInFile = "");

    bool           Delete(const QString& HistName);
    void           DeleteAllHist();

signals:
    void           RequestDraw(TObject* obj, QString options, bool fFocus);

private:
    AScriptObjStore & TmpHub;

    bool           bAbortIfExists = false;

};

#endif // AHIST_SI_H
