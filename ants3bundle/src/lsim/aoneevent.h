#ifndef AONEEVENT_H
#define AONEEVENT_H

#include <QBitArray>

#include <vector>

class ASensorHub;
class ARandomHub;
class APhotonSimSettings;
class APhotonStatistics;

class AOneEvent
{
public:
    AOneEvent();

    std::vector<float>     PMhits;       // PM hits in photoelectrons [PM#]
    std::vector<QBitArray> SiPMpixels;   // on/off status of SiPM pixels [PM#] [pixY] [pixX]

    void init();

    void clearHits();
    bool isHitsEmpty() const;

    bool checkSensorHit(int ipm, double time, int iWave, double x, double y, double angle, int numTransitions, double rnd);

    void  addDarkCounts();
    void  convertHitsToSignals();

private:
    const APhotonSimSettings & SimSet;
    const ASensorHub         & SensorHub;
    ARandomHub               & RandomHub;
    APhotonStatistics        & SimStat;

    int numPMs;

    bool  registerSiPMhit(int ipm, size_t binX, size_t binY); // return false if the pixel is already lit
    void  fillDetectionStatistics(int waveIndex, double time, double angle, int numTransitions);
};

#endif // AONEEVENT_H
