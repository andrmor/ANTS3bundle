#ifndef APARTICLESOURCEDIALOGBASE_H
#define APARTICLESOURCEDIALOGBASE_H

#include <QDialog>
#include <QString>

class AParticleSourceRecordBase;
class AParticleGun;
class TObject;

class AParticleSourceDialogBase : public QDialog
{
    Q_OBJECT
public:
    explicit AParticleSourceDialogBase(QWidget * parent = nullptr);
    virtual ~AParticleSourceDialogBase(){}

    virtual AParticleSourceRecordBase * getResult() = 0;

    static AParticleSourceDialogBase * factory(AParticleSourceRecordBase * source, QWidget * parent);

signals:
    void requestTestParticleGun(AParticleGun * gun, int num, bool fillStatistics);
    void requestShowSource();
    void requestDraw(TObject * obj, QString options, bool transferOwnership, bool focusWindow);

};

#endif // APARTICLESOURCEDIALOGBASE_H
