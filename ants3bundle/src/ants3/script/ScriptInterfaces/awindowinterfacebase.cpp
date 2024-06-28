#include "awindowinterfacebase.h"
#include "aguiwindow.h"

#include <QTimer>

AWindowInterfaceBase::AWindowInterfaceBase(AGuiWindow * window) :
    BaseWindow(window)
{
    Help["showWindow"] = "Show window. If 'activateFlag' is true, also bring the window to the front and make it active";
    Help["showWindowMaximized"] = "Show window maximized. If 'activateFlag' is true, also bring the window to the front and make it active";
    Help["hide"] = "Hide the window";
    Help["getGeometry"] = "Return array with the window's position and size: [X-position, Y-position, Width, Height, isMaximizedFlag]";
    Help["setGeometry"] = "Configure position and size of the window. The argument is an array of [X-position, Y-position, Width, Height, isMaximizedFlag]";
}

void AWindowInterfaceBase::showWindow(bool activateFlag)
{
    QTimer::singleShot(0, BaseWindow, [this, activateFlag]()
    {
        BaseWindow->showNormal();
        if (activateFlag) BaseWindow->activateWindow();
    } );
}

void AWindowInterfaceBase::showWindowMaximized(bool activate)
{
    QTimer::singleShot(0, BaseWindow, [this, activate]()
    {
        BaseWindow->showMaximized();
        if (activate) BaseWindow->activateWindow();
    } );
}

void AWindowInterfaceBase::hide()
{
    QTimer::singleShot(0, BaseWindow, [this](){BaseWindow->hide();} );
}

QVariantList AWindowInterfaceBase::getGeometry()
{
    return BaseWindow->getGeometry();
}

void AWindowInterfaceBase::setGeometry(QVariantList XYWHm)
{
    if (XYWHm.size() < 4)
        abort("setGeometry arguments shoud be an array of X Y Width Height [MaximizedBool]");
    else
        QTimer::singleShot(0, BaseWindow, [this, XYWHm](){BaseWindow->setGeometry(XYWHm);} );
}
