#ifndef ALRFPLOTTER_H
#define ALRFPLOTTER_H

#include <QObject>
#include <QString>

#include <vector>
#include <array>

class LRModel;
class TObject;

class ALrfPlotter : public QObject
{
    Q_OBJECT

public:
    QString drawRadial(int iSens, bool showLrf, bool showNodes, bool addData, bool differenceOption);   // returns error

    // TODO
    QString drawXY(int iSens);                       // returns error

    int countSensors() const;

    bool PlotData = false;
    std::vector<std::vector<double>>  DataSignals;
    std::vector<std::array<double,4>> DataPositions; // XYZE

    size_t NumPointsInRadialGraph = 100;
    size_t NumPointsInXYGraph = 100;

    bool   FixedVerticalMin = false;
    double VerticalMin      = 0;
    bool   FixedVerticalMax = false;
    double VerticalMax      = 100.0;

    int    VerticalNumBins  = 100;

private:
    void doDrawRadialData (int iSens, bool differenceOption);
    void doDrawRadialLrf  (int iSens, bool onTopOfData);
    void doDrawRadialNodes(int iSens);

signals:
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
};

#endif // ALRFPLOTTER_H
