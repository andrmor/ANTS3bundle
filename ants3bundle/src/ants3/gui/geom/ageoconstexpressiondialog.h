#ifndef AGEOCONSTEXPRESSIONDIALOG_H
#define AGEOCONSTEXPRESSIONDIALOG_H

#include <QObject>
#include <QDialog>
#include <QString>

class AGeoTreeWin;
class AOneLineTextEdit;

class AGeoConstExpressionDialog : public QDialog
{
    Q_OBJECT
public:
    AGeoConstExpressionDialog(AGeoTreeWin * geoConW, int index);

private:
    AGeoTreeWin * GW = nullptr;
    QString       OriginalText;
    int           Index;

    AOneLineTextEdit * ed = nullptr;

private slots:
    void onAcceptPressed();
    void onCancelPressed();
};

#endif // AGEOCONSTEXPRESSIONDIALOG_H
