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

    void resetPredictedTrackID()     {NextTrackID = 1;}
    void incrementPredictedTrackID() {NextTrackID++;}
    int  getPredictedTrackID() const {return NextTrackID;}

    bool        AbortRequested = false;

protected:
    int         NextTrackID    = 1;
};

#endif // APARTICLEGUN_H
