#ifndef APHOTONTRACKVISDIALOG_H
#define APHOTONTRACKVISDIALOG_H

#include "atrackvisattributes.h"

#include <vector>

#include <QDialog>

namespace Ui {
class APhotonTrackVisDialog;
}

class QComboBox;
class QSpinBox;
class QPushButton;

class APhotonTrackVisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit APhotonTrackVisDialog(QWidget * parent = nullptr);
    ~APhotonTrackVisDialog();

private slots:
    void on_pbClose_clicked();

private:
    Ui::APhotonTrackVisDialog * ui = nullptr;

    ATrackVisAttributes & CurrentAts;

    struct AProps
    {
        ATrackAttributes * attr = nullptr;
        QComboBox   * cob = nullptr;
        QSpinBox    * sb  = nullptr;
        QPushButton * pb  = nullptr;
    };

    std::vector<AProps> AllTypes;

    //ATrackAttributes PrimaryPhotonTracks;
    //ATrackAttributes SecondaryPhotonTracks;
    //ATrackAttributes HitSensorPhotonTracks;

    void updateGui();
    void updateColor(QPushButton * pb, int color);
};

#endif // APHOTONTRACKVISDIALOG_H
