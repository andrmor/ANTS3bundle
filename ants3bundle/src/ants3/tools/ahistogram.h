#ifndef AHISTOGRAM_H
#define AHISTOGRAM_H

#include <cstddef>
#include <vector>
#include <array>
#include <tuple>

class AHistogram1D
{
public:
    AHistogram1D(int Bins, double From, double To);
    AHistogram1D();

    void configureFromData(const std::vector<std::pair<double,double>> & VecOfPairs);

    void setBufferSize(size_t size) {BufferSize = size;}

    void fill(double x, double val = 1.0);

    int  getNumBins() const {return Bins;}
    void getLimits(double & from, double & to) const {from = From; to = To;}
    const std::vector<double> & getContent(); // [0] - underflow;  [1] - bin#0, ..., [bins] - bin#(bins-1); [bins+1] - overflow
    std::vector<double> getStat() const;    // [0] - sumVals, [1] - sumVals2, [2] - sumValX, [3] - sumValX2, [4] - # entries

    bool   initRandomGenerator(); // returns false if data size is < 3 or there are bins with negative content!
    double getRandom() const;           // returns 0 if initRandomGenerator was not yet calculated (and it returned true)
    int    getRandomBin();        // returns 0 if initRandomGenerator was not yet calculated (and it returned true)

    void clear() {} // !!!***

private:
    int    Bins = 100;
    double From = 0;
    double To   = 0;
    double DeltaBin;

    double Entries  = 0;
    double SumVal   = 0;
    double SumVal2  = 0;
    double SumValX  = 0;
    double SumValX2 = 0;

    bool   FixedRange = false;
    size_t BufferSize = 1000;

    std::vector<double> Data; // [0] - underflow, [bins+1] - overflow; size = Bins + 2
    std::vector<std::pair<double, double>> Buffer;

    std::vector<double> SumBins; // size = Bins + 1

private:
    void init();
    void fillFixed(double x, double val);
    void processBuffer();

};

// !!!*** consider making a part of AHistogram1D
class ARandomSampler
{
public:
    bool configure(const std::vector<std::pair<double,double>> & data, bool bAssumePointMeasures);
    bool isReady() const {return !SumBins.empty();}

    double getRandom() const; // if not configured or error during configure -> always returns 0

private:
    std::vector<double> LeftBounds;
    std::vector<double> Values;
    std::vector<double> SumBins;

    void clear();
};

class AHistogram2D
{
public:
    AHistogram2D(int XBins, double XFrom, double XTo, int YBins, double YFrom, double YTo);
    void setBufferSize(size_t size) {bufferSize = size;}

    void fill(double x, double y, double val = 1.0);

    void getLimits(double & Xfrom, double & Xto, double & Yfrom, double & Yto) const {Xfrom = xfrom; Xto = xto; Yfrom = yfrom; Yto = yto;}
    const std::vector< std::vector<double> > & getContent(); //[y][x]; in each X: [0] - underflow, [1] - bin#0, ..., [bins] - bin#(bins-1), [bins+1] - overflow
    const std::vector<double> getStat();    // [0] - sumVals, [1] - sumVals2, [2] - sumValX, [3] - sumValX2, [4] - sumValY, [5] - sumValY2, [6] - # entries

private:
    int    xbins;
    double xfrom;
    double xto;
    double xdeltaBin;

    int    ybins;
    double yfrom;
    double yto;
    double ydeltaBin;

    double entries = 0;
    double sumVal = 0;
    double sumVal2 = 0;
    double sumValX = 0;
    double sumValX2 = 0;
    double sumValY = 0;
    double sumValY2 = 0;

    bool   bFixedRange = true;
    size_t bufferSize = 1000;

    std::vector< std::vector<double> > data; // [y][x]; for x: [0] - underflow, [bins+1] - overflow
    std::vector<std::tuple<double, double, double>> buffer;

private:
    void fillFixed(double x, double y, double val);
    void processBuffer();

};

class AHistogram3Dfixed
{
public:
    AHistogram3Dfixed(std::array<double, 3> origin, std::array<double, 3> step, std::array<int, 3> bins);

    void fill(const std::array<double, 3> & xyz, double val = 1.0);

    const std::vector< std::vector< std::vector<double> > > & getContent() const {return Data;} //[x][y][z];
    double getEntries() const {return Entries;}
    const std::vector<double> & getStat() const {return Stats;} // See fill() for explanation

private:
    std::array<double, 3> Origin;
    std::array<double, 3> Step;
    std::array<int,    3> Bins;

    std::vector< std::vector< std::vector<double> > > Data; // [x][y][z]

    double Entries = 0;
    std::vector<double> Stats;

    bool getVoxel(const std::array<double, 3> & pos, std::array<int, 3> & index);
};

#endif // AHISTOGRAM_H
