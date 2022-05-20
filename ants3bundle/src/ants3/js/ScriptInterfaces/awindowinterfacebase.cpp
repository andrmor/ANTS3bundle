#include "awindowinterfacebase.h"
#include "aguiwindow.h"

#include <QTimer>

AWindowInterfaceBase::AWindowInterfaceBase(AGuiWindow * window) :
    BaseWindow(window)
{
    Help["getGeometry"] = "Returns array of X Y Width Height isMaximized";
}

void AWindowInterfaceBase::showWindow(bool activate)
{
    QTimer::singleShot(0, BaseWindow, [this, activate]()
    {
        BaseWindow->showNormal();
        if (activate) BaseWindow->activateWindow();
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
