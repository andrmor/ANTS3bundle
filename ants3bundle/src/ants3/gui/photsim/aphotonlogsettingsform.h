#ifndef APHOTONLOGSETTINGSFORM_H
#define APHOTONLOGSETTINGSFORM_H

#include <QWidget>

#include <QString>

namespace Ui {
class APhotonLogSettingsForm;
}

class APhotonLogSettings;

class APhotonLogSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit APhotonLogSettingsForm(QWidget *parent = nullptr);
    ~APhotonLogSettingsForm();

    void updateGui(const APhotonLogSettings & settings);
    QString updateSettings(APhotonLogSettings & settings) const; // returns error string if any

private:
    Ui::APhotonLogSettingsForm *ui;
};

#endif // APHOTONLOGSETTINGSFORM_H
