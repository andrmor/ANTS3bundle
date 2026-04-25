#ifndef AGEOMARKERPROPSDIALOG_H
#define AGEOMARKERPROPSDIALOG_H

#include "ageomarkerproperties.h"

#include <vector>

#include <QDialog>
#include <QObject>

class QPushButton;
class QVBoxLayout;
class QGridLayout;
class QDoubleValidator;
class QLineEdit;

class AGeoMarkerPropsDialog : public QDialog
{
    Q_OBJECT

public:
    AGeoMarkerPropsDialog(AGeoMarkerPropDatabase & geoMarkProps, QWidget * parent);

    void updateGui();

private slots:
    void onApply();
    void onApplyAndClose();
    void updateMultiplier();

    void save();
    void load();
    void restoreDefault();
    void makeDefault();
    void factoryReset();

private:
    AGeoMarkerPropDatabase & GeoMarkProps;
    std::vector<std::pair<QString, AGeoMarkerProperties>> LocalData;
    double SizeMultiplier = 1.0;

    QGridLayout * layGr   = nullptr;
    QLineEdit   * ledMult = nullptr;

    QDoubleValidator * Validator = nullptr;

    void copyLocalToGlobal();
    void updateColor(QPushButton * pb, int color);
    void showInfo(QString type);

signals:
    void requestRedraw(bool activateWindow, bool same, bool colorUpdateAllowed);
};

#endif // AGEOMARKERPROPSDIALOG_H
