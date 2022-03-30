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
    void showNormal();
    void showMaximized();
    void hide();

    QVariantList getGeometry();
    void setGeometry(QVariantList XYWHm);

protected:
    AGuiWindow * BaseWindow = nullptr;
};

#endif // AWINDOWINTERFACEBASE_H
