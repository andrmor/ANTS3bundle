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

bool ATH1D::mergeIdentical(const TH1D & other)
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

    if (min >= max) return false; // range is not explicitly defined

    for (int i = 0; i < size+2; i++)
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
    SetEntries(statsArray[4]);
}

void ATH1D::setStats(const std::array<double, 5> & statsArray)
{
    fTsumw   = statsArray[0];
    fTsumw2  = statsArray[1];
    fTsumwx  = statsArray[2];
    fTsumwx2 = statsArray[3];
    SetEntries(statsArray[4]);
}

void ATH1D::merge(TH1D* & to, TH1D* const & from)
{
    if (to->GetEntries() == 0)
    {
        delete to; to = new TH1D(*from);
        return;
    }

    if (to->GetXaxis()->GetXmin() < to->GetXaxis()->GetXmax())
    {
        ATH1D * ahist = new ATH1D(*to);
        bool ok = ahist->mergeIdentical(*from);
        if (ok)
        {
            delete to;
            to = ahist;
            return;
        }

        delete ahist;
        // then using the general case below
    }

    // general case: not fully compatible histograms
    int numEv = to->GetEntries();
    for (int i = 1; i <= from->GetNbinsX(); i++)
        to->Fill(from->GetBinCenter(i), from->GetBinContent(i));
    to->SetEntries(numEv + from->GetEntries());
}

void ATH1D::mergeFrom(const ATH1D * other)
{
    bool ok = mergeIdentical(*other);
    if (ok) return;

    // the histograms are no identical, e.g. they have auto-range
    int numEv = GetEntries();
    for (int i = 1; i <= other->GetNbinsX(); i++)
        Fill(other->GetBinCenter(i), other->GetBinContent(i));

    SetEntries(numEv + other->GetEntries());
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

ATH2D::ATH2D(const TH2D &other) :
    TH2D(other) {}

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

bool ATH2D::mergeIdentical(const TH2D & other)
{
    int sizeX = GetXaxis()->GetNbins();
    int otherSizeX = other.GetXaxis()->GetNbins();
    if (sizeX != otherSizeX) return false;

    int sizeY = GetYaxis()->GetNbins();
    int otherSizeY = other.GetYaxis()->GetNbins();
    if (sizeY != otherSizeY) return false;

    double minX = GetXaxis()->GetXmin();
    double otherMinX = other.GetXaxis()->GetXmin();
    if (minX != otherMinX) return false;
    double maxX = GetXaxis()->GetXmax();
    double otherMaxX = other.GetXaxis()->GetXmax();
    if (maxX != otherMaxX) return false;

    if (minX >= maxX) return false; // not explicitly defined X range

    double minY = GetYaxis()->GetXmin();
    double otherMinY = other.GetYaxis()->GetXmin();
    if (minY != otherMinY) return false;
    double maxY = GetYaxis()->GetXmax();
    double otherMaxY = other.GetYaxis()->GetXmax();
    if (maxY != otherMaxY) return false;

    if (minY >= maxY) return false; // not explicitly defined Y range

    for (int ix = 0; ix < sizeX+2; ix++)
        for (int iy = 0; iy < sizeY+2; iy++)
        {
            int iBin = GetBin(ix, iy);
            AddBinContent(iBin, other.GetBinContent(iBin));
        }

    double otherStats[20];
    other.GetStats(otherStats);
    ///  - s[0]  = sumw       s[1]  = sumw2
    ///  - s[2]  = sumwx      s[3]  = sumwx2
    ///  - s[4]  = sumwy      s[5]  = sumwy2   s[6]  = sumwxy
    ///  - s[7]  = sumwz      s[8]  = sumwz2   s[9]  = sumwxz   s[10]  = sumwyz
    fTsumw   += otherStats[0];
    fTsumw2  += otherStats[1];
    fTsumwx  += otherStats[2];
    fTsumwx2 += otherStats[3];
    fTsumwy  += otherStats[4];
    fTsumwy2 += otherStats[5];
    fTsumwxy += otherStats[6];

    SetEntries(GetEntries() + other.GetEntries());
    return true;
}

void ATH2D::merge(TH2D* & to, TH2D* const & from)
{
    if (to->GetEntries() == 0)
    {
        delete to; to = new TH2D(*from);
        return;
    }

    // histograms with explicitly defined (and the same) binning
    if (to->GetXaxis()->GetXmin() < to->GetXaxis()->GetXmax() &&
        to->GetYaxis()->GetXmin() < to->GetYaxis()->GetXmax())
        {
            ATH2D * ahist2D = new ATH2D(*to);
            bool ok = ahist2D->mergeIdentical(*from);
            if (ok)
            {
                delete to;
                to = ahist2D;
                return;
            }

            delete ahist2D;
            // then using the general case below
        }

    // general case: not fully compatible histograms
    int numEv = to->GetEntries();
    for (int ix = 1; ix <= from->GetNbinsX(); ix++)
    {
        const double X = from->GetXaxis()->GetBinCenter(ix);
        for (int iy = 1; iy <= from->GetNbinsY(); iy++)
                to->Fill(X, from->GetYaxis()->GetBinCenter(iy), from->GetBinContent(from->GetBin(ix, iy)));
    }
    to->SetEntries(numEv + from->GetEntries());
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
