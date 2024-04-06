#include "aphotonlogsettingsform.h"
#include "ui_aphotonlogsettingsform.h"
#include "aphotonsimsettings.h"

APhotonLogSettingsForm::APhotonLogSettingsForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::APhotonLogSettingsForm)
{
    ui->setupUi(this);
}

APhotonLogSettingsForm::~APhotonLogSettingsForm()
{
    delete ui;
}

void APhotonLogSettingsForm::updateGui(const APhotonLogSettings & settings)
{
    ui->sbMaxNumber->setValue(settings.MaxNumber);
}

QString APhotonLogSettingsForm::updateSettings(APhotonLogSettings & settings) const
{
    settings.MaxNumber = ui->sbMaxNumber->value();

    return "";
}
