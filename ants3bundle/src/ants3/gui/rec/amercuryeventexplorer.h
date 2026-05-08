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
    AMercuryEventExplorer(Reconstructor * reconstructor, std::vector<std::vector<double>> * events, QWidget * parent = nullptr);
    ~AMercuryEventExplorer();

private slots:
    void on_pbPrevious_clicked();
    void on_pbNext_clicked();
    void on_sbEvent_editingFinished();

    void on_pbSignalVsModel_pressed();
    void on_pbMap_pressed();

private:
    Ui::AMercuryEventExplorer * ui = nullptr;
    Reconstructor * Rec = nullptr;
    std::vector<std::vector<double>> * Events;

    LRModel * Model = nullptr;

    bool bGood = false;

    void onEventChanged();
    void showSignals();

signals:
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);
};

#endif // AMERCURYEVENTEXPLORER_H
