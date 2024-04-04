#ifndef AGUIFROMSCRWIN_H
#define AGUIFROMSCRWIN_H

#include "aguiwindow.h"
#include <QObject>

class AGuiFromScrWin : public AGuiWindow
{
    Q_OBJECT
public:
    AGuiFromScrWin(QWidget * parent);

    void restoreGeomStatus() override;

    QLayout * resetLayout();

protected:
    void closeEvent(QCloseEvent * event) override;

signals:
    void windowClosed();

};

#endif // AGUIFROMSCRWIN_H
