#ifndef APARTICLEGUN_H
#define APARTICLEGUN_H

#include <QObject>
#include <QString>

#include <vector>

class QJsonObject;
class AParticleRecord;

class AParticleGun : public QObject
{
    Q_OBJECT

public:
    virtual ~AParticleGun(){}

    virtual bool Init() = 0;             //called before first use
    virtual void ReleaseResources() {}   //called after end of operation
    virtual bool GenerateEvent(std::vector<AParticleRecord*> & GeneratedParticles, int iEvent) = 0;

    virtual void SetStartEvent(int) {} // for 'from file' generator

    const QString & GetErrorString() const {return ErrorString;}
    void            SetErrorString(const QString& str) {ErrorString = str;}

    bool            IsAbortRequested() const {return bAbortRequested;}

public slots:
    virtual void abort() {bAbortRequested = true;}

protected:
    QString ErrorString;
    bool bAbortRequested = false;
};

#endif // APARTICLEGUN_H
