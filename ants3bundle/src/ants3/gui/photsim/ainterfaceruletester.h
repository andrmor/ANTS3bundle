#ifndef AINTERFACERULETESTER_H
#define AINTERFACERULETESTER_H

#include <QMainWindow>

#include <vector>
#include <array>

#include "TVector3.h"

namespace Ui {
class AInterfaceRuleTester;
}

struct ATmpTrackRec
{
    ATmpTrackRec(int type, short color) : Type(type), Color(color) {}

    int   Type;
    short Color;
    std::vector<std::array<double,3>> Nodes;
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
class AGeometryHub;
class APhotonStatistics;
class ARandomHub;
class QJsonObject;
class TObject;
class APhoton;

class AInterfaceRuleTester : public QMainWindow
{
    Q_OBJECT

public:
    explicit AInterfaceRuleTester(AInterfaceRule * ovLocal, int matFrom, int matTo, QWidget * parent = nullptr);
    ~AInterfaceRuleTester();

    void writeToJson(QJsonObject& json) const;
    void readFromJson(const QJsonObject& json);

public slots:
    void updateGUI();
    void showGeometry();

private slots:
    void on_pbST_RvsAngle_clicked();
    void on_pbTracePhotons_clicked();
    void on_pbST_showTracks_clicked();
    void on_pbST_uniform_clicked();

    void on_cbWavelength_toggled(bool checked);
    void on_ledST_wave_editingFinished();
    void on_ledAngle_editingFinished();

protected:
    void closeEvent(QCloseEvent * e);

private:
    AMaterialHub const & MatHub;
    AGeometryHub       & GeoHub;
    ARandomHub         & RandomHub;
    APhotonStatistics  & Stats;

    AInterfaceRule * pOV;
    int MatFrom;
    int MatTo;

    Ui::AInterfaceRuleTester * ui = nullptr;

    const size_t MaxNumTracks = 1000;

    std::vector<ATmpTrackRec> Tracks;

    bool     testOverride();
    int      getWaveIndex();
    TVector3 getPhotonVector();
    void     reportStatistics(const AReportForOverride & rep, int numPhot);
    double   calculateReflectionProbability(const APhoton & Photon) const;

signals:
    void requestDraw(TObject * obj, const QString & options, bool transferOwnership, bool focusWindow);
    void requestDrawLegend(double x1, double y1, double x2, double y2, QString title);
    void requestClearGeometryViewer(); // also has to set current canvas to geometry view window!
    void requestShowTracks(bool activateWindow);
    void closed(bool);

};

#endif // AINTERFACERULETESTER_H
