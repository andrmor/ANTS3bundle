#ifndef AADVANCEDPARTICLEOPTIONSDIALOG_H
#define AADVANCEDPARTICLEOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class AAdvancedParticleOptionsDialog;
}

class AParticleSimSettings;

class AAdvancedParticleOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AAdvancedParticleOptionsDialog(QWidget *parent = nullptr);
    ~AAdvancedParticleOptionsDialog();

private slots:
    void on_pClose_clicked();

private:
    Ui::AAdvancedParticleOptionsDialog * ui = nullptr;

    AParticleSimSettings & Settings;
};

#endif // AADVANCEDPARTICLEOPTIONSDIALOG_H
