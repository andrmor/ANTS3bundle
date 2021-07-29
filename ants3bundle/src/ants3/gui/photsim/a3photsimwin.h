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
    explicit A3PhotSimWin(QWidget * parent = nullptr);
    ~A3PhotSimWin();

public slots:
    void updateGui();

private slots:
    void on_pbdWave_clicked();
    void on_sbMaxNumbPhTransitions_editingFinished();
    void on_cbRndCheckBeforeTrack_clicked();
    void on_pbQEacceleratorHelp_clicked();

    void on_cobSimType_activated(int index);

    void on_cobScanNumPhotonsMode_activated(int index);

    void on_cobNodeGenerationMode_activated(int index);

    void on_ledSingleX_editingFinished();

    void on_ledSingleY_editingFinished();

    void on_ledSingleZ_editingFinished();

private:
    APhotSimSettings & SimSet;
    Ui::A3PhotSimWin * ui = nullptr;

    void updatePhotBombGui();
    void updateGeneralSettingsGui();

    void storeGeneralSettings();
};

#endif // A3PHOTSIMWIN_H
