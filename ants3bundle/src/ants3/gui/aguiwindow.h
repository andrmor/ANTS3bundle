#ifndef AGUIWINDOW_H
#define AGUIWINDOW_H

#include <QMainWindow>
#include <QObject>

class WindowNavigatorClass;

class AGuiWindow : public QMainWindow
{
    Q_OBJECT
public:
    AGuiWindow(const QString & idStr, QWidget * parent);

    void storeGeomStatus();
    void restoreGeomStatus();

protected:

//    bool event(QEvent * event) override;
//    void showEvent(QShowEvent *event) override;
//    void hideEvent(QHideEvent *event) override;

    bool bWinGeomUpdateAllowed = true;

    int  WinPos_X = 40;
    int  WinPos_Y = 40;
    int  WinSize_W = 800;
    int  WinSize_H = 600;
    bool bWinVisible = false;
    bool bMaximized  = false;

private:
    QString IdStr;

    bool bColdStart = true;
};

#endif // AGUIWINDOW_H
