#ifndef AVIEWER3DSETTINGSDIALOG_H
#define AVIEWER3DSETTINGSDIALOG_H

#include <vector>

#include <QDialog>
#include <QString>

namespace Ui {
class AViewer3DSettingsDialog;
}

class AViewer3DSettings;

class AViewer3DSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AViewer3DSettingsDialog(AViewer3DSettings & settings, QWidget * parent = nullptr);
    ~AViewer3DSettingsDialog();

    bool isRecalcMaxRequired() const;

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();
    void on_pbCopyToScaling_clicked();
    void on_pbApply_clicked();

private:
    AViewer3DSettings & Settings;
    Ui::AViewer3DSettingsDialog * ui = nullptr;

    double oldFoV = -1e10;

private:
    void updateSettings();

signals:
    void requestUpdate(bool needRecalc);
};

#endif // AVIEWER3DSETTINGSDIALOG_H
