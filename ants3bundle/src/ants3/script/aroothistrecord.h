#ifndef AROOTHISTRECORD_H
#define AROOTHISTRECORD_H

#include "arootobjbase.h"

#include <QVector>
#include <QString>
#include <QMutex>

#include <TObject.h>

#include <vector>

class ARootHistRecord : public ARootObjBase
{
public:
    ARootHistRecord(TObject* hist, const QString&  title, const QString& type);

    TObject* GetObject() override;  // unsave for multithread (draw on queued signal), only GUI thread can trigger draw

    void setTitle(const QString & title);
    void setAxisTitles(const QString & x_Title, const QString & y_Title, const QString & z_Title = "");
    void setLineProperties(int lineColor, int lineStyle, int lineWidth);
    void setMarkerProperties(int markerColor, int markerStyle, double markerSize);
    void setFillColor(int color);
    void setXLabels(const std::vector<QString> & labels);
    void setXDivisions(int primary, int secondary, int tertiary, bool canOptimize);
    void setYDivisions(int primary, int secondary, int tertiary, bool canOptimize);
    void setXLabelProperties(double size, double offset);
    void setYLabelProperties(double size, double offset);

    void fill1D(double x, double weight);
    void fill2D(double x, double y, double weight);
    void fill3D(double x, double y, double z, double weight);

    void fill1D(const std::vector<double> & x, const std::vector<double> & weight);
    void fill2D(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & weight);
    void fill3D(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & z, const std::vector<double> & weight);

    void setMax(double max);
    void setMin(double min);

    void save(const QString & fileName) const;

    bool divide(ARootHistRecord* other);

    void   smooth(int times);
    void   smear(double sigma);
    void   scaleInegralTo(double ScaleIntegralTo, bool bDividedByBinWidth = false);
    void   scaleMaxTo(double scaleMaxTo);
    bool   medianFilter(int span, int spanRight = -1);

    double getIntegral(bool bMultipliedByBinWidth = false);
    int    getEntries();
    void   GetStatistics(int & num, std::vector<double> & mean, std::vector<double> & std);
    void   setEntries(int num);
    double getMaximum() const;
    double getMinimum() const;
    bool   getContent1D(std::vector<double> & x, std::vector<double> & w) const;
    bool   getContent2D(std::vector<double> & x, std::vector<double> & y, std::vector<double> & w) const;
    bool   getContent3D(std::vector<double> & x, std::vector<double> & y, std::vector<double> & z, std::vector<double> & w) const;
    bool   getUnderflow(double & undeflow) const;
    bool   getOverflow (double & overflow) const;
    double GetRandom();
    std::vector<double> GetRandomMultiple(int numRandoms);

    bool   is1D() const;
    bool   is2D() const;
    bool   is3D() const;

    std::vector<double> fitGauss(const QString & options = "");
    std::vector<double> fitGaussWithInit(const std::vector<double> & initialParValues, const QString & options = "");
    std::vector<double> findPeaks(double sigma, double threshold);

};

#endif // AROOTHISTRECORD_H
