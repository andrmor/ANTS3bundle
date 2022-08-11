#ifndef AROUGHSURFACEDIALOG_H
#define AROUGHSURFACEDIALOG_H

#include <QWidget>

class ASurfaceSettings;

namespace Ui {
class ARoughSurfaceDialog;
}

class ARoughSurfaceDialog : public QWidget
{
    Q_OBJECT

public:
    ARoughSurfaceDialog(ASurfaceSettings & settings, QWidget * parent = nullptr);
    ~ARoughSurfaceDialog();

    void updateGui();

private slots:
    void on_cobModel_currentIndexChanged(int index);

    void on_cobModel_activated(int index);

private:
    ASurfaceSettings & Settings;
    Ui::ARoughSurfaceDialog *ui;
};

#endif // AROUGHSURFACEDIALOG_H
