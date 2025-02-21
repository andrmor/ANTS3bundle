#ifndef AGUIWINDOW_H
#define AGUIWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QVariantList>

class WindowNavigatorClass;

class AGuiWindow : public QMainWindow
{
    Q_OBJECT
public:
    AGuiWindow(const QString & idStr, QWidget * parent);

    void storeGeomStatus();
    virtual void restoreGeomStatus();

    void onMainWinButtonClicked(bool show);

public slots:
    QVariantList getGeometry();
    void setGeometry(QVariantList XYWHm);

protected:
    void hideEvent(QHideEvent *event) override;
    //void moveEvent(QMoveEvent * event) override; // cannot do it in Linux: the position is not yet finalized so the wrong window position is stored

    //bool event(QEvent * event) override;
    //void showEvent(QShowEvent *event) override;

/*
    bool bWinGeomUpdateAllowed = true;

    int  WinPos_X = 40;
    int  WinPos_Y = 40;
    int  WinSize_W = 800;
    int  WinSize_H = 600;
    bool bWinVisible = false;
    bool bMaximized  = false;
*/

private:
    QString IdStr;

    bool bColdStart = true;
};

#endif // AGUIWINDOW_H
