#ifndef A3PHOTSIMWIN_H
#define A3PHOTSIMWIN_H

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

private:
    Ui::A3PhotSimWin *ui;
};

#endif // A3PHOTSIMWIN_H
