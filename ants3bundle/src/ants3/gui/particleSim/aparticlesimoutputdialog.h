#ifndef APARTICLESIMOUTPUTDIALOG_H
#define APARTICLESIMOUTPUTDIALOG_H

#include <QDialog>

class AParticleRunSettings;

namespace Ui {
class AParticleSimOutputDialog;
}

class AParticleSimOutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AParticleSimOutputDialog(QWidget *parent = nullptr);
    ~AParticleSimOutputDialog();

private slots:
    void on_pbAccept_clicked();

    void on_pbChangeDir_clicked();

    void on_cobAsciiBinary_currentIndexChanged(int index);

    void on_pbChangeTracks_clicked();

private:
    AParticleRunSettings & RunSet;

    Ui::AParticleSimOutputDialog * ui = nullptr;
};

#endif // APARTICLESIMOUTPUTDIALOG_H
