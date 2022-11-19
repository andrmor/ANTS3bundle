#ifndef AONEEVENT_H
#define AONEEVENT_H

#include <QBitArray>

#include <vector>

class ASensorHub;
class ARandomHub;
class APhotonSimSettings;
class APhotonStatistics;
class QTextStream;

class AOneEvent
{
public:
    AOneEvent();

    std::vector<float>     PMhits;       // PM hits [pm]
    //std::vector<float>     PMsignals;    // -- converted to signal [pm]
    std::vector<QBitArray> SiPMpixels;   //on/off status of SiPM pixels [PM#] [pixY] [pixX]

    void init();

    void clearHits();
    bool isHitsEmpty() const;

    bool checkSensorHit(int ipm, double time, int iWave, double x, double y, double angle, int numTransitions, double rnd);

    void  addDarkCounts(); // !!!*** expand to all sensor types
    void  convertHitsToSignals();

    //void addHits(int ipm, float hits) {PMhits[ipm] += hits;}
    //void addSignals(int ipm, float signal) {PMsignals[ipm] += signal;}  //only used in LRF-based sim

private:
    const APhotonSimSettings & SimSet;
    const ASensorHub         & SensorHub;
    ARandomHub               & RandomHub;
    APhotonStatistics        & SimStat;

    //settings
    int numPMs;

    void  registerSiPMhit(int ipm, int binX, int binY, float numHits = 1.0f); // numHits != 1 for two cases: 1) simplistic model of microcell cross-talk  2) advanced model of dark counts
    float generateDarkHitIncrement(int ipm) const;
    void  fillDetectionStatistics(int WaveIndex, double time, double angle, int Transitions);
};

#endif // AONEEVENT_H
