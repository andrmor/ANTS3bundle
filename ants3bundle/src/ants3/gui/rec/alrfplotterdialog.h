#ifndef ALRFPLOTTERDIALOG_H
#define ALRFPLOTTERDIALOG_H

#include <QDialog>

#include <vector>
#include <array>

namespace Ui {
class ALrfPlotterDialog;
}

class LRModel;

class ALrfPlotterDialog : public QDialog
{
    Q_OBJECT

public:
    ALrfPlotterDialog(QWidget * parent = nullptr);
    ~ALrfPlotterDialog();

    void setModel(LRModel * model);
    void setData(const std::vector<std::vector<double>> & sensSignals, const std::vector<std::array<double, 4>> & xyze);

private slots:
    void on_pbClose_clicked();
    void on_pbRedraw_clicked();

private:
    Ui::ALrfPlotterDialog * ui = nullptr;

    LRModel * Model = nullptr;

    std::vector<std::vector<double>>   Signals;
    std::vector<std::array<double, 4>> XYZE;
};

#endif // ALRFPLOTTERDIALOG_H
