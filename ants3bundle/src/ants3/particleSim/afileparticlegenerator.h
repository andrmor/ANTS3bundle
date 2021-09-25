#ifndef AFILEPARTICLEGENERATOR_H
#define AFILEPARTICLEGENERATOR_H

#include "aparticlegun.h"

#include <QString>
#include <QFile>
#include <QRegularExpression>
#include <QVector>

#include <string>
#include <vector>

class AFileGeneratorSettings;
class QTextStream;
class AFilePGEngine;

class AFileParticleGenerator : public AParticleGun
{
public:
    AFileParticleGenerator(AFileGeneratorSettings & settings);
    virtual         ~AFileParticleGenerator();

    bool            init() override;  // cannot modify settings, so validation is not possible with this method
    bool            initWithCheck(bool bExpanded);

    void            releaseResources() override;
    bool            generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) override;

    void            setStartEvent(int startEvent) override;

    bool            generateG4File(int eventBegin, int eventEnd, const QString & FileName);

public:
    AFileGeneratorSettings & Settings;

private:
    AFilePGEngine * Engine = nullptr;

    bool determineFileFormat();
    bool isFileG4Binary();
    bool isFileG4Ascii();
};

class AFilePGEngine
{
public:
    AFilePGEngine(AFileGeneratorSettings & settings) : Settings(settings) {}
    virtual ~AFilePGEngine(){}

    virtual bool doInit() = 0;
    virtual bool doInitAndInspect(bool bDetailedInspection) = 0;
    virtual bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) = 0;
    virtual bool doSetStartEvent(int startEvent) = 0;
    virtual bool doGenerateG4File(int eventBegin, int eventEnd, const QString & FileName) = 0;

    std::string ErrorString;

protected:
    AFileGeneratorSettings & Settings;

    const QRegularExpression rx = QRegularExpression("(\\ |\\,|\\:|\\t)");  // separators are: ' ' or ',' or ':' or '\t'
};

class AFilePGEngineG4antsTxt : public AFilePGEngine
{
public:
    AFilePGEngineG4antsTxt(AFileGeneratorSettings & settings) : AFilePGEngine(settings) {}
    ~AFilePGEngineG4antsTxt();

    bool doInit() override;
    bool doInitAndInspect(bool bDetailedInspection) override;
    bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) override;
    bool doSetStartEvent(int startEvent) override;
    bool doGenerateG4File(int eventBegin, int eventEnd, const QString & FileName) override;

private:
    std::ifstream * inStream = nullptr;
};

class AFilePGEngineG4antsBin : public AFilePGEngine
{
public:
    AFilePGEngineG4antsBin(AFileGeneratorSettings & settings) : AFilePGEngine(settings) {}
    ~AFilePGEngineG4antsBin();

    bool doInit() override;
    bool doInitAndInspect(bool bDetailedInspection) override;
    bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) override;
    bool doSetStartEvent(int startEvent) override;
    bool doGenerateG4File(int eventBegin, int eventEnd, const QString & FileName) override;

private:
    std::ifstream * inStream = nullptr;
};

#endif // AFILEPARTICLEGENERATOR_H
