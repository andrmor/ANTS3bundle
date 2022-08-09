#ifndef ALOCALNORMALSAMPLER_H
#define ALOCALNORMALSAMPLER_H

class ASurfaceSettings;
class ARandomHub;

class ALocalNormalSampler
{
public:
    ALocalNormalSampler(const ASurfaceSettings & settings);

    void getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal);

protected:
    const ASurfaceSettings & Settings;
    ARandomHub             & RandomHub;
};

#endif // ALOCALNORMALSAMPLER_H
