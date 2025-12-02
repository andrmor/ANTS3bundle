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
    ALrfPlotterDialog(ALrfPlotter * plotter, QWidget * parent = nullptr);
    ~ALrfPlotterDialog();

    void triggerRedraw();

private slots:
    void on_pbClose_clicked();
    void on_pbRedraw_clicked();

    void on_sbSensor_editingFinished();

private:
    ALrfPlotter           * Plotter;
    Ui::ALrfPlotterDialog * ui = nullptr;

private:
    void makeRadialPlot();
};

#endif // ALRFPLOTTERDIALOG_H
