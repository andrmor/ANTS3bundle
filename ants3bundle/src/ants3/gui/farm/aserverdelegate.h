#ifndef ASERVERDELEGATE_H
#define ASERVERDELEGATE_H

#include <QString>
#include <QObject>
#include <QFrame>

class AFarmNodeRecord;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QLabel;
class QProgressBar;

class AServerDelegate : public QFrame
{
    Q_OBJECT
public:
    AServerDelegate(AFarmNodeRecord * modelRecord = nullptr);

    void updateGui();

private:
    AFarmNodeRecord * modelRecord;

    QLabel    * labStatus;
    QLineEdit * leName;
    QCheckBox * cbEnabled;
    QLineEdit * leIP;
    QSpinBox  * sbPort;
    QLabel    * labProcesses;
    QLineEdit * ledSpeedFactor;
    //QProgressBar*  pbProgress;

private slots:
    void           updateModel();

private:
    void           setBackgroundGray(bool flag);
    void           setIcon(int option); // 0 is grey, 1 is green, 2 is red, 3 is yellow

signals:
    void           updateSizeHint(AServerDelegate*);

};

#endif // ASERVERDELEGATE_H
