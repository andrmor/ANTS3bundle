#ifndef APARTICLESOURCEPLOTTER_H
#define APARTICLESOURCEPLOTTER_H

#include "ageomarkerclass.h"
class TVirtualGeoTrack;
class AParticleSourceRecordBase;
class AParticleSourceRecord_Standard;
class AParticleSourceRecord_EcoMug;

class AParticleSourcePlotter
{
public:
    static AGeoMarkerClass * plotSource(const AParticleSourceRecordBase * source);

    // next two will be private after adding an abstraction layer for ParticleSourceDialog
    static AGeoMarkerClass * plotSource(const AParticleSourceRecord_Standard & p);
    static AGeoMarkerClass * plotSource(const AParticleSourceRecord_EcoMug & p);

    static void clearTracks();

    static constexpr double pi = 3.14159265358979323846;

private:
    static TVirtualGeoTrack * createTrack();
};

#endif // APARTICLESOURCEPLOTTER_H
