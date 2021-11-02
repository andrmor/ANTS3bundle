#ifndef ADEMOWINDOW_H
#define ADEMOWINDOW_H

#include <QMainWindow>

namespace Ui {
class ADemoWindow;
}

class A3Config;

class ADemoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ADemoWindow(QWidget *parent = nullptr);
    ~ADemoWindow();

private slots:
    void onProgressReceived(double progress);
    void onDemoFinsihed();

    void on_pbRun_clicked();

    void on_leFrom_editingFinished();
    void on_leTo_editingFinished();

private:
    Ui::ADemoWindow *ui;

    A3Config & Config;
};

#endif // ADEMOWINDOW_H
