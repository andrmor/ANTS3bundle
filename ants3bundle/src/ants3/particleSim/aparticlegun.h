#ifndef APARTICLEGUN_H
#define APARTICLEGUN_H

#include "aparticlerecord.h"

#include <vector>
#include <string>
#include <functional>

class AParticleGun
{
public:
    virtual ~AParticleGun(){}

    virtual bool init() = 0;             //called before first use
    virtual void releaseResources() {}   //called after end of operation
    virtual bool generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) = 0;

    virtual void setStartEvent(int) {} // for 'from file' generator

    bool         AbortRequested = false;
};

#endif // APARTICLEGUN_H
