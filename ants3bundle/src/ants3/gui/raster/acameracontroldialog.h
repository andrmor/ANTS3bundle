#ifndef ACAMERACONTROLDIALOG_H
#define ACAMERACONTROLDIALOG_H

#include <QDialog>

class ARasterWindow;
class AGeometryHub;
class QLineEdit;
class QJsonObject;

namespace Ui {
class ACameraControlDialog;
}

class ACameraControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ACameraControlDialog(ARasterWindow * RasterWin, QWidget * parent = nullptr);
    ~ACameraControlDialog();

    void showAndUpdate();

    void setView(bool bSkipReadRange = false);
    void resetView();

    QString setFocus(const QString & name);

    int xPos = 0;
    int yPos = 0;

public slots:
    void updateGui();

private slots:
    void on_pbClose_clicked();

    void on_ledCenterX_editingFinished();
    void on_ledCenterY_editingFinished();
    void on_ledCenterZ_editingFinished();
    void on_pbSetView_clicked();
    void on_pbStepXM_clicked();
    void on_pbStepXP_clicked();
    void on_pbStepYM_clicked();
    void on_pbStepYP_clicked();
    void on_pbStepZM_clicked();
    void on_pbStepZP_clicked();
    void on_pbSetFocus_clicked();

protected:
    void closeEvent(QCloseEvent *);

private:
    ARasterWindow * RW = nullptr;
    const AGeometryHub & Geometry;

    Ui::ACameraControlDialog * ui;

    void setCenter(int index, QLineEdit * led);
    void makeStep(int index, double step);

};

#endif // ACAMERACONTROLDIALOG_H
