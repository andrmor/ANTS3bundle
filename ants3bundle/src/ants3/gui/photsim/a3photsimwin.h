#ifndef A3PHOTSIMWIN_H
#define A3PHOTSIMWIN_H

#include "aphotsimsettings.h"

#include <QMainWindow>

namespace Ui {
class A3PhotSimWin;
}

class A3PhotSimWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit A3PhotSimWin(QWidget *parent = nullptr);
    ~A3PhotSimWin();

    void updateGui();
    void updatePhotBombGui();
    void updateGeneralSettingsGui();


private slots:
    void on_pbdWave_clicked();

    void on_sbMaxNumbPhTransitions_editingFinished();

    void on_cbRndCheckBeforeTrack_clicked();

    void on_pbQEacceleratorHelp_clicked();

    void on_pbdGenerationModeStore_clicked();

private:
    APhotSimSettings & SimSet;
    Ui::A3PhotSimWin * ui = nullptr;


    void storeGeneralSettings();
    void storeGenerationModeSettings();
};

#endif // A3PHOTSIMWIN_H
