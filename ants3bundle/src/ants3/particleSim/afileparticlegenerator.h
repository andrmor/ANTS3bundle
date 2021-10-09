#ifndef AFILEPARTICLEGENERATOR_H
#define AFILEPARTICLEGENERATOR_H

#include "aparticlegun.h"

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
    bool            checkFile(bool bExpanded);

    void            releaseResources() override;
    bool            generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) override;

    void            setStartEvent(int startEvent) override;

    std::string     getPreview(int maxLines);

    bool            generateG4File(int eventBegin, int eventEnd, const std::string & FileName);

public:
    AFileGeneratorSettings & Settings;

private:
    AFilePGEngine * Engine = nullptr;

    void determineFileFormat();
};

class AFilePGEngine
{
public:
    AFilePGEngine(AFileGeneratorSettings & settings) : Settings(settings) {}
    virtual ~AFilePGEngine(){}

    bool inspect(bool bDetailedInspection);

    virtual bool doInit() = 0;
    virtual bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) = 0;
    virtual bool doSetStartEvent(int startEvent) = 0;
    virtual bool doGenerateG4File(int eventBegin, int eventEnd, const std::string & FileName) = 0;

    virtual std::string getPreview(int maxLines) = 0;

protected:
    AFileGeneratorSettings & Settings;

    virtual bool doInspect(bool bDetailedInspection) = 0;
};

class AFilePGEngineG4antsTxt : public AFilePGEngine
{
public:
    AFilePGEngineG4antsTxt(AFileGeneratorSettings & settings) : AFilePGEngine(settings) {}
    ~AFilePGEngineG4antsTxt();

    bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) override;
    bool doSetStartEvent(int startEvent) override;
    bool doGenerateG4File(int eventBegin, int eventEnd, const std::string & FileName) override;

    std::string getPreview(int maxLines) override;

    static bool isFileG4AntsAscii(const std::string & FileName); // !!!*** check atoi is ok for this purpose

private:
    std::ifstream * inStream = nullptr;

    bool doInit() override;
    bool doInspect(bool bDetailedInspection) override;
};

class AFilePGEngineG4antsBin : public AFilePGEngine
{
public:
    AFilePGEngineG4antsBin(AFileGeneratorSettings & settings) : AFilePGEngine(settings) {}
    ~AFilePGEngineG4antsBin();

    bool doInit() override;
    bool doGenerateEvent(std::function<void(const AParticleRecord&)> handler) override;
    bool doSetStartEvent(int startEvent) override;
    bool doGenerateG4File(int eventBegin, int eventEnd, const std::string & FileName) override;

    std::string getPreview(int maxLines) override;

    static bool isFileG4AntsBin(const std::string & FileName);

private:
    std::ifstream * inStream = nullptr;

    bool doInspect(bool bDetailedInspection) override;
};

#endif // AFILEPARTICLEGENERATOR_H
