#ifndef AFLOODRUNTIME_H
#define AFLOODRUNTIME_H

#include "ahistogram.h"

#include <QString>

#include <vector>
#include <utility>
#include <functional>

class AFloodSettings;
struct AFloodProfile;

// Samples one coordinate of a flood region in local coordinates (offset from the flood Position)
class AFloodCoordSampler
{
public:
    enum EMode {Fixed, Uniform, Exponential, Gauss, Grid,      // axis-like: box axes, cylinder axial, radial "distribution"
                ShellUniform, GaussRadialRejection};           // radial density shortcuts

    EMode  Mode = Fixed;
    double Lo   = 0;      // range, Lo <= Hi
    double Hi   = 0;

    // mode parameters
    double Lambda  = 1.0;     // Exponential
    double Entry   = 0;       // Exponential: entry face coordinate
    double Sign    = 1.0;     // Exponential: +1 decay toward Hi, -1 toward Lo
    double Mu      = 0;       // Gauss
    double SigmaG  = 1.0;     // Gauss / GaussRadialRejection
    double PLo     = 0;       // Gauss: CDF at Lo
    double PHi     = 1.0;     // Gauss: CDF at Hi
    int    Dim     = 1;       // ShellUniform / GaussRadialRejection / Grid density: 2 (cylinder) or 3 (sphere)
    int    MaxTries = 1000;   // GaussRadialRejection retry cap

    ARandomSampler Sampler;   // Grid: bins relative to Lo
    std::vector<double> GridEdges;    // Grid: absolute edges (for expectedPdf)
    std::vector<double> GridWeights;  // Grid: bin weights
    double GridTotal = 0;

    double sample() const;                 // not for GaussRadialRejection (sampled as a vector by the runtime)
    double expectedPdf(double x) const;    // normalized distribution of the coordinate (incl. r / r^2 for radial density)
};

class AFloodRuntime
{
public:
    bool   Ready = false;
    int    Shape = 0;           // AFloodSettings::EShape
    int    Axis  = 2;           // AFloodSettings::EAxis
    double Center[3] = {0, 0, 0};

    AFloodCoordSampler C1, C2, C3;   // box: X,Y,Z; cylinder: R, axial; sphere: R

    void    clear();
    QString configure(const AFloodSettings & settings, QString * warnings);  // returns error, "" if ok
    bool    generatePosition(double * r) const;                              // global coordinates

    // radial gauss rejection helper: fills local transverse/3D vector; returns false if cap is reached
    bool    sampleGaussVector(const AFloodCoordSampler & s, double * v) const;

    // compiles the formula (geometry constants allowed) and evaluates it at x; returns error, "" if ok
    static QString checkFormula(const QString & formula, double x, double & value);

    // helpers, public for testing
    static QString configureAxis(AFloodCoordSampler & s, const AFloodProfile & p, double size, const QString & name, QString * warnings);
    static QString configureRadial(AFloodCoordSampler & s, const AFloodProfile & p, double rIn, double rOut, int dim, const QString & name, QString * warnings);
    static QString buildGrid(AFloodCoordSampler & s, const std::vector<double> & edges, const std::vector<double> & weights, const QString & name);
    static QString buildGridFromFunction(AFloodCoordSampler & s, double lo, double hi, int fixedBins,
                                         const std::function<double(double)> & w, const QString & name, QString * warnings);
    static QString buildGridFromTable(AFloodCoordSampler & s, const AFloodProfile & p, double lo, double hi, int jacobianDim, const QString & name, QString * warnings);
};

#endif // AFLOODRUNTIME_H
