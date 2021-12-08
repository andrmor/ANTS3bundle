#ifndef ASENSORWINDOW_H
#define ASENSORWINDOW_H

#include <QMainWindow>

namespace Ui {
class ASensorWindow;
}

class ASensorHub;

class ASensorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ASensorWindow(QWidget * parent = nullptr);
    ~ASensorWindow();

    void updateGui();

private slots:
    //automatic
    void on_cobSensorType_currentIndexChanged(int index);

    //user actions
    void on_cobModel_activated(int index);
    void on_pbAddNewModel_clicked();
    void on_pbCloneModel_clicked();
    void on_pbRemoveModel_clicked();
    void on_leModelName_editingFinished();
    void on_ledEffectivePDE_editingFinished();


    void on_cobSensorType_activated(int index);

private:
    ASensorHub & SensHub;
    Ui::ASensorWindow * ui = nullptr;
};

#endif // ASENSORWINDOW_H
