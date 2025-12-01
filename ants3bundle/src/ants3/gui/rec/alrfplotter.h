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
    QString drawRadial(int iSens, bool showNodes); // returns error
    QString drawXY(int iSens);                     // returns error

    QString drawRadial_Data(int iSens, bool addLRF);                     // returns error

    bool PlotData = false;
    std::vector<std::vector<double>>  DataSignals;
    std::vector<std::array<double,4>> DataPositions; // XYZE

    size_t NumPointsInRadialGraph = 100;
    size_t NumPointsInXYGraph = 100;

private:
    void drawDataAxial();

signals:
    void requestDraw(TObject * obj, QString options, bool fFocus);
};

#endif // ALRFPLOTTER_H
