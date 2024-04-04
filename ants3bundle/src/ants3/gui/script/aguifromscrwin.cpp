#include "aguifromscrwin.h"

#include <QDebug>
#include <QVBoxLayout>

AGuiFromScrWin::AGuiFromScrWin(QWidget * parent) :
    AGuiWindow("GuiScr", parent)
{
    Qt::WindowFlags windowFlags = (Qt::Window | Qt::CustomizeWindowHint);
    windowFlags |= Qt::WindowCloseButtonHint;
    setWindowFlags( windowFlags );

    QWidget * widget = new QWidget();
    QVBoxLayout * lay = new QVBoxLayout();
    widget->setLayout(lay);
    setCentralWidget(widget);
}

void AGuiFromScrWin::restoreGeomStatus()
{
    AGuiWindow::restoreGeomStatus();
    hide();
}

QLayout * AGuiFromScrWin::resetLayout()
{
    QWidget * oldCentral = centralWidget();

    QWidget * widget = new QWidget();
    QVBoxLayout * lay = new QVBoxLayout();
    widget->setLayout(lay);
    setCentralWidget(widget);

    delete oldCentral;

    return lay;
}

void AGuiFromScrWin::closeEvent(QCloseEvent *)
{
    emit windowClosed();
}
