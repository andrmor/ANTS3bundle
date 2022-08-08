#ifndef ALOCALNORMALSAMPLER_H
#define ALOCALNORMALSAMPLER_H

class ASurfaceSettings;
class ARandomHub;

class ALocalNormalSampler
{
public:
    ALocalNormalSampler(ASurfaceSettings & settings);

    void getLocalNormal(const double * globalNormal, const double * photonDirection, double * localNormal);

protected:
    ASurfaceSettings & Settings;
    ARandomHub       & RandHub;
};

#endif // ALOCALNORMALSAMPLER_H
