#include "a3photsimwin.h"
#include "ui_a3photsimwin.h"

A3PhotSimWin::A3PhotSimWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::A3PhotSimWin)
{
    ui->setupUi(this);
}

A3PhotSimWin::~A3PhotSimWin()
{
    delete ui;
}
