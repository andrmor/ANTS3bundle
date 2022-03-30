#include "awindowinterfacebase.h"
#include "aguiwindow.h"

#include <QTimer>

AWindowInterfaceBase::AWindowInterfaceBase(AGuiWindow * window) :
    BaseWindow(window)
{
    Help["getGeometry"] = "Returns array of X Y Width Height isMaximized";
}

void AWindowInterfaceBase::showNormal()
{
    QTimer::singleShot(0, BaseWindow, [this](){BaseWindow->showNormal();} );
}

void AWindowInterfaceBase::showMaximized()
{
    QTimer::singleShot(0, BaseWindow, [this](){BaseWindow->showMaximized();} );
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
