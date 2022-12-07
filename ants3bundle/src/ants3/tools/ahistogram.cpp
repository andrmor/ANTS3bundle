#include "ahistogram.h"

#ifdef GEANT4
#include "arandomg4hub.h"
#else
#include "arandomhub.h"
#endif

#include <algorithm>
#include <limits>
#include <tuple>
#include <cmath>

//#include <QDebug>

AHistogram1D::AHistogram1D(int Bins, double From, double To) :
    Bins(Bins), From(From), To(To)
{
    init();
}

void AHistogram1D::init()
{
    if (Bins < 1) Bins = 1;
    Data.resize(Bins + 2);

    if (To > From)
    {
        DeltaBin = (To - From) / Bins;
        FixedRange = true;
    }
    else
    {
        Buffer.reserve(BufferSize);
        FixedRange = false;
    }
}

AHistogram1D::AHistogram1D()
{
    init();
}

void AHistogram1D::configureFromData(const std::vector<std::pair<double, double>> & VecOfPairs)
{
    Bins = VecOfPairs.size();
    if (!VecOfPairs.empty())
    {
        From = VecOfPairs.front().first;
        To = VecOfPairs.back().first;
    }

    init();

    for (size_t i = 0; i < VecOfPairs.size(); i++)
    {
        Data[i+1] = VecOfPairs[i].second;
    }
}

void AHistogram1D::fill(double x, double val)
{
    Entries += 1.0;

    if (FixedRange)
        fillFixed(x, val);
    else
    {
        Buffer.push_back( std::pair<double,double>(x, val) );
        if (Buffer.size() == BufferSize)
            processBuffer();
    }
}

const std::vector<double> &AHistogram1D::getContent()
{
    if (!FixedRange) processBuffer();
    return Data;
}

std::vector<double> AHistogram1D::getStat() const
{
    return {SumVal, SumVal2, SumValX, SumValX2, Entries};
}

bool AHistogram1D::initRandomGenerator()
{
    if (!FixedRange) processBuffer();

    const size_t size = Data.size();
    if (size < 3) return false; //over and underflow!

    SumBins.resize(size - 1);

    double sum = 0;
    SumBins[0] = 0;
    for (size_t i = 1; i < size-1; i++)
    {
        if (Data[i] < 0) return false;
        sum += Data[i];
        SumBins[i] = sum;
    }
    //qDebug() << Data << SumBins;
    return true;
}

double AHistogram1D::getRandom() const
{
    if (SumBins.empty()) return 0;

    double integral = SumBins.back();
    if (integral == 0) return 0;

    double r1 = integral * ARandomHub::getInstance().uniform();
    int ibin = std::upper_bound(SumBins.begin(), SumBins.end(), r1) - SumBins.begin() - 1;

    double x = From + ibin * DeltaBin;
    if (r1 > SumBins[ibin])
        x += DeltaBin * (r1 - SumBins[ibin]) / (SumBins[ibin+1] - SumBins[ibin]);

    return x;
}

int AHistogram1D::getRandomBin()
{
    if (SumBins.empty()) return 0;

    double integral = SumBins.back();
    if (integral == 0) return 0;

    double r1 = integral * ARandomHub::getInstance().uniform();
    return std::upper_bound(SumBins.begin(), SumBins.end(), r1) - SumBins.begin() - 1;
}

void AHistogram1D::fillFixed(double x, double val)
{
    if (x < From)
        Data[0] += val;
    else if (x > To)
        Data[Bins+1] += val;
    else
    {
        Data[ 1 + (x - From)/DeltaBin ] += val;

        SumVal   += val;
        SumVal2  += val*val;
        SumValX  += val*x;
        SumValX2 += val*x*x;
    }
}

void AHistogram1D::processBuffer()
{
    if (Buffer.empty()) return;

    From = To = Buffer[0].first;
    for (size_t i=1; i<Buffer.size(); i++)
    {
        const double & x = Buffer[i].first;
        if      (x < From) From = x;
        else if (x > To)   To   = x;
    }

    if (From == To)
    {
        From *= 0.995;
        To   *= 1.005;
    }

    DeltaBin = (To - From) / Bins + std::numeric_limits<double>::epsilon();

    for (size_t i=0; i<Buffer.size(); i++)
    {
        const double & x   = Buffer[i].first;
        const double & val = Buffer[i].second;
        fillFixed(x, val);
    }

    FixedRange = true;
}

AHistogram2D::AHistogram2D(int XBins, double XFrom, double XTo, int YBins, double YFrom, double YTo) :
    xbins(XBins), xfrom(XFrom), xto(XTo), ybins(YBins), yfrom(YFrom), yto(YTo)
{
    if (xbins < 1) xbins = 1;
    if (ybins < 1) ybins = 1;
    data.resize(ybins + 2);
    for (auto & v : data)
        v.resize(xbins + 2);

    if (xto > xfrom && yto > yfrom)
    {
        xdeltaBin = (xto - xfrom) / xbins;
        ydeltaBin = (yto - yfrom) / ybins;
    }
    else
    {
        bFixedRange = false;
        buffer.reserve(bufferSize);
    }
}

void AHistogram2D::fill(double x, double y, double val)
{
    entries += 1.0;

    if (bFixedRange)
        fillFixed(x, y, val);
    else
    {
        buffer.push_back( std::tuple<double,double,double>(x, y, val) );
        if (buffer.size() == bufferSize)
            processBuffer();
    }
}

const std::vector<std::vector<double> > & AHistogram2D::getContent()
{
    if (!bFixedRange) processBuffer();
    return data;
}

const std::vector<double> AHistogram2D::getStat()
{
    return {sumVal, sumVal2, sumValX, sumValX2, sumValY, sumValY2, entries};
}

void AHistogram2D::fillFixed(double x, double y, double val)
{
    int ixbin, iybin;
    bool bGood = true;

    if      (x < xfrom)
    {
        ixbin = 0;
        bGood = false;
    }
    else if (x > xto)
    {
        ixbin = xbins + 1;
        bGood = false;
    }
    else
        ixbin = 1 + (x - xfrom)/xdeltaBin;

    if      (y < yfrom)
    {
        iybin = 0;
        bGood = false;
    }
    else if (y > yto)
    {
        iybin = ybins + 1;
        bGood = false;
    }
    else
        iybin = 1 + (y - yfrom)/ydeltaBin;

    data[iybin][ixbin] += val;

    if (bGood)
    {
        sumVal   += val;
        sumVal2  += val*val;
        sumValX  += val*x;
        sumValX2 += val*x*x;
        sumValY  += val*y;
        sumValY2 += val*y*y;
    }
}

void AHistogram2D::processBuffer()
{
    if (buffer.empty()) return;

    xfrom = xto = std::get<0>(buffer[0]);
    yfrom = yto = std::get<1>(buffer[0]);
    for (size_t i=1; i<buffer.size(); i++)
    {
        const double & x = std::get<0>(buffer[i]);
        const double & y = std::get<1>(buffer[i]);
        if      (x < xfrom) xfrom = x;
        else if (x > xto)   xto   = x;
        if      (y < yfrom) yfrom = y;
        else if (y > yto)   yto   = y;
    }

    xdeltaBin = (xto - xfrom) / xbins + std::numeric_limits<double>::epsilon();
    ydeltaBin = (yto - yfrom) / ybins + std::numeric_limits<double>::epsilon();

    for (size_t i=0; i<buffer.size(); i++)
    {
        const double & x   = std::get<0>(buffer[i]);
        const double & y   = std::get<1>(buffer[i]);
        const double & val = std::get<2>(buffer[i]);
        fillFixed(x, y, val);
    }

    bFixedRange = true;
}

// ---

AHistogram3Dfixed::AHistogram3Dfixed(std::array<double, 3> origin, std::array<double, 3> step, std::array<int, 3> bins) :
    Origin(origin), Step(step), Bins(bins)
{
    Data.resize(Bins[0]);
    for (std::vector<std::vector<double>> & ary : Data)
    {
        ary.resize(Bins[1]);
        for (std::vector<double> & arz : ary)
            arz = std::vector<double>(Bins[2], 0);
    }

    Stats.resize(11);
}

void AHistogram3Dfixed::fill(const std::array<double, 3> & pos, double val)
{
    std::array<int,3> index; // on stack, fast
    const bool ok = getVoxel(pos, index);
    if (!ok) return;

    Data[index[0]][index[1]][index[2]] += val;

    Entries += 1.0;

    /* // GetStats() of TH3D
    s[0] = sumw s[1] = sumw2
    s[2] = sumwx s[3] = sumwx2
    s[4] = sumwy s[5] = sumwy2 s[6] = sumwxy
    s[7] = sumwz s[8] = sumwz2 s[9] = sumwxz s[10] = sumwyz
    */
    //if (bGood)
    {
        Stats[0]  += val;
        Stats[1]  += val * val;
        Stats[2]  += val * pos[0];
        Stats[3]  += val * pos[0]*pos[0];
        Stats[4]  += val * pos[1];
        Stats[5]  += val * pos[1]*pos[1];
        Stats[6]  += val * pos[0]*pos[1];
        Stats[7]  += val * pos[2];
        Stats[8]  += val * pos[2]*pos[2];
        Stats[9]  += val * pos[0]*pos[2];
        Stats[10] += val * pos[1]*pos[2];
    }
}

bool AHistogram3Dfixed::getVoxel(const std::array<double,3> & pos, std::array<int,3> & index)
{
    for (int i = 0; i < 3; i++)
    {
        index[i] = floor( (pos[i] - Origin[i]) / Step[i] );
        if ( index[i] < 0 || index[i] >= Bins[i]) return false;
    }
    return true;
}

// ---

bool ARandomSampler::configure(const std::vector<std::pair<double, double>> & data, bool bAssumePointMeasures)
{
    clear();

    const size_t dataSize = data.size();
    if (dataSize < 2) return false;

    LeftBounds.resize(dataSize);
    Values.resize(dataSize - 1);
    SumBins.resize(dataSize - 1);

    double sum = 0;
    for (size_t i = 0; i < dataSize-1; i++)
    {
        const double x = data[i].first;
        if (i != 0 && x <= data[i-1].first) // only continuously increasing bin bounds
        {
            clear();
            return false;
        }
        LeftBounds[i] = x;

        const double val = data[i].second;
        if (val < 0) // non-negative values
        {
            clear();
            return false;
        }
        Values[i] = val;

        if (bAssumePointMeasures) sum += Values[i] * (data[i+1].first - data[i].first);
        else                      sum += Values[i];
        SumBins[i] = sum;
    }
    LeftBounds.back() = data.back().first;

    if (SumBins.back() == 0) // integral cannot be zero
    {
        clear();
        return false;
    }

    return true;
}

void ARandomSampler::clear()
{
    LeftBounds.clear();
    Values.clear();
    SumBins.clear();
}

double ARandomSampler::getRandom() const
{
    if (SumBins.empty()) return 0;

    const double r1 = SumBins.back() * ARandomHub::getInstance().uniform(); // integral * random[0,1)
    int ibin = std::upper_bound(SumBins.begin(), SumBins.end(), r1) - SumBins.begin() - 1;

    const double From = LeftBounds[ibin];
    const double To   = LeftBounds[ibin+1];

    double x = From;
    if (r1 > SumBins[ibin])
        x += (To - From) * (r1 - SumBins[ibin]) / (SumBins[ibin+1] - SumBins[ibin]);

    return x;
}
