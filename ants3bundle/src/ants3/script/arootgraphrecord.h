#ifndef AROOTGRAPHCOLLECTION_H
#define AROOTGRAPHCOLLECTION_H

#include "arootobjbase.h"

#include <QPair>
#include <QString>
#include <QMutex>
#include <TObject.h>

#include <vector>

class ARootGraphRecord : public ARootObjBase
{
public:
    ARootGraphRecord(TObject* graph, const QString& name, const QString& type);

    TObject* GetObject() override;  // unasve for multithread (draw on queued signal), only GUI thread can trigger draw

    void     setMarkerProperties(int markerColor, int markerStyle, double markerSize);
    void     setLineProperties(int lineColor, int lineStyle, int lineWidth);

    void     setTitle(QString graphTitle);
    void     setAxisTitles(const QString & titleX, const QString & titleY);

    // Protected by Mutex
    void     addPoint(double x, double y, double errorX = 0, double errorY = 0);
    void     addPoint2D(double x, double y, double z);
    EStatus  addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr);
    EStatus  addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr, const std::vector<double> & xErrArr, const std::vector<double> & yErrArr);
    EStatus  addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr, const std::vector<double> & zArr);
    void     sort();
    void     setYRange(double min, double max);
    void     setMinimum(double min);
    void     setMaximum(double max);
    void     setXRange(double min, double max);
    void     setXDivisions(int numDiv);
    void     setYDivisions(int numDiv);

    void     getData(std::vector<double> & x, std::vector<double> & y, std::vector<double> & z,
                     std::vector<double> & errx, std::vector<double> & erry);

    void     exportRoot(const QString & fileName);

    QString  LastDrawOption;

private:
    QString  TitleX, TitleY;
    int      MarkerColor = 4, MarkerStyle = 20;
    double   MarkerSize = 1.0;
    int      LineColor = 4,   LineStyle = 1,    LineWidth = 1;
};

#endif // AROOTGRAPHCOLLECTION_H
