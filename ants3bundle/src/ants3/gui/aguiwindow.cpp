#include "aguiwindow.h"
#include "ajsontools.h"
#include "guitools.h"

//#include <QEvent>
#include <QSettings>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QDebug>

AGuiWindow::AGuiWindow(const QString & idStr, QWidget * parent) :
    QMainWindow(parent), IdStr(idStr) {}

void AGuiWindow::storeGeomStatus()
{
    QSettings settings;
    settings.beginGroup(IdStr);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("visible", isVisible());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void AGuiWindow::restoreGeomStatus()
{
    QSettings settings;
    settings.beginGroup(IdStr);
    restoreGeometry(settings.value("geometry").toByteArray());
    bool bVisible = settings.value("visible", false).toBool();
    bool bmax = settings.value("maximized", false).toBool();
    if (bVisible)
    {
        if (bmax) showMaximized();
        else      showNormal();
    }
    settings.endGroup();
}

void AGuiWindow::onMainWinButtonClicked(bool show)
{
    if (show)
    {
        restoreGeomStatus();
        showNormal();
        activateWindow();
    }
    else
    {
        storeGeomStatus();
        hide();
    }
}

QVariantList AGuiWindow::getGeometry()
{
    QVariantList vl;
    QRect rec = frameGeometry();
    vl << rec.x() << rec.y() << rec.width() << rec.height() << isMaximized();
    return vl;
}

void AGuiWindow::setGeometry(QVariantList XYWHm)
{
    if (XYWHm.size() < 4) return;

    move(XYWHm[0].toInt(), XYWHm[1].toInt());
    resize(XYWHm[2].toInt(), XYWHm[3].toInt());
    bool bMax = false;
    if (XYWHm.size() > 4) bMax = XYWHm[4].toBool();

    if (bMax) showMaximized();
    else      showNormal();
}

/*
bool AGuiWindow::event(QEvent *event)
{
    //qDebug() << IdStr << event->type();

    if (event->type() == QEvent::Show)
    {
        qDebug() << "SHOWWWW EVENT!!!!";
        if (bColdStart)
        {
            qDebug() << "Cold start!" << IdStr;
            bColdStart = false;
            //showNormal();

            resize(WinSize_W, WinSize_H);
            move(WinPos_X, WinPos_Y);


            guitools::assureWidgetIsWithinVisibleArea(this);

            if (bMaximized) showMaximized();
            else showNormal();

            bWinGeomUpdateAllowed = true;
        }
    }
    return QMainWindow::event(event);


    if (event->type() == QEvent::WindowStateChange)
    {
        if( windowState() == Qt::WindowMinimized )
        {
            //qDebug() << IdStr<<"Minimized!";
            event->accept();
            return true;
        }
        else if( windowState() == Qt::WindowNoState )
        {
            //qDebug() << IdStr<<"Restored!";
        }
        //qDebug() << windowState();
    }

    //if (WNav)
    {
        if (event->type() == QEvent::Hide)
        {
            qDebug() << IdStr<<"----Hide event"<<isVisible();
            bWinVisible = false;
            bWinGeomUpdateAllowed = false;
            //WNav->HideWindowTriggered(IdStr);
            return true;
        }
        else if (event->type() == QEvent::Show)
        {
            qDebug() << IdStr<<"----Show event";
            //WNav->ShowWindowTriggered(IdStr);
            bWinGeomUpdateAllowed = true;
            bWinVisible = true;

            return true;
        }
    }
    return QMainWindow::event(event);
}
*/

/*
void AGuiWindow::showEvent(QShowEvent * event)
{
    qDebug() << IdStr<< "----Show event";
    bWinGeomUpdateAllowed = true;
    bWinVisible = true;
}
*/

void AGuiWindow::hideEvent(QHideEvent * event)
{
    //qDebug() << IdStr<<"----Hide event"<<isVisible();
    storeGeomStatus();
    QMainWindow::hideEvent(event);
}

/*
void AGuiWindow::moveEvent(QMoveEvent *event)
{
    storeGeomStatus();
    QMainWindow::moveEvent(event);
}
*/

