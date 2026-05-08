#ifndef AMERCURYEVENTEXPLORER_H
#define AMERCURYEVENTEXPLORER_H

#include <vector>

#include <QDialog>
#include <QString>

namespace Ui {
class AMercuryEventExplorer;
}

class LRModel;
class Reconstructor;
class TObject;

class AMercuryEventExplorer : public QDialog
{
    Q_OBJECT

public:
    AMercuryEventExplorer(QWidget * parent = nullptr);
    ~AMercuryEventExplorer();

    QString start(Reconstructor * reconstructor, std::vector<std::vector<double>> * events, std::vector<std::array<double, 3>> * truePositions);

    bool bFinished = false;

private slots:
    void on_pbPrevious_clicked();
    void on_pbNext_clicked();
    void on_sbEvent_editingFinished();

    void on_pbSignalVsModel_clicked(bool checked);
    void on_pbMap_clicked(bool checked);

    void on_pbUpdateMap_clicked();

    void on_pbClose_clicked();

    void on_cbSymmetricXY_clicked(bool checked);

    void on_ledXrange_editingFinished();

    void on_sbXbins_editingFinished();

    void on_cobMapCenter_currentIndexChanged(int index);

    void on_leMapRootOption_editingFinished();

    void on_cobMapHow_activated(int index);

private:
    Ui::AMercuryEventExplorer * ui = nullptr;

    LRModel       * Model = nullptr;
    Reconstructor * Rec   = nullptr;
    std::vector<std::vector<double>>   * Events        = nullptr;
    std::vector<std::array<double, 3>> * TruePositions = nullptr;

    bool bGood = false;

    QString MapRootOption_2D = "colz";
    QString MapRootOption_3D = "box3";

    void onEventChanged();
    void showSignals();
    void showMap();

signals:
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
};

#endif // AMERCURYEVENTEXPLORER_H
