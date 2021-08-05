#ifndef AONEEVENT_H
#define AONEEVENT_H

#include <QVector>
#include <QBitArray>

class ASensorHub;
class ARandomHub;
class APhotonSimSettings;
class ASimulationStatistics;

class AOneEvent
{
public:
    AOneEvent(ASimulationStatistics * simStat);

    QVector<float>          PMhits;           // PM hits [pm]
    QVector<float>          PMsignals;        // -- converted to signal [pm]
    QVector<QBitArray>      SiPMpixels;       //on/off status of SiPM pixels [PM#] [time] [pixY] [pixX]

    ASimulationStatistics * SimStat;

    void configure();

    //hits processing
    void clearHits();
    bool isHitsEmpty() const;
    bool CheckPMThit(int ipm, double time, int WaveIndex, double x, double y, double cosAngle, int Transitions, double rnd);
    bool CheckSiPMhit(int ipm, double time, int WaveIndex, double x, double y, double cosAngle, int Transitions, double rnd);
    void HitsToSignal();  //convert hits of PMs to signal using electronics settings

    //  void addHits(int ipm, float hits) {PMhits[ipm] += hits;}
    void addSignals(int ipm, float signal) {PMsignals[ipm] += signal;}  //only used in LRF-based sim

    void CollectStatistics(int WaveIndex, double time, double cosAngle, int Transitions);

private:
    const APhotonSimSettings & SimSet;
    const ASensorHub         & SensorHub;
    ARandomHub         & RandomHub;

    //settings
    int numPMs;

    void  registerSiPMhit(int ipm, int iTime, int binX, int binY, float numHits = 1.0f); // numHits != 1 for two cases: 1) simplistic model of microcell cross-talk  2) advanced model of dark counts
    void  AddDarkCounts();
    void  convertHitsToSignal(const QVector<float>& pmHits, QVector<float>& pmSignals);
    float generateDarkHitIncrement(int ipm) const;
};

#endif // AONEEVENT_H
