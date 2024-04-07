#include "aphotonlogsettingsform.h"
#include "ui_aphotonlogsettingsform.h"
#include "aphotonsimsettings.h"
#include "aphotonhistorylog.h"

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

    QString txt;
    for (int iPr : settings.MustInclude_Processes)
        txt += APhotonHistoryLog::GetProcessName(iPr) + " ";
    ui->leProcessesMust->setText(txt);

    txt.clear();
    for (int iPr : settings.MustNotInclude_Processes)
        txt += APhotonHistoryLog::GetProcessName(iPr) + " ";
    ui->leProcessesMustNot->setText(txt);
}

QString APhotonLogSettingsForm::updateSettings(APhotonLogSettings & settings) const
{
    settings.MaxNumber = ui->sbMaxNumber->value();

    settings.MustInclude_Processes.clear();
    const QStringList slMIP = ui->leProcessesMust->text().split(" ", Qt::SkipEmptyParts);
    for (const QString & strPr : slMIP)
    {
        int res = APhotonHistoryLog::GetProcessIndex(strPr);
        if (res == -1) return "Not valid process name: " + strPr;
        settings.MustInclude_Processes.push_back(res);
    }

    settings.MustNotInclude_Processes.clear();
    const QStringList slMNIP = ui->leProcessesMustNot->text().split(" ", Qt::SkipEmptyParts);
    for (const QString & strPr : slMNIP)
    {
        int res = APhotonHistoryLog::GetProcessIndex(strPr);
        if (res == -1) return "Not valid process name: " + strPr;
        settings.MustNotInclude_Processes.emplace(res);
    }

    return "";
}
