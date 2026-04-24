#ifndef AGEOMARKERPROPSDIALOG_H
#define AGEOMARKERPROPSDIALOG_H

#include "ageomarkerproperties.h"

#include <vector>

#include <QDialog>
#include <QObject>

class A3Global;
class QPushButton;

class AGeoMarkerPropsDialog : public QDialog
{
    Q_OBJECT

public:
    AGeoMarkerPropsDialog(QWidget * parent);

    std::vector<std::pair<QString, AGeoMarkerProperties>> LocalData;

private slots:
    void onApply();
    void onApplyAndClose();

private:
    A3Global & GlobSet;

    void copyLocalToGlobal();
    void updateColor(QPushButton * pb, int color);

signals:
    void requestRedraw(bool activateWindow, bool same, bool colorUpdateAllowed);
};

#endif // AGEOMARKERPROPSDIALOG_H
