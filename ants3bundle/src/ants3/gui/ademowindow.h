#ifndef ADEMOWINDOW_H
#define ADEMOWINDOW_H

#include <QMainWindow>

namespace Ui {
class ADemoWindow;
}

class AConfig;

class ADemoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ADemoWindow(QWidget *parent = nullptr);
    ~ADemoWindow();

private slots:
    void onProgressReceived(double progress);
    void onDemoFinsihed(bool bSuccess);

    void on_pbRun_clicked();

    void on_leFrom_editingFinished();
    void on_leTo_editingFinished();

    void on_pbAbort_clicked();

private:
    Ui::ADemoWindow * ui;
    AConfig        & Config;

    void disableInterface(bool flag);
};

#endif // ADEMOWINDOW_H
