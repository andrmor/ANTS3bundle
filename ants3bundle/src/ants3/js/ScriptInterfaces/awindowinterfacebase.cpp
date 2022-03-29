#include "awindowinterfacebase.h"

#include <QMainWindow>

AWindowInterfaceBase::AWindowInterfaceBase(QMainWindow * window) :
    BaseWindow(window)
{
    Help["getGeometry"] = "Returns array of X Y Width Height isMaximized";
}

void AWindowInterfaceBase::showNormal()
{
    BaseWindow->showNormal();
}

void AWindowInterfaceBase::showMaximized()
{
    BaseWindow->showMaximized();
}

void AWindowInterfaceBase::hide()
{
    BaseWindow->hide();
}

QVariantList AWindowInterfaceBase::getGeometry()
{
    QVariantList vl;
    const QRect rect = BaseWindow->geometry();
    vl << rect.x() << rect.y() << rect.width() << rect.height() << BaseWindow->isMaximized();
    return vl;
}

void AWindowInterfaceBase::setGeometry(QVariantList XYWHm)
{
    if (XYWHm.size() < 4)
        abort("setGeometry arguments shoud be an array of X Y Width Height [MaximizedBool]");
    else
    {
        BaseWindow->move(XYWHm[0].toInt(), XYWHm[1].toInt());
        BaseWindow->resize(XYWHm[2].toInt(), XYWHm[3].toInt());
        if (XYWHm.size() > 4 && XYWHm[4].toBool()) BaseWindow->showMaximized();
    }
}
