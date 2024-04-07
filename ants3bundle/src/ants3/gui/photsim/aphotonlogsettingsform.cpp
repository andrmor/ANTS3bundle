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

    txt.clear();
    for (const AVolumeIndexPair & pair : settings.MustInclude_Volumes)
    {
        QString vol = pair.Volume.Data();
        int index = pair.Index;
        QString str = (index == -1 ? vol : vol + "#" + QString::number(index));
        txt += str + " ";
    }
    ui->leVolumesMust->setText(txt);

    txt.clear();
    for (const TString & ts : settings.MustNotInclude_Volumes)
        txt += QString(ts.Data()) + " ";
    ui->leVolumesMustNot->setText(txt);
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
    const QStringList slMnIP = ui->leProcessesMustNot->text().split(" ", Qt::SkipEmptyParts);
    for (const QString & strPr : slMnIP)
    {
        int res = APhotonHistoryLog::GetProcessIndex(strPr);
        if (res == -1) return "Not valid process name: " + strPr;
        settings.MustNotInclude_Processes.emplace(res);
    }

    settings.MustInclude_Volumes.clear();
    const QStringList slMIV = ui->leVolumesMust->text().split(" ", Qt::SkipEmptyParts);
    for (const QString & strPair : slMIV)
    {
        const QStringList sl = strPair.split('#', Qt::SkipEmptyParts);
        QString vol = sl.front();
        int index = (sl.size() == 1 ? -1 : sl[1].toInt());
        settings.MustInclude_Volumes.push_back({vol.toLatin1().data(), index});
    }

    settings.MustNotInclude_Volumes.clear();
    const QStringList slMnIV = ui->leVolumesMustNot->text().split(" ", Qt::SkipEmptyParts);
    for (const QString & strVol : slMnIV)
        settings.MustNotInclude_Volumes.emplace(strVol.toLatin1().data());

    return "";
}
