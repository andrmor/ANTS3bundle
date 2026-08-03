#ifndef ALRFPLOTTERDIALOG_H
#define ALRFPLOTTERDIALOG_H

#include <QDialog>

#include <vector>
#include <array>

namespace Ui {
class ALrfPlotterDialog;
}

class ALrfPlotter;

class ALrfPlotterDialog : public QDialog
{
    Q_OBJECT

public:
    ALrfPlotterDialog(QWidget * parent = nullptr);
    ~ALrfPlotterDialog();

    void setPlotter(ALrfPlotter * plotter);

    void redraw();
    void start();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_pbClose_clicked();
    void on_pbRedraw_clicked();

    void on_sbSensor_editingFinished();

    void on_pbPrevious_clicked();
    void on_pbNext_clicked();

    void on_cbData_clicked(bool checked);
    void on_cbDiff_clicked(bool checked);

    void on_cobPlotType_currentIndexChanged(int index);

private:
    ALrfPlotter           * Plotter;
    Ui::ALrfPlotterDialog * ui = nullptr;

    bool HaveData = false;

    bool    storedFix = false;
    QString storedMin = "0";
    QString storedMax = "100";
    int     storedBins = 100;

private:
    void makeRadialPlot(int iSens);
    void makeXYPlot(int iSens);
    void updateVisibilityAndStatus();
};

#endif // ALRFPLOTTERDIALOG_H
