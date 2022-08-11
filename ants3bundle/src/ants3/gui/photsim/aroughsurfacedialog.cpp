#include "aroughsurfacedialog.h"
#include "ui_aroughsurfacedialog.h"
#include "asurfacesettings.h"

ARoughSurfaceDialog::ARoughSurfaceDialog(ASurfaceSettings & settings, QWidget * parent) :
    QWidget(parent),
    Settings(settings),
    ui(new Ui::ARoughSurfaceDialog)
{
    ui->setupUi(this);
}

ARoughSurfaceDialog::~ARoughSurfaceDialog()
{
    delete ui;
}

void ARoughSurfaceDialog::updateGui()
{
    int index = 0;
    switch (Settings.Model)
    {
    case ASurfaceSettings::Polished : index = 0; break;
    case ASurfaceSettings::GaussSimplistic : index = 1; break;
    default:
        qWarning() << "Unknown model!";
        index = 0;
        break;
    }

    ui->swModel->setVisible(index != 0);
}

void ARoughSurfaceDialog::on_cobModel_currentIndexChanged(int index)
{
    ui->swModel->setVisible(index != 0);
}

void ARoughSurfaceDialog::on_cobModel_activated(int index)
{
    if (index == 0) Settings.Model = ASurfaceSettings::Polished;
    else Settings.Model = ASurfaceSettings::GaussSimplistic;
}

