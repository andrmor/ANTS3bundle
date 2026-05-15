#ifndef APARTICLESOURCEDIALOGBASE_H
#define APARTICLESOURCEDIALOGBASE_H

#include <QDialog>
#include <QString>

class AParticleSourceRecordBase;
class AParticleGun;
class TObject;
class AOneLineTextEdit;

class AParticleSourceDialogBase : public QDialog
{
    Q_OBJECT

public:
    explicit AParticleSourceDialogBase(QWidget * parent = nullptr);
    virtual ~AParticleSourceDialogBase(){}

    virtual AParticleSourceRecordBase * getResult() = 0;

    static AParticleSourceDialogBase * factory(AParticleSourceRecordBase * source, QWidget * parent);

protected:
    void processGeoConstAwareEditFinished(AOneLineTextEdit * edit, QString & str, double & val, const QString & name, QWidget * parent,
                                          bool bForbidZero = false, bool bForbidNegative = false, bool bMakeHalf = false);

signals:
    void requestTestParticleGun(AParticleGun * gun, int num, bool fillStatistics);
    void sourceRecordChangedInEditMode(AParticleSourceRecordBase * sourceRecord);
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);

};

#endif // APARTICLESOURCEDIALOGBASE_H
