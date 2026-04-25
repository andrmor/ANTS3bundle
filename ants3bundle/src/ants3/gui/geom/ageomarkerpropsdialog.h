#ifndef AGEOMARKERPROPSDIALOG_H
#define AGEOMARKERPROPSDIALOG_H

#include "ageomarkerproperties.h"

#include <vector>

#include <QDialog>
#include <QObject>

class QPushButton;

class AGeoMarkerPropsDialog : public QDialog
{
    Q_OBJECT

public:
    AGeoMarkerPropsDialog(AGeoMarkerPropDatabase & geoMarkProps, QWidget * parent);


private slots:
    void onApply();
    void onApplyAndClose();

private:
    AGeoMarkerPropDatabase & GeoMarkProps;
    std::vector<std::pair<QString, AGeoMarkerProperties>> LocalData;

    void copyLocalToGlobal();
    void updateColor(QPushButton * pb, int color);

signals:
    void requestRedraw(bool activateWindow, bool same, bool colorUpdateAllowed);
};

#endif // AGEOMARKERPROPSDIALOG_H
