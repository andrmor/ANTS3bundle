#ifndef ALRFPLOTTER_H
#define ALRFPLOTTER_H

#include <QObject>
#include <QString>

#include <vector>
#include <array>

class LRModel;
class TObject;
class LRFaxial;

class ALrfPlotter : public QObject
{
    Q_OBJECT

public:
    QString drawRadial(int iSens, bool showLrf, bool showNodes, bool addData, bool differenceOption);   // returns error
    QString drawXY(int iSens, bool showLrf, bool addData, bool differenceOption);                       // returns error

    int countSensors() const;

    std::vector<std::vector<double>>  DataSignals;
    std::vector<std::array<double,4>> DataPositions; // XYZE

    size_t NumPointsInRadialGraph = 100;
    size_t NumPointsInXYGraph = 100;

    bool   UseFixedVertical   = false;
    double VerticalMin      = 0;
    double VerticalMax      = 100.0;
    int    VerticalNumBins  = 100;

    bool   UseFixedRange = false;
    double RangeMin      = 0;
    double RangeMax      = 100.0;
    int    XDataBins     = 100;
    int    YDataBins     = 100;

    int    NumberRadialProfiles = 36;

private:
    void doDrawRadialData (int iSens, bool differenceOption);
    void doDrawRadialLrf  (int iSens, bool onTopOfData);
    void doDrawRadialNodes(int iSens);

    void doDrawXYData(int iSens);
    void doDrawXYDiff(int iSens);
    void doDrawXYLrf (int iSens, bool onTopOfData);

    void doDrawRadialForNonAxial(int iSens); // !!!*** todo: set angular step instead

    void computeRadialDataSpan(int iSens, bool differenceOption, double & xFrom, double & xTo, double & yFrom, double & yTo);

signals:
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
};

#endif // ALRFPLOTTER_H
