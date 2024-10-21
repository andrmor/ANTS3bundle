#ifndef AWINDOWINTERFACEBASE_H
#define AWINDOWINTERFACEBASE_H

#include "ascriptinterface.h"

#include <QVariantList>

class AGuiWindow;

class AWindowInterfaceBase : public AScriptInterface
{
    Q_OBJECT

public:
    AWindowInterfaceBase(AGuiWindow * window);

public slots:
    void showWindow(bool activateFlag = true);
    void showWindowMaximized(bool activate);
    void hide();

    QVariantList getGeometry();
    void setGeometry(QVariantList XYWHm);

protected:
    AGuiWindow * BaseWindow = nullptr;
};

#endif // AWINDOWINTERFACEBASE_H
