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

    void setPlotter(ALrfPlotter * plotter) {Plotter = plotter;}

    void redraw();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_pbClose_clicked();
    void on_pbRedraw_clicked();

    void on_sbSensor_editingFinished();

    void on_pbPrevious_clicked();

    void on_pbNext_clicked();

private:
    ALrfPlotter           * Plotter;
    Ui::ALrfPlotterDialog * ui = nullptr;

private:
    void makeRadialPlot(int iSens);
};

#endif // ALRFPLOTTERDIALOG_H
