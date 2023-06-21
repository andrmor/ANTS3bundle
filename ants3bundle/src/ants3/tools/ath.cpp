#include "ath.h"

ATH1D::ATH1D(const char *name, const char *title, int bins, double from, double to) :
    TH1D(name, title, bins, from, to) {}

ATH1D::ATH1D(const TH1D &other) :
    TH1D(other) {}

QString ATH1D::Import(double from, double to, const std::vector<double> & binContent, const std::vector<double> & stats)
{
    const int size = binContent.size();
    if (size < 3) return QString("Number of bins should be at lreast 3 (1+under+over), here it is %1").arg(size);
    if (from >= to) return "'from' has to be smaller than 'to'";
    if (stats.size() != 5) return "stats should have size of 5";

    SetBins(size-2, from, to);
    for (int i=0; i<size; i++)
        SetBinContent(i, binContent[i]);
    SetStatistic(stats);
    return "";
}

bool ATH1D::merge(const TH1D & other)
{
    int size = GetXaxis()->GetNbins();
    int otherSize = other.GetXaxis()->GetNbins();
    if (size != otherSize) return false;

    double min = GetXaxis()->GetXmin();
    double otherMin = other.GetXaxis()->GetXmin();
    if (min != otherMin) return false;

    double max = GetXaxis()->GetXmax();
    double otherMax = other.GetXaxis()->GetXmax();
    if (max != otherMax) return false;

    for (int i = 0; i < size; i++)
        AddBinContent(i, other.GetBinContent(i));

    double otherStats[20];
    other.GetStats(otherStats);
    ///  - s[0]  = sumw       s[1]  = sumw2
    ///  - s[2]  = sumwx      s[3]  = sumwx2

    fTsumw   += otherStats[0];
    fTsumw2  += otherStats[1];
    fTsumwx  += otherStats[2];
    fTsumwx2 += otherStats[3];

    SetEntries(GetEntries() + other.GetEntries());
    return true;
}

void ATH1D::setStats(double *statsArray)
{
    fTsumw   = statsArray[0];
    fTsumw2  = statsArray[1];
    fTsumwx  = statsArray[2];
    fTsumwx2 = statsArray[3];
    SetEntries(statsArray[0]);
}

void ATH1D::SetStatistic(const std::vector<double> & stats)
{
    fTsumw   = stats[0];
    fTsumw2  = stats[1];
    fTsumwx  = stats[2];
    fTsumwx2 = stats[3];
    SetEntries(stats[4]);
}

ATH2D::ATH2D(const char *name, const char *title, int xbins, double xfrom, double xto, int ybins, double yfrom, double yto) :
    TH2D(name, title, xbins, xfrom, xto, ybins, yfrom, yto){}

QString ATH2D::Import(double xfrom, double xto, double yfrom, double yto, const std::vector< std::vector<double> > & binContent, const std::vector<double> &stats)
{
    const int ysize = binContent.size();
    if (ysize < 3) return QString("Number of bins should be at lreast 3 (1+under+over), here Y dimension has %1").arg(ysize);
    const int xsize = binContent[0].size();
    if (xsize < 3) return QString("Number of bins should be at lreast 3 (1+under+over), here X dimension has %1").arg(xsize);
    if (xfrom >= xto) return "'xfrom' has to be smaller than 'xto'";
    if (yfrom >= yto) return "'yfrom' has to be smaller than 'yto'";
    if (stats.size() != 7) return "stats should have size of 7";

    SetBins(xsize-2, xfrom, xto, ysize-2, yfrom, yto);
    for (int iy=0; iy<ysize; iy++)
        for (int ix=0; ix<xsize; ix++)
            SetBinContent(ix, iy, binContent[iy][ix]);
    SetStatistic(stats);
    return "";
}

void ATH2D::SetStatistic(const std::vector<double> &stats)
{
    fTsumw   = stats[0];
    fTsumw2  = stats[1];
    fTsumwx  = stats[2];
    fTsumwx2 = stats[3];
    fTsumwy  = stats[4];
    fTsumwy2 = stats[5];
    SetEntries(stats[6]);
}

// ---

ATH3D::ATH3D(const char * name, const char * title, int xbins, double xfrom, double xto, int ybins, double yfrom, double yto, int zbins, double zfrom, double zto) :
    TH3D(name, title, xbins, xfrom, xto, ybins, yfrom, yto, zbins, zfrom, zto) {}

void ATH3D::setStatistics(const std::array<double, 11> & stats)
{
    fTsumw   = stats[0];
    fTsumw2  = stats[1];
    fTsumwx  = stats[2];
    fTsumwx2 = stats[3];
    fTsumwy  = stats[4];
    fTsumwy2 = stats[5];
    fTsumwxy = stats[6];
    fTsumwz  = stats[7];
    fTsumwz2 = stats[8];
    fTsumwxz = stats[9];
    fTsumwyz = stats[10];
}
