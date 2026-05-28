#ifndef ALIGHTSENSOREVENT_H
#define ALIGHTSENSOREVENT_H

#include <QBitArray>

#include <vector>

class ASensorHub;
class ARandomHub;
class APhotonSimSettings;
class APhotonStatistics;

class ALightSensorEvent
{
public:
    ALightSensorEvent();

    std::vector<float>     PMhits;       // Sensor hits in photoelectrons [PM#]
    std::vector<QBitArray> SiPMpixels;   // on/off status of SiPM pixels [PM#] [pixY] [pixX]

    void init();

    void clearHits();
    bool isHitsEmpty() const;

    bool checkSensorHit(int ipm, double time, int iWave, double x, double y, double angle, int numTransitions, int iSensorMat, double rnd);

    void  addDarkCounts();
    void  convertHitsToSignals();

    void generateHitsForLrfMode(int numPhotons, const double * position);

private:
    const APhotonSimSettings & SimSet;
    const ASensorHub         & SensorHub;
    ARandomHub               & RandomHub;
    APhotonStatistics        & SimStat;

    int numPMs = 0;

    bool  registerSiPMhit(int ipm, size_t binX, size_t binY); // return false if the pixel is already lit
    void  fillDetectionStatistics(int waveIndex, double time, double angle, int numTransitions);
};

#endif // ALIGHTSENSOREVENT_H
