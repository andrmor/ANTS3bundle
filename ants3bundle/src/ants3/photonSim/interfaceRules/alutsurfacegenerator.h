#ifndef ALUTSURFACEGENERATOR_H
#define ALUTSURFACEGENERATOR_H

#include <QString>

#include <vector>
#include <functional>

class ALutSurfaceData;
class ARandomHub;

// Generates surface look-up tables for the "DavisLUT" interface rule by ray tracing over
// a 3D surface topography Z(x,y), e.g. measured with AFM
// (approach of E.Roncali and S.R.Cherry, Phys.Med.Biol. 58 (2013) 2185).
//
// Geometry conventions:
//  * the medium the photons arrive from (refractive index n1, e.g. the scintillator)
//    occupies z < Z(x,y); the medium behind the surface (n2) is above;
//  * photons are launched upward from below the surface with polar angle thetaInc
//    (bin centers) and azimuth phi0 (random by default);
//  * at each intersection with the surface the local facet normal is used to compute the
//    unpolarized Fresnel reflection probability; the photon is specularly reflected or
//    Snell-refracted about the LOCAL normal and traced further, so multiple reflections,
//    shadowing and masking are modelled directly;
//  * the surface is tiled periodically in x and y (note: a generic heightmap is not
//    exactly periodic, so there is a seam at the tile border - negligible for
//    statistically uniform roughness);
//  * a photon escaping downward (in medium 1) is tallied as reflected, escaping upward
//    (in medium 2) as transmitted. Outgoing azimuth is stored relative to the incidence
//    plane in the frame ex = tangential projection of the incident direction,
//    ey = meanNormal x ex, where meanNormal (+z here) points along the initial photon
//    direction: the same frame ALutInterfaceRule reconstructs at runtime.

class ALutSurfaceGenerator
{
public:
    ALutSurfaceGenerator();

    // --- configuration ---
    double n1 = 1.82;                  // refractive index below the surface (photons start here)
    double n2 = 1.0;                   // refractive index above the surface
    double wavelength = 420.0;         // [nm] stored as LUT metadata only
    int    thetaIncBins = 40;
    int    thetaOutBins = 45;
    int    phiOutBins   = 36;
    int    photonsPerThetaBin = 80000;
    int    phiSteps   = 0;             // 0 = random incidence azimuth (default); >0 = that many discrete azimuth steps
    int    maxBounces = 100;
    int    seed = 0;                   // 0 = keep the current state of the random generator
    // false (default): the n1 medium (where the photons start) is BELOW the heightmap surface,
    //   e.g. photons inside the crystal hitting its AFM-measured surface;
    // true: the n1 medium is ABOVE the same physical surface (e.g. photons in the optical
    //   gap re-entering the crystal): the generator internally mirrors the heightmap and
    //   the tallied azimuth so that the runtime frame convention of ALutInterfaceRule holds
    bool   reverseGeometry = false;
    QString comment;

    std::function<bool(int)> progressCallback;   // argument: percent done; return false to abort

    // --- heightmap input ---
    // text file with a matrix of heights: rows <-> y, columns <-> x; any length units,
    // as long as the pixel size is given in the same units
    QString loadHeightmapMatrix(const QString & fileName, double pixelSizeX, double pixelSizeY);
    // text file with three columns (x y z) forming a regular grid in x and y
    QString loadHeightmapXYZ(const QString & fileName);

    // --- run ---
    QString generate(ALutSurfaceData & result);  // returns error string, empty on success

    // --- statistics of the last run ---
    double meanBounces() const {return MeanBounces;}
    long   anomalies()   const {return Anomalies;}  // photons lost due to numerical issues / bounce limit
    long   wraps()       const {return Wraps;}      // periodic wrap-around events
    // diagnostics: photons whose reflect/refract medium flag disagreed with the actual escape
    // geometry after steep multi-bounce (classified by direction instead), and truly degenerate ones:
    long   upEscapeReclassified()   const {return UpEscapeReclass;}    // labelled crystal but escaped upward -> counted as transmitted
    long   downEscapeReclassified() const {return DownEscapeReclass;}  // labelled air but escaped downward -> counted as reflected
    long   degenerateDiscarded()    const {return DegenerateDiscarded;} // exactly horizontal escape, discarded

private:
    ARandomHub & RandomHub;

    // heightmap: Z[iy][ix], Nx x Ny grid points, pixel size Dx x Dy
    std::vector<std::vector<double>> Z;
    int     Nx = 0, Ny = 0;
    double  Dx = 0, Dy = 0;
    double  Zmin = 0, Zmax = 0;
    QString HeightmapFileName;

    double MeanBounces = 0;
    long   Anomalies = 0;
    long   Wraps = 0;
    long   UpEscapeReclass = 0, DownEscapeReclass = 0, DegenerateDiscarded = 0;

    QString validateConfig() const;
    QString finalizeHeightmap();   // computes Zmin/Zmax, validates the grid

    // traces one photon launched at 'pos' with direction 'dir' (unit);
    // returns +1 = escaped as reflected, -1 = escaped as transmitted, 0 = anomaly;
    // on return 'dir' holds the escape direction
    int  tracePhoton(double * pos, double * dir, int & bounces);

    // 2D DDA walk over the heightmap cells (periodic in x,y) searching for the nearest
    // triangle intersection; returns true and the distance/facet normal (not flipped)
    bool findIntersection(const double * origin, const double * dir,
                          double & tHit, double * hitNormal);

    // Moller-Trumbore ray-triangle intersection; accepts hits with t > tMin
    static bool intersectTriangle(const double * origin, const double * dir,
                                  const double * v0, const double * v1, const double * v2,
                                  double tMin, double & t);

    static double fresnelReflection(double cosI, double nFrom, double nTo); // unpolarized, 1.0 in case of TIR
};

#endif // ALUTSURFACEGENERATOR_H
