#ifndef ATRACKDRAWPROPERTIES_H
#define ATRACKDRAWPROPERTIES_H

#include <QDialog>

class AParticleTrackVisuals;

namespace Ui {
class ATrackDrawProperties;
}

class ATrackDrawDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ATrackDrawDialog(QWidget * parent);
    ~ATrackDrawDialog();

private slots:
    void on_pbClose_clicked();

    void on_pbDefaultParticleAtt_clicked();

    void on_pbEditCustom_clicked();

    void on_pbSave_clicked();
    void on_pbLoad_clicked();

    void on_cobParticle_activated(int index);

    void on_pbNew_clicked();

    void on_pbRemove_clicked();

private:
    AParticleTrackVisuals & settings;
    Ui::ATrackDrawProperties * ui;

    void updateParticleAttributes();
    void updateParticles(int forceIndex = -1);
};

#endif // ATRACKDRAWPROPERTIES_H
