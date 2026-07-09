#include "alutsurfacegenerator.h"
#include "alutsurfacedata.h"
#include "arandomhub.h"
#include "afiletools.h"

#include <QFileInfo>
#include <QDateTime>

#include <algorithm>
#include <cmath>

ALutSurfaceGenerator::ALutSurfaceGenerator() :
    RandomHub(ARandomHub::getInstance()) {}

QString ALutSurfaceGenerator::loadHeightmapMatrix(const QString & fileName, double pixelSizeX, double pixelSizeY)
{
    if (pixelSizeX <= 0 || pixelSizeY <= 0) return "Pixel size should be positive";

    QString err = ftools::loadMatrix(fileName, Z);
    if (!err.isEmpty()) return err;

    Dx = pixelSizeX;
    Dy = pixelSizeY;
    HeightmapFileName = fileName;
    return finalizeHeightmap();
}

QString ALutSurfaceGenerator::loadHeightmapXYZ(const QString & fileName)
{
    std::vector<double> xs, ys, zs;
    std::vector<std::vector<double>*> vec = {&xs, &ys, &zs};
    QString err = ftools::loadDoubleVectorsFromFile(fileName, vec);
    if (!err.isEmpty()) return err;
    if (xs.size() < 4) return "Too few data points in the heightmap file";

    auto extractAxis = [](const std::vector<double> & values, std::vector<double> & nodes) -> QString
    {
        std::vector<double> sorted = values;
        std::sort(sorted.begin(), sorted.end());
        const double range = sorted.back() - sorted.front();
        if (range <= 0) return "all coordinates are the same";

        // smallest positive gap defines the merge tolerance
        double minGap = range;
        for (size_t i = 1; i < sorted.size(); i++)
        {
            const double gap = sorted[i] - sorted[i-1];
            if (gap > 1e-9 * range && gap < minGap) minGap = gap;
        }
        const double tolerance = 0.5 * minGap;

        nodes.clear();
        for (double v : sorted)
            if (nodes.empty() || v - nodes.back() > tolerance) nodes.push_back(v);
        if (nodes.size() < 2) return "grid should have at least two nodes";

        const double step = (nodes.back() - nodes.front()) / (nodes.size() - 1);
        for (size_t i = 1; i < nodes.size(); i++)
            if (std::fabs(nodes[i] - nodes[i-1] - step) > 0.01 * step) return "grid nodes are not uniformly spaced";
        return "";
    };

    std::vector<double> xNodes, yNodes;
    err = extractAxis(xs, xNodes); if (!err.isEmpty()) return "Bad heightmap x axis: " + err;
    err = extractAxis(ys, yNodes); if (!err.isEmpty()) return "Bad heightmap y axis: " + err;

    const size_t nx = xNodes.size();
    const size_t ny = yNodes.size();
    if (nx * ny != xs.size())
        return QString("Heightmap is not a full regular grid: %1 x %2 nodes but %3 points").arg(nx).arg(ny).arg(xs.size());

    const double stepX = (xNodes.back() - xNodes.front()) / (nx - 1);
    const double stepY = (yNodes.back() - yNodes.front()) / (ny - 1);

    Z.assign(ny, std::vector<double>(nx, 0));
    std::vector<std::vector<bool>> filled(ny, std::vector<bool>(nx, false));
    for (size_t i = 0; i < xs.size(); i++)
    {
        const int ix = (int)std::round((xs[i] - xNodes.front()) / stepX);
        const int iy = (int)std::round((ys[i] - yNodes.front()) / stepY);
        if (ix < 0 || ix >= (int)nx || iy < 0 || iy >= (int)ny) return "Heightmap grid mapping error";
        if (filled[iy][ix]) return "Heightmap contains duplicated grid points";
        Z[iy][ix] = zs[i];
        filled[iy][ix] = true;
    }

    Dx = stepX;
    Dy = stepY;
    HeightmapFileName = fileName;
    return finalizeHeightmap();
}

QString ALutSurfaceGenerator::finalizeHeightmap()
{
    Ny = Z.size();
    if (Ny < 2) return "Heightmap should have at least 2 rows";
    Nx = Z.front().size();
    if (Nx < 2) return "Heightmap should have at least 2 columns";
    for (const std::vector<double> & row : Z)
        if ((int)row.size() != Nx) return "Heightmap rows have different lengths";

    Zmin = Zmax = Z[0][0];
    for (const std::vector<double> & row : Z)
        for (double z : row)
        {
            if (z < Zmin) Zmin = z;
            if (z > Zmax) Zmax = z;
        }
    return "";
}

QString ALutSurfaceGenerator::validateConfig() const
{
    if (Z.empty()) return "Heightmap is not loaded";
    if (n1 <= 0 || n2 <= 0) return "Refractive indices should be positive";
    if (thetaIncBins < 1 || thetaIncBins > 1000) return "thetaIncBins should be in [1, 1000]";
    if (thetaOutBins < 1 || thetaOutBins > 1000) return "thetaOutBins should be in [1, 1000]";
    if (phiOutBins   < 1 || phiOutBins   > 1000) return "phiOutBins should be in [1, 1000]";
    if (photonsPerThetaBin < 100) return "photonsPerThetaBin should be at least 100";
    if (maxBounces < 1) return "maxBounces should be positive";
    if (phiSteps < 0) return "phiSteps cannot be negative";
    return "";
}

double ALutSurfaceGenerator::fresnelReflection(double cosI, double nFrom, double nTo)
{
    // unpolarized Fresnel reflection, same formulation as in FsnpInterfaceRule::calculate()
    const double sin2I = 1.0 - cosI*cosI;
    double nsqr = nTo / nFrom; nsqr *= nsqr;
    if (sin2I > nsqr) return 1.0;   // total internal reflection

    const double f1 = nsqr * cosI;
    const double f2 = sqrt(nsqr - sin2I);
    double Rp = (f1 - f2) / (f1 + f2);     Rp *= Rp;
    double Rs = (cosI - f2) / (cosI + f2); Rs *= Rs;
    return 0.5 * (Rp + Rs);
}

bool ALutSurfaceGenerator::intersectTriangle(const double * origin, const double * dir,
                                             const double * v0, const double * v1, const double * v2,
                                             double tMin, double & t)
{
    const double e1[3] = {v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]};
    const double e2[3] = {v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]};

    const double p[3] = {dir[1]*e2[2] - dir[2]*e2[1],
                         dir[2]*e2[0] - dir[0]*e2[2],
                         dir[0]*e2[1] - dir[1]*e2[0]};
    const double det = e1[0]*p[0] + e1[1]*p[1] + e1[2]*p[2];
    if (det == 0) return false;
    const double invDet = 1.0 / det;

    const double s[3] = {origin[0]-v0[0], origin[1]-v0[1], origin[2]-v0[2]};
    const double u = (s[0]*p[0] + s[1]*p[1] + s[2]*p[2]) * invDet;
    if (u < -1e-9 || u > 1.0 + 1e-9) return false;   // small tolerance to avoid cracks along shared edges

    const double q[3] = {s[1]*e1[2] - s[2]*e1[1],
                         s[2]*e1[0] - s[0]*e1[2],
                         s[0]*e1[1] - s[1]*e1[0]};
    const double v = (dir[0]*q[0] + dir[1]*q[1] + dir[2]*q[2]) * invDet;
    if (v < -1e-9 || u + v > 1.0 + 1e-9) return false;

    t = (e2[0]*q[0] + e2[1]*q[1] + e2[2]*q[2]) * invDet;
    return (t > tMin);
}

bool ALutSurfaceGenerator::findIntersection(const double * origin, const double * dir,
                                            double & tHit, double * hitNormal)
{
    const int cellsX = Nx - 1;
    const int cellsY = Ny - 1;
    const double tMin = 1e-9 * (Dx + Dy);

    int ix = (int)std::floor(origin[0] / Dx);
    int iy = (int)std::floor(origin[1] / Dy);

    const int stepX = (dir[0] > 0 ? 1 : (dir[0] < 0 ? -1 : 0));
    const int stepY = (dir[1] > 0 ? 1 : (dir[1] < 0 ? -1 : 0));

    double tMaxX = 1e300, tDeltaX = 0;
    if (stepX != 0)
    {
        const double border = (stepX > 0 ? (ix + 1) * Dx : ix * Dx);
        tMaxX   = (border - origin[0]) / dir[0];
        tDeltaX = Dx / std::fabs(dir[0]);
    }
    double tMaxY = 1e300, tDeltaY = 0;
    if (stepY != 0)
    {
        const double border = (stepY > 0 ? (iy + 1) * Dy : iy * Dy);
        tMaxY   = (border - origin[1]) / dir[1];
        tDeltaY = Dy / std::fabs(dir[1]);
    }

    int periodX = 0, periodY = 0;   // for wrap statistics

    const long maxCells = 100L * (cellsX + cellsY) + 1000;   // safety cap for near-horizontal rays
    for (long counter = 0; counter < maxCells; counter++)
    {
        // heights are taken with periodically wrapped indices, positions stay unwrapped
        int wx = ix % cellsX; if (wx < 0) wx += cellsX;
        int wy = iy % cellsY; if (wy < 0) wy += cellsY;

        const double x0 = ix * Dx, x1 = (ix + 1) * Dx;
        const double y0 = iy * Dy, y1 = (iy + 1) * Dy;
        const double P00[3] = {x0, y0, Z[wy  ][wx  ]};
        const double P10[3] = {x1, y0, Z[wy  ][wx+1]};
        const double P01[3] = {x0, y1, Z[wy+1][wx  ]};
        const double P11[3] = {x1, y1, Z[wy+1][wx+1]};

        double t;
        double bestT = 1e300;
        int    bestTriangle = -1;
        if (intersectTriangle(origin, dir, P00, P10, P01, tMin, t) && t < bestT) { bestT = t; bestTriangle = 0; }
        if (intersectTriangle(origin, dir, P10, P11, P01, tMin, t) && t < bestT) { bestT = t; bestTriangle = 1; }

        if (bestTriangle != -1)
        {
            tHit = bestT;
            double e1[3], e2[3];
            if (bestTriangle == 0)
                for (int i = 0; i < 3; i++) { e1[i] = P10[i] - P00[i]; e2[i] = P01[i] - P00[i]; }
            else
                for (int i = 0; i < 3; i++) { e1[i] = P11[i] - P10[i]; e2[i] = P01[i] - P10[i]; }
            hitNormal[0] = e1[1]*e2[2] - e1[2]*e2[1];
            hitNormal[1] = e1[2]*e2[0] - e1[0]*e2[2];
            hitNormal[2] = e1[0]*e2[1] - e1[1]*e2[0];
            const double norm = sqrt(hitNormal[0]*hitNormal[0] + hitNormal[1]*hitNormal[1] + hitNormal[2]*hitNormal[2]);
            for (int i = 0; i < 3; i++) hitNormal[i] /= norm;
            return true;
        }

        // advance to the next cell
        double tEntry;
        if (tMaxX < tMaxY)
        {
            tEntry = tMaxX;
            ix += stepX;
            tMaxX += tDeltaX;
        }
        else
        {
            tEntry = tMaxY;
            iy += stepY;
            tMaxY += tDeltaY;
        }

        // once above the highest (moving up) or below the lowest (moving down) point of the
        // surface, no intersection is possible anymore
        const double zEntry = origin[2] + dir[2] * tEntry;
        if (dir[2] > 0 && zEntry > Zmax) return false;
        if (dir[2] < 0 && zEntry < Zmin) return false;

        const int newPeriodX = (ix >= 0 ? ix / cellsX : (ix + 1) / cellsX - 1);
        const int newPeriodY = (iy >= 0 ? iy / cellsY : (iy + 1) / cellsY - 1);
        if (newPeriodX != periodX) { periodX = newPeriodX; Wraps++; }
        if (newPeriodY != periodY) { periodY = newPeriodY; Wraps++; }
    }
    return false;   // traversal cap exceeded (near-horizontal ray) -> caller classifies by direction
}

int ALutSurfaceGenerator::tracePhoton(double * pos, double * dir, int & bounces)
{
    const double Lx = (Nx - 1) * Dx;
    const double Ly = (Ny - 1) * Dy;
    const double epsOffset = 1e-6 * std::min(Dx, Dy);

    bool inMedium1 = true;

    while (true)
    {
        double tHit;
        double normal[3];
        if (!findIntersection(pos, dir, tHit, normal))
        {
            // Classify an escaping photon by the medium it is in, following Roncali & Cherry:
            // a photon whose last interface event was a reflection is still in the incident
            // medium (crystal) and counts as reflected/back; one that has transmitted through
            // the interface is in the outer medium and counts as transmitted/forward. The
            // reflect/refract history (not the momentary flight direction) defines the fate,
            // so no photon is ever discarded. The counters record the minority of photons whose
            // flight direction disagrees with their medium (steep-facet multi-bounce escapees).
            if (dir[2] > 0 &&  inMedium1) UpEscapeReclass++;    // reflected photon flying upward
            if (dir[2] < 0 && !inMedium1) DownEscapeReclass++;  // transmitted photon flying downward
            return inMedium1 ? 1 : -1;
        }

        for (int i = 0; i < 3; i++) pos[i] += dir[i] * tHit;

        // flip the facet normal to face the incoming photon
        double cosI = -(dir[0]*normal[0] + dir[1]*normal[1] + dir[2]*normal[2]);
        if (cosI < 0)
        {
            for (int i = 0; i < 3; i++) normal[i] = -normal[i];
            cosI = -cosI;
        }
        if (cosI > 1.0) cosI = 1.0;

        const double nFrom = (inMedium1 ? n1 : n2);
        const double nTo   = (inMedium1 ? n2 : n1);

        if (RandomHub.uniform() < fresnelReflection(cosI, nFrom, nTo))
        {
            // specular reflection about the local facet normal
            for (int i = 0; i < 3; i++) dir[i] += 2.0 * cosI * normal[i];
        }
        else
        {
            // Snell refraction about the local facet normal
            const double eta = nFrom / nTo;
            const double cosT = sqrt(1.0 - eta*eta * (1.0 - cosI*cosI));   // no TIR here: Fresnel returned 1 in that case
            for (int i = 0; i < 3; i++) dir[i] = eta * dir[i] + (eta * cosI - cosT) * normal[i];
            inMedium1 = !inMedium1;
        }
        const double norm = sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
        for (int i = 0; i < 3; i++) dir[i] /= norm;

        bounces++;
        if (bounces > maxBounces) { DegenerateDiscarded++; return 0; }  // trapped in a deep valley

        // move away from the facet and re-wrap the position into the base tile
        for (int i = 0; i < 3; i++) pos[i] += dir[i] * epsOffset;
        const double shiftX = std::floor(pos[0] / Lx);
        if (shiftX != 0) { pos[0] -= shiftX * Lx; Wraps++; }
        const double shiftY = std::floor(pos[1] / Ly);
        if (shiftY != 0) { pos[1] -= shiftY * Ly; Wraps++; }
    }
}

namespace
{
    // temporarily turns the heightmap upside down (z -> -z) to trace photons arriving
    // from the other side of the same physical surface; restored on destruction
    struct AHeightmapFlipper
    {
        AHeightmapFlipper(std::vector<std::vector<double>> & z, double & zmin, double & zmax, bool active) :
            Zref(z), ZminRef(zmin), ZmaxRef(zmax), Active(active) {if (Active) flip();}
        ~AHeightmapFlipper() {if (Active) flip();}

        void flip()
        {
            for (std::vector<double> & row : Zref)
                for (double & z : row) z = -z;
            const double newMin = -ZmaxRef;
            ZmaxRef = -ZminRef;
            ZminRef = newMin;
        }

        std::vector<std::vector<double>> & Zref;
        double & ZminRef;
        double & ZmaxRef;
        bool Active;
    };
}

QString ALutSurfaceGenerator::generate(ALutSurfaceData & result)
{
    QString err = validateConfig();
    if (!err.isEmpty()) return err;

    if (seed != 0) RandomHub.setSeed(seed);

    AHeightmapFlipper flipper(Z, Zmin, Zmax, reverseGeometry);

    result.clear();
    result.n1 = n1;
    result.n2 = n2;
    result.Wavelength = wavelength;
    result.SourceHeightmap = QFileInfo(HeightmapFileName).fileName();
    result.PixelSizeX = Dx;
    result.PixelSizeY = Dy;
    result.GridSizeX = Nx;
    result.GridSizeY = Ny;
    result.PhotonsPerThetaBin = photonsPerThetaBin;
    result.Seed = seed;
    result.MaxBounces = maxBounces;
    result.GenerationDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    result.Comment = comment;

    result.ThetaIncBins = thetaIncBins;
    result.ThetaOutBins = thetaOutBins;
    result.PhiOutBins   = phiOutBins;
    result.Launched.assign         (thetaIncBins, 0);
    result.ReflectedCounts.assign  (thetaIncBins, 0);
    result.TransmittedCounts.assign(thetaIncBins, 0);
    result.AbsorbedCounts.assign   (thetaIncBins, 0);
    result.ReflectedHist.assign    (thetaIncBins, std::vector<int>(thetaOutBins * phiOutBins, 0));
    result.TransmittedHist.assign  (thetaIncBins, std::vector<int>(thetaOutBins * phiOutBins, 0));

    const double Lx = (Nx - 1) * Dx;
    const double Ly = (Ny - 1) * Dy;
    const double launchZ = Zmin - std::max(Dx, Dy);   // strictly below the surface everywhere

    Anomalies = 0;
    Wraps = 0;
    UpEscapeReclass = DownEscapeReclass = DegenerateDiscarded = 0;
    long long bounceSum = 0;
    long long tallied = 0;

    for (int iTheta = 0; iTheta < thetaIncBins; iTheta++)
    {
        const double thetaInc = (iTheta + 0.5) * (90.0 / thetaIncBins) * M_PI / 180.0;
        const double sinTi = sin(thetaInc);
        const double cosTi = cos(thetaInc);

        for (int iPhoton = 0; iPhoton < photonsPerThetaBin; iPhoton++)
        {
            const double phi0 = (phiSteps > 0 ? (iPhoton % phiSteps) * 2.0*M_PI / phiSteps
                                              : 2.0*M_PI * RandomHub.uniform());

            double pos[3] = {Lx * RandomHub.uniform(), Ly * RandomHub.uniform(), launchZ};
            double dir[3] = {sinTi * cos(phi0), sinTi * sin(phi0), cosTi};

            int bounces = 0;
            const int outcome = tracePhoton(pos, dir, bounces);
            if (outcome == 0)
            {
                Anomalies++;   // excluded from the LUT to keep count conservation exact
                continue;
            }

            result.Launched[iTheta]++;
            bounceSum += bounces;
            tallied++;

            double phiRel = atan2(dir[1], dir[0]) - phi0;
            if (reverseGeometry) phiRel = -phiRel;   // mirrored geometry flips the frame handedness
            phiRel -= 2.0*M_PI * std::floor(phiRel / (2.0*M_PI));   // wrap to [0, 2pi)
            int iPhi = (int)(phiRel / (2.0*M_PI) * phiOutBins);
            if (iPhi >= phiOutBins) iPhi = phiOutBins - 1;

            double cosOut = (outcome == 1 ? -dir[2] : dir[2]);
            if (cosOut > 1.0) cosOut = 1.0;
            else if (cosOut < 0) cosOut = 0;
            int iThetaOut = (int)(acos(cosOut) / (M_PI / 2.0) * thetaOutBins);
            if (iThetaOut >= thetaOutBins) iThetaOut = thetaOutBins - 1;

            if (outcome == 1)
            {
                result.ReflectedCounts[iTheta]++;
                result.ReflectedHist[iTheta][iThetaOut * phiOutBins + iPhi]++;
            }
            else
            {
                result.TransmittedCounts[iTheta]++;
                result.TransmittedHist[iTheta][iThetaOut * phiOutBins + iPhi]++;
            }
        }

        if (progressCallback)
            if (!progressCallback((iTheta + 1) * 100 / thetaIncBins)) return "Generation aborted";
    }

    MeanBounces = (tallied > 0 ? bounceSum / (double)tallied : 0);
    result.MeanBounces  = MeanBounces;
    result.AnomalyCount = Anomalies;

    // buildRuntime() includes the full consistency check of the generated tables
    return result.buildRuntime();
}
