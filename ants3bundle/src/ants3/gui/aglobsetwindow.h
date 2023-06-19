#ifndef AGLOBSETWINDOW_H
#define AGLOBSETWINDOW_H

#include "aguiwindow.h"

class A3Global;
class MainWindow;
class ANetworkModule;
class ARootStyle_SI;

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
    void setTab(int iTab);

    ARootStyle_SI * GStyleInterface = nullptr;  // if created -> owned by the script manager

protected:
    bool event(QEvent *event);

public slots:
    void updateNetGui();
    void showNetSettings();
    void onRequestConfigureExchangeDir();

private slots:
    void on_pbgStyleScript_clicked();

    void on_pbChangeDataExchangeDir_clicked();
    void on_leDataExchangeDir_editingFinished();
    void on_pbChangeDataExchangeDir_customContextMenuRequested(const QPoint &pos);

    void on_cbOpenImageExternalEditor_clicked(bool checked);

    void on_sbNumSegments_editingFinished();
    void on_sbNumBinsHistogramsX_editingFinished();
    void on_sbNumBinsHistogramsY_editingFinished();
    void on_sbNumBinsHistogramsZ_editingFinished();

    void on_pbOpen_clicked();

//    void on_sbNumPointsFunctionX_editingFinished();
//    void on_sbNumPointsFunctionY_editingFinished();
//    void on_cbSaveRecAsTree_IncludePMsignals_clicked(bool checked);
//    void on_cbSaveRecAsTree_IncludeRho_clicked(bool checked);
//    void on_cbSaveRecAsTree_IncludeTrue_clicked(bool checked);

//    void on_cbRunWebSocketServer_clicked(bool checked);
//    void on_leWebSocketPort_editingFinished();
//    void on_leWebSocketIP_editingFinished();
//    void on_cbRunWebSocketServer_toggled(bool checked);

    void on_cbAutoRunRootServer_clicked();
    void on_leRootServerPort_editingFinished();
    void on_leJSROOT_editingFinished();
    void on_cbRunRootServer_clicked(bool checked);

//    void on_cbSaveSimAsText_IncludeNumPhotons_clicked(bool checked);
//    void on_cbSaveSimAsText_IncludePositions_clicked(bool checked);

    //void on_cobColorPalette_activated(int index);

    void on_cobStyle_textActivated(const QString &arg1);

    void on_cbUseStyleSystPalette_clicked(bool checked);

private:
    A3Global & GlobSet;
    Ui::AGlobSetWindow * ui = nullptr;
};

#endif // AGLOBSETWINDOW_H
