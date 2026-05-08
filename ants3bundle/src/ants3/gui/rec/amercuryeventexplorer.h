#ifndef AMERCURYEVENTEXPLORER_H
#define AMERCURYEVENTEXPLORER_H

#include <vector>

#include <QDialog>

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

    QString start(Reconstructor * reconstructor, std::vector<std::vector<double>> * events);

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

private:
    Ui::AMercuryEventExplorer * ui = nullptr;

    LRModel * Model = nullptr;
    Reconstructor * Rec = nullptr;
    std::vector<std::vector<double>> * Events = nullptr;

    bool bGood = false;

    void onEventChanged();
    void showSignals();
    void showMap();

signals:
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
};

#endif // AMERCURYEVENTEXPLORER_H
