#ifndef AGUIFROMSCRWIN_H
#define AGUIFROMSCRWIN_H

#include "aguiwindow.h"
#include <QObject>

class AGuiFromScrWin : public AGuiWindow
{
    Q_OBJECT
public:
    AGuiFromScrWin(QWidget * parent);

    QLayout * resetLayout();
};

#endif // AGUIFROMSCRWIN_H
