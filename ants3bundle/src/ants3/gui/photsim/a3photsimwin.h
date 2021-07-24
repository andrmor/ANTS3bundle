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

    void updateGeneralOptions();

private:
    APhotSimSettings & SimSet;
    Ui::A3PhotSimWin * ui = nullptr;
};

#endif // A3PHOTSIMWIN_H
