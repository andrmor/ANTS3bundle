#ifndef AOPTICALOVERRIDETESTER_H
#define AOPTICALOVERRIDETESTER_H

#include <QMainWindow>

#include <vector>

#include "TVector3.h"

namespace Ui {
class AOpticalOverrideTester;
}

struct ATmpTrackRec
{

};

struct AReportForOverride
{
    double abs = 0;
    double back = 0;
    double forw = 0;
    double notTrigger = 0;
    double error = 0;

    double Bspike = 0;
    double Blobe = 0;
    double Blamb = 0;

    double waveChanged = 0;
    double timeChanged = 0;
};

class AInterfaceRule;
class AMaterialHub;
class APhotonStatistics;
class ARandomHub;
class QJsonObject;
class TObject;

class AOpticalOverrideTester : public QMainWindow
{
    Q_OBJECT

public:
    explicit AOpticalOverrideTester(AInterfaceRule ** ovLocal, int matFrom, int matTo, QWidget * parent = nullptr);
    ~AOpticalOverrideTester();

    void writeToJson(QJsonObject& json) const;
    void readFromJson(const QJsonObject& json);

public slots:
    void updateGUI();
    void showGeometry();

private slots:
    void on_pbST_RvsAngle_clicked();
    void on_pbCSMtestmany_clicked();
    void on_pbST_showTracks_clicked();
    void on_pbST_uniform_clicked();

    void on_cbWavelength_toggled(bool checked);

    void on_ledST_wave_editingFinished();

    void on_ledAngle_editingFinished();

private:
    AMaterialHub const & MatHub;
    ARandomHub         & RandomHub;
    APhotonStatistics  & Stats;

    AInterfaceRule ** pOV; // !!!*** to reference to pointer
    int MatFrom;
    int MatTo;

    Ui::AOpticalOverrideTester * ui = nullptr;

    const int maxNumTracks = 1000;

    std::vector<ATmpTrackRec> Tracks;

    bool testOverride();
    int getWaveIndex();
    const TVector3 getPhotonVector();
    void reportStatistics(const AReportForOverride & rep, int numPhot);

signals:
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend();
    void requestClearGeometryViewer(); // also has to set current canvas to geometry view window!
    void requestShowTracks(); // also focuses the geo view window

};

#endif // AOPTICALOVERRIDETESTER_H
