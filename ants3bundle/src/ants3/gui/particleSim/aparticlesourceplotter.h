#ifndef APARTICLESOURCEPLOTTER_H
#define APARTICLESOURCEPLOTTER_H

class AParticleSourceRecord_Standard;

class AParticleSourcePlotter
{
public:
    static void plotSource(const AParticleSourceRecord_Standard & p);
    static void clearTracks();

    static constexpr double pi = 3.14159265358979323846;
};

#endif // APARTICLESOURCEPLOTTER_H
