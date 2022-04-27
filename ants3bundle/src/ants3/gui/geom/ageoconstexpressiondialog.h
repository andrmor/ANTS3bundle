#ifndef AGEOCONSTEXPRESSIONDIALOG_H
#define AGEOCONSTEXPRESSIONDIALOG_H

#include <QObject>
#include <QDialog>
#include <QString>

class A3GeoConWin;
class AOneLineTextEdit;

class AGeoConstExpressionDialog : public QDialog
{
    Q_OBJECT
public:
    AGeoConstExpressionDialog(A3GeoConWin * geoConW, int index);

private:
    A3GeoConWin * GW = nullptr;
    QString       OriginalText;
    int           Index;

    AOneLineTextEdit * ed = nullptr;

private slots:
    void onAcceptPressed();
    void onCancelPressed();
};

#endif // AGEOCONSTEXPRESSIONDIALOG_H
