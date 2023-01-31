#ifndef AROOTGRAPHCOLLECTION_H
#define AROOTGRAPHCOLLECTION_H

#include "arootobjbase.h"

#include <QVector>
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
    void     addPoints(const std::vector<double> & xArr, const std::vector<double> & yArr);
    void     addPoints(const QVector<double> &xArr, const QVector<double> &yArr, const QVector<double> &xErrArr, const QVector<double> &yErrArr);
    void     sort();
    void     setYRange(double min, double max);
    void     setMinimum(double min);
    void     setMaximum(double max);
    void     setXRange(double min, double max);
    void     setXDivisions(int numDiv);
    void     setYDivisions(int numDiv);

    void     AddPoint2D(double x, double y, double z);

    const std::vector<std::pair<double, double>> GetPoints(); // !!!*** make more general

    void     exportRoot(const QString & fileName);

    QString  LastDrawOption;

private:
    QString  TitleX, TitleY;
    int      MarkerColor = 4, MarkerStyle = 20;
    double   MarkerSize = 1.0;
    int      LineColor = 4,   LineStyle = 1,    LineWidth = 1;
};

#endif // AROOTGRAPHCOLLECTION_H
