#ifndef AGLOBSETWINDOW_H
#define AGLOBSETWINDOW_H

#include "aguiwindow.h"

class A3Global;

namespace Ui {
class AGlobSetWindow;
}

class AGlobSetWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit AGlobSetWindow(QWidget * parent = nullptr);
    ~AGlobSetWindow();

    void updateGui();

protected:
    bool event(QEvent * event);

public slots:
    void updateNetGui();
    void showNetSettings();
    void onRequestConfigureExchangeDir();

private slots:
    void on_pbChangeDataExchangeDir_clicked();
    void on_leDataExchangeDir_editingFinished();
    void on_pbChangeDataExchangeDir_customContextMenuRequested(const QPoint &pos);

    //void on_sbNumBinsHistogramsX_editingFinished();
    //void on_sbNumBinsHistogramsY_editingFinished();
    //void on_sbNumBinsHistogramsZ_editingFinished();

    void on_pbOpen_clicked();

#ifdef WEBSOCKETS
    void on_cbRunWebSocketServer_clicked(bool checked);
    void on_leWebSocketPort_editingFinished();
    void on_leWebSocketIP_editingFinished();
    void on_cbRunWebSocketServer_toggled(bool checked);
#endif

#ifdef USE_ROOT_HTML
    void on_cbAutoRunRootServer_clicked();
    void on_leRootServerPort_editingFinished();
    void on_cbRunRootServer_clicked(bool checked);
#endif

#ifdef __USE_ANTS_JSROOT__
    void on_leJSROOT_editingFinished();
#endif

    //void on_cobColorPalette_activated(int index);

    void on_cobStyle_textActivated(const QString &arg1);

    void on_cbUseStyleSystPalette_clicked(bool checked);

    //void on_pbForceDark_clicked();

private:
    A3Global & GlobSet;
    Ui::AGlobSetWindow * ui = nullptr;

    void setTab(int iTab);
};

#endif // AGLOBSETWINDOW_H
