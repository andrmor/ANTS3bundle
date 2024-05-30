#ifndef APHOTONLOGSETTINGSFORM_H
#define APHOTONLOGSETTINGSFORM_H

#include <QWidget>

#include <QString>

namespace Ui {
class APhotonLogSettingsForm;
}

class APhotonLogSettings;
class QLineEdit;

class APhotonLogSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit APhotonLogSettingsForm(QWidget *parent = nullptr);
    ~APhotonLogSettingsForm();

    void updateGui(const APhotonLogSettings & settings);
    QString updateSettings(APhotonLogSettings & settings) const; // returns error string if any

    void setNumber(int num);

private slots:
    void on_leProcessesMust_customContextMenuRequested(const QPoint &pos);

    void on_leProcessesMustNot_customContextMenuRequested(const QPoint &pos);

private:
    Ui::APhotonLogSettingsForm * ui = nullptr;

    void addProcess(QLineEdit * le);
};

#endif // APHOTONLOGSETTINGSFORM_H
