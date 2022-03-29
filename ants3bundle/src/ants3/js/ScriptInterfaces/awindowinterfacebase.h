#ifndef AWINDOWINTERFACEBASE_H
#define AWINDOWINTERFACEBASE_H

#include "ascriptinterface.h"

#include <QVariantList>

class QMainWindow;

class AWindowInterfaceBase : public AScriptInterface
{
    Q_OBJECT

public:
    AWindowInterfaceBase(QMainWindow * window);

    void setWindow();

public slots:
    void showNormal();
    void showMaximized();
    void hide();

    QVariantList getGeometry();
    void setGeometry(QVariantList XYWHm);

protected:
    QMainWindow * BaseWindow = nullptr;
};

#endif // AWINDOWINTERFACEBASE_H
