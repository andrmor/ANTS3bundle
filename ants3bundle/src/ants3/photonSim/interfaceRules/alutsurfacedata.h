#ifndef ALUTSURFACEDATA_H
#define ALUTSURFACEDATA_H

#include <QString>

#include <vector>

class QJsonObject;

// Container for the surface look-up tables of the "DavisLUT" interface rule
// (approach of Roncali & Cherry 2013, Phys.Med.Biol. 58:2185):
// for each incidence-angle bin, the probabilities for the photon to be reflected /
// transmitted / absorbed and the 2D angular distributions (thetaOut x phiOut) of the
// escaping rays. PhiOut is defined relative to the incidence plane.
// The tables are direction-specific: valid only for photons flying from the medium
// with refractive index n1 towards the medium with n2.
// Filled by ALutSurfaceGenerator, used at runtime by ALutInterfaceRule.

class ALutSurfaceData
{
public:
    static constexpr int CurrentFormatVersion = 1;

    // --- metadata ---
    double  n1 = 1.82;                // refractive index of the medium the photons come from
    double  n2 = 1.0;                 // refractive index of the medium behind the surface
    double  Wavelength = 420.0;       // [nm] wavelength assumed during generation
    QString SourceHeightmap;          // file name of the heightmap used for generation
    double  PixelSizeX = 0;           // heightmap pixel size (same length units as heights)
    double  PixelSizeY = 0;
    int     GridSizeX = 0;            // heightmap grid dimensions
    int     GridSizeY = 0;
    int     PhotonsPerThetaBin = 0;
    int     Seed = 0;
    int     MaxBounces = 0;
    double  MeanBounces = 0;          // generation statistics
    int     AnomalyCount = 0;         // photons discarded during generation (lost rays / bounce limit)
    QString GenerationDate;
    QString Comment;

    // --- binning ---
    int ThetaIncBins = 40;            // incidence angle, [0, 90) degrees, bin centers at (i+0.5)*width
    int ThetaOutBins = 45;            // outgoing polar angle from the mean-surface normal, [0, 90] degrees
    int PhiOutBins   = 36;            // outgoing azimuth relative to the incidence plane, [0, 360) degrees

    // --- data: outer index = incidence angle bin ---
    std::vector<int> Launched;
    std::vector<int> ReflectedCounts;
    std::vector<int> TransmittedCounts;
    std::vector<int> AbsorbedCounts;
    std::vector<std::vector<int>> ReflectedHist;    // flattened 2D: [iThetaOut * PhiOutBins + iPhiOut]
    std::vector<std::vector<int>> TransmittedHist;

    bool isLoaded() const {return !Launched.empty();}
    void clear();

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);  // returns error string, empty on success

    QString check() const;      // consistency of binning, array sizes and count conservation
    QString buildRuntime();     // check() + prepare R/T probabilities and sampling CDFs

    // --- runtime access, valid only after successful buildRuntime() ---
    bool   isRuntimeReady() const {return RuntimeReady;}
    double getReflectionProbability(int iThetaBin) const   {return R[iThetaBin];}
    double getTransmissionProbability(int iThetaBin) const {return T[iThetaBin];}

    // incidence bin for a given angle [deg]: stochastic interpolation between the two
    // adjacent bin centers (rnd = uniform [0,1)), clamped at the first/last bin
    int selectThetaBin(double thetaDeg, double rnd) const;

    // samples outgoing direction [deg] from the reflected or transmitted distribution of the given
    // incidence bin using three uniform [0,1) random numbers (bin selection + within-bin smearing);
    // returns false if the corresponding distribution is empty
    bool sampleOutgoing(bool reflected, int iThetaBin, double rnd1, double rnd2, double rnd3,
                        double & thetaOut, double & phiOut) const;

private:
    // runtime data (never serialized)
    std::vector<double> R;    // reflection probability per incidence bin
    std::vector<double> T;    // transmission probability per incidence bin
    std::vector<std::vector<double>> ReflCdf;   // normalized cumulative distributions over the
    std::vector<std::vector<double>> TransCdf;  // flattened (thetaOut, phiOut) bins; empty if no counts
    bool RuntimeReady = false;
};

#endif // ALUTSURFACEDATA_H
