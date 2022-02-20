#ifndef APARTICLESOURCEPLOTTER_H
#define APARTICLESOURCEPLOTTER_H

class AParticleSourceRecord;

class AParticleSourcePlotter
{
public:
    static void plotSource(const AParticleSourceRecord & p);

    static constexpr double pi = 3.14159265358979323846;
};

#endif // APARTICLESOURCEPLOTTER_H
