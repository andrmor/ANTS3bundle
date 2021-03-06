#include "apeakfinder.h"

#include "TSpectrum.h"

APeakFinder::APeakFinder(const TH1 *hist) : H(hist) {}

const std::vector<double> APeakFinder::findPeaks(const double sigma, const double threshold, const int MaxNumberOfPeaks, bool SuppressDraw) const
{
    std::vector<double> peaks;
    if (!H) return peaks;

    TSpectrum * s = new TSpectrum(MaxNumberOfPeaks);
    const int numPeaks = s->Search(H, sigma, (SuppressDraw ? "goff nodraw" : ""), threshold);

#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    double * pos = s->GetPositionX();
#else
    float * pos  = s->GetPositionX();
#endif

    for (int i = 0; i < numPeaks; i++)
        peaks.push_back(pos[i]);
    return peaks;
}
