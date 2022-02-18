#ifndef AREMOTEWINDOW_H
#define AREMOTEWINDOW_H

#include "aguiwindow.h"

#include <QWidget>
#include <QFrame>

#include <vector>

class AFarmHub;
class A3FarmNodeRecord;
class AServerDelegate;

namespace Ui {
class ARemoteWindow;
}

class ARemoteWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit ARemoteWindow(QWidget * parent);
    ~ARemoteWindow();

    void updateGui();

public slots:
    void onBusy(bool flag);

private:
    AFarmHub & FarmHub;
    Ui::ARemoteWindow *ui;

    std::vector<AServerDelegate*> Delegates;

private slots:
    void onGuiUpdate();
    void onUpdateSizeHint(AServerDelegate *d);

    void on_pbStatus_clicked();
    void on_pbRateServers_clicked();
    void on_leiTimeout_editingFinished();
    void on_pbAdd_clicked();    
    void on_pbRemove_clicked();

    void on_cbUseLocal_clicked(bool checked);
    void on_sbLocalProcesses_editingFinished();
    void on_cbUseFarm_clicked(bool checked);

private:
    void clear();
    void addNewNodeDelegate(A3FarmNodeRecord * record);

};

#endif // AREMOTEWINDOW_H
