#include "afloodruntime.h"
#include "aphotonsimsettings.h"
#include "arandomhub.h"
#include "ageoconsts.h"

#include "TFormula.h"
#include "TMath.h"

#include <cmath>
#include <memory>
#include <algorithm>

// see the flood spec, section 4

namespace
{
    constexpr double FloodTwoPi           = 2.0 * M_PI;
    constexpr int    FloodAutoStartBins   = 64;
    constexpr double FloodAutoTolerance   = 1e-4;
    constexpr int    FloodAutoMaxBins     = 100000;
    constexpr int    FloodAutoMaxDepth    = 20;
    constexpr int    FloodTableTargetBins = 1000;
    constexpr double FloodGaussMinAccept  = 0.05;

    double floodPhi(double z) {return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));}   // standard normal CDF

    // evaluates a user function and keeps the first bad value
    struct AFloodEval
    {
        const std::function<double(double)> & Fun;
        bool   Bad  = false;
        double BadX = 0;
        double BadV = 0;

        AFloodEval(const std::function<double(double)> & fun) : Fun(fun) {}

        double operator()(double x)
        {
            const double v = Fun(x);
            if (!std::isfinite(v) || v < 0)
            {
                if (!Bad) {Bad = true; BadX = x; BadV = v;}
                return 0;
            }
            return v;
        }

        QString error(const QString & name) const
        {
            return QString("%1: profile value is %2 at x = %3").arg(name, (std::isfinite(BadV) ? "negative" : "not finite"), QString::number(BadX));
        }
    };

    QString compileFormula(const QString & formula, const QString & name, std::shared_ptr<TFormula> & f)
    {
        if (formula.simplified().isEmpty()) return name + ": formula is empty";

        const AGeoConsts & GC = AGeoConsts::getConstInstance();
        if (GC.getIndexByName("x") != -1) return name + ": a geometry constant named \"x\" conflicts with the formula variable";

        const QString str = GC.substituteConstsWithParameters(formula);
        f = std::make_shared<TFormula>("", str.toLocal8Bit().data());
        if (!f->IsValid()) return name + ": invalid formula: " + formula;
        if (f->GetNdim() > 1) return name + ": formula can use only the variable x: " + formula;
        return "";
    }

    double formulaValue(const std::shared_ptr<TFormula> & f, double x)
    {
        double xx[1] = {x};
        return f->EvalPar(xx, AGeoConsts::getConstInstance().getParameterValues());
    }

    void addWarning(QString * warnings, const QString & txt)
    {
        if (!warnings) return;
        if (!warnings->isEmpty()) *warnings += '\n';
        *warnings += txt;
    }

    double jacobian(double r, int dim) // 1 for axes, r (2D) or r^2 (3D) for radial density
    {
        if (dim == 2) return r;
        if (dim == 3) return r * r;
        return 1.0;
    }

    double integralJacobian(double a, double b, int dim)
    {
        if (dim == 2) return 0.5 * (b*b - a*a);
        if (dim == 3) return (b*b*b - a*a*a) / 3.0;
        return b - a;
    }
}

// ---------------- AFloodCoordSampler ----------------

double AFloodCoordSampler::sample() const
{
    ARandomHub & RH = ARandomHub::getInstance();

    switch (Mode)
    {
    case Fixed :
        return Lo;
    case Uniform :
        return Lo + (Hi - Lo) * RH.uniform();
    case ShellUniform :
    {
        const double u   = RH.uniform();
        const double loD = std::pow(Lo, Dim);
        const double hiD = std::pow(Hi, Dim);
        return std::pow(loD + u * (hiD - loD), 1.0 / Dim);
    }
    case Exponential :
    {
        const double L = Hi - Lo;
        const double d = -Lambda * std::log1p(RH.uniform() * std::expm1(-L / Lambda));
        const double x = Entry + Sign * d;
        return std::min(Hi, std::max(Lo, x));
    }
    case Gauss :
    {
        const double p = PLo + RH.uniform() * (PHi - PLo);
        double arg = 2.0 * p - 1.0;
        arg = std::min(1.0 - 1e-16, std::max(-1.0 + 1e-16, arg));
        const double x = Mu + SigmaG * std::sqrt(2.0) * TMath::ErfInverse(arg);
        return std::min(Hi, std::max(Lo, x));
    }
    case Grid :
        return Lo + Sampler.getRandom();
    case GaussRadialRejection :
        return 0; // sampled as a vector by AFloodRuntime
    }
    return 0;
}

double AFloodCoordSampler::expectedPdf(double x) const
{
    if (x < Lo || x > Hi) return 0;

    switch (Mode)
    {
    case Fixed :
        return 0;
    case Uniform :
        return (Hi > Lo ? 1.0 / (Hi - Lo) : 0);
    case ShellUniform :
        return Dim * std::pow(x, Dim - 1) / (std::pow(Hi, Dim) - std::pow(Lo, Dim));
    case Exponential :
    {
        const double L = Hi - Lo;
        const double d = Sign * (x - Entry);
        return std::exp(-d / Lambda) / (-Lambda * std::expm1(-L / Lambda));
    }
    case Gauss :
    {
        const double z = (x - Mu) / SigmaG;
        return std::exp(-0.5 * z * z) / (std::sqrt(FloodTwoPi) * SigmaG * (PHi - PLo));
    }
    case Grid :
    {
        if (GridEdges.size() < 2 || GridTotal <= 0) return 0;
        auto it = std::upper_bound(GridEdges.begin(), GridEdges.end(), x);
        size_t i = (it == GridEdges.begin() ? 0 : (it - GridEdges.begin()) - 1);
        if (i >= GridWeights.size()) i = GridWeights.size() - 1;
        const double width = GridEdges[i+1] - GridEdges[i];
        return (width > 0 ? GridWeights[i] / (width * GridTotal) : 0);
    }
    case GaussRadialRejection :
    {
        // r^(d-1) exp(-r^2/2s^2) normalized on [Lo, Hi]; PLo/PHi hold the regularized lower gamma values
        const double a    = 0.5 * Dim;
        const double norm = std::pow(SigmaG, Dim) * std::pow(2.0, a - 1.0) * TMath::Gamma(a) * (PHi - PLo);
        return std::pow(x, Dim - 1) * std::exp(-0.5 * x * x / (SigmaG * SigmaG)) / norm;
    }
    }
    return 0;
}

// ---------------- AFloodRuntime ----------------

QString AFloodRuntime::checkFormula(const QString & formula, double x, double & value)
{
    std::shared_ptr<TFormula> f;
    const QString err = compileFormula(formula, "Formula", f);
    if (!err.isEmpty()) return err;
    value = formulaValue(f, x);
    return "";
}

void AFloodRuntime::clear()
{
    *this = AFloodRuntime();
}

QString AFloodRuntime::buildGrid(AFloodCoordSampler & s, const std::vector<double> & edges, const std::vector<double> & weights, const QString & name)
{
    // drop zero-width bins (the sampler needs strictly increasing bounds)
    std::vector<double> e;
    std::vector<double> w;
    e.reserve(edges.size());
    w.reserve(weights.size());
    for (size_t i = 0; i < weights.size(); i++)
    {
        if (edges[i+1] <= edges[i]) continue;
        if (e.empty()) e.push_back(edges[i]);
        e.push_back(edges[i+1]);
        w.push_back(weights[i]);
    }

    double total = 0;
    for (double v : w) total += v;
    if (w.empty() || !(total > 0)) return name + ": total weight of the distribution is zero";

    std::vector<std::pair<double,double>> data;
    data.reserve(e.size());
    for (size_t i = 0; i < w.size(); i++) data.push_back({e[i] - e.front(), w[i]});
    data.push_back({e.back() - e.front(), 0});

    const std::string err = s.Sampler.configure(data, true);
    if (!err.empty()) return name + ": " + QString(err.data());

    s.Mode        = AFloodCoordSampler::Grid;
    s.Lo          = e.front();
    s.Hi          = e.back();
    s.GridEdges   = e;
    s.GridWeights = w;
    s.GridTotal   = total;
    return "";
}

QString AFloodRuntime::buildGridFromFunction(AFloodCoordSampler & s, double lo, double hi, int fixedBins,
                                             const std::function<double(double)> & fun, const QString & name, QString * warnings)
{
    if (!(hi > lo)) return name + ": empty range";

    AFloodEval w(fun);
    std::vector<double> edges;
    std::vector<double> weights;

    if (fixedBins > 0)
    {
        const double h = (hi - lo) / fixedBins;
        for (int i = 0; i <= fixedBins; i++) edges.push_back(lo + i * h);
        for (int i = 0; i < fixedBins; i++)
        {
            const double a = edges[i], b = edges[i+1];
            weights.push_back( (b - a) / 6.0 * (w(a) + 4.0 * w(0.5*(a+b)) + w(b)) );
        }
        if (w.Bad) return w.error(name);
    }
    else
    {
        struct Interval {double a, b, wa, wb; int depth;};
        const double h0 = (hi - lo) / FloodAutoStartBins;

        std::vector<double> nodeVals(FloodAutoStartBins + 1);
        for (int i = 0; i <= FloodAutoStartBins; i++) nodeVals[i] = w(lo + i * h0);

        double total = 0;
        for (int i = 0; i < FloodAutoStartBins; i++)
        {
            const double a = lo + i * h0, b = a + h0;
            total += h0 / 6.0 * (nodeVals[i] + 4.0 * w(0.5*(a+b)) + nodeVals[i+1]);
        }
        if (w.Bad) return w.error(name);
        if (!(total > 0)) return name + ": total weight of the distribution is zero";

        const double tol = FloodAutoTolerance * total;
        bool capReached = false;
        edges.push_back(lo);

        // depth-first refinement keeps the intervals ordered
        for (int i = 0; i < FloodAutoStartBins; i++)
        {
            std::vector<Interval> stack;
            stack.push_back({lo + i * h0, lo + (i+1) * h0, nodeVals[i], nodeVals[i+1], 0});
            while (!stack.empty())
            {
                Interval iv = stack.back();
                stack.pop_back();
                const double h  = iv.b - iv.a;
                const double m  = 0.5 * (iv.a + iv.b);
                const double wm = w(m);
                const double T  = 0.5 * h * (iv.wa + iv.wb);
                const double M  = h * wm;

                const bool canSplit = (iv.depth < FloodAutoMaxDepth) && ((int)weights.size() + (int)stack.size() + 2 < FloodAutoMaxBins);
                if (std::fabs(T - M) > tol && iv.depth < FloodAutoMaxDepth && !canSplit) capReached = true;

                if (std::fabs(T - M) > tol && canSplit)
                {
                    stack.push_back({m, iv.b, wm, iv.wb, iv.depth + 1});   // right half processed after the left one
                    stack.push_back({iv.a, m, iv.wa, wm, iv.depth + 1});
                }
                else
                {
                    weights.push_back( (T + 2.0 * M) / 3.0 );   // Simpson
                    edges.push_back(iv.b);
                }
            }
        }
        if (w.Bad) return w.error(name);
        if (capReached) addWarning(warnings, name + QString(": grid refinement stopped at %1 bins; the distribution may be approximated coarsely").arg(FloodAutoMaxBins));
    }

    return buildGrid(s, edges, weights, name);
}

QString AFloodRuntime::buildGridFromTable(AFloodCoordSampler & s, const AFloodProfile & p, double lo, double hi, int jacobianDim, const QString & name, QString * warnings)
{
    const std::vector<std::pair<double,double>> & d = p.Data;
    const size_t n = d.size();
    if (n < 2) return name + ": table needs at least 2 points";
    for (size_t i = 0; i < n; i++)
    {
        if (i > 0 && d[i].first <= d[i-1].first) return name + ": table x values must be strictly increasing";
        if (d[i].second < 0 || !std::isfinite(d[i].second)) return name + ": table values must be finite and non-negative";
    }

    const double dLo = std::max(lo, d.front().first);
    const double dHi = std::min(hi, d.back().first);
    if (!(dHi > dLo)) return name + ": table does not overlap the range of the shape";
    if (d.front().first > lo || d.back().first < hi)
        addWarning(warnings, name + ": table does not cover the whole range of the shape; the density is zero outside of the table");

    std::vector<double> edges;
    std::vector<double> weights;
    edges.push_back(dLo);

    const int k = std::max(1, (int)std::ceil((double)FloodTableTargetBins / (n - 1)));
    const int nBins = (int)n - 1;

    for (int i = 0; i < nBins; i++)
    {
        const double x0 = d[i].first, x1 = d[i+1].first;
        if (x1 <= dLo || x0 >= dHi) continue;

        if (p.TableMode == AFloodProfile::Bins && jacobianDim <= 1)
        {
            // the user's bin directly; a clipped bin keeps its proportional part
            const double a = std::max(x0, dLo), b = std::min(x1, dHi);
            weights.push_back(d[i].second * (b - a) / (x1 - x0));
            edges.push_back(b);
            continue;
        }

        const double h = (x1 - x0) / k;
        for (int j = 0; j < k; j++)
        {
            double a = x0 + j * h, b = (j == k-1 ? x1 : x0 + (j+1) * h);
            if (b <= dLo || a >= dHi) continue;
            a = std::max(a, dLo);
            b = std::min(b, dHi);

            double weight;
            if (p.TableMode == AFloodProfile::Bins)
            {
                // value is the weight of the user's bin, distributed inside it as the Jacobian
                weight = d[i].second * integralJacobian(a, b, jacobianDim) / integralJacobian(x0, x1, jacobianDim);
            }
            else
            {
                // linear interpolation times the Jacobian, Simpson (exact up to cubic)
                auto f = [&](double x) {return (d[i].second + (d[i+1].second - d[i].second) * (x - x0) / (x1 - x0)) * jacobian(x, jacobianDim);};
                weight = (b - a) / 6.0 * (f(a) + 4.0 * f(0.5*(a+b)) + f(b));
            }
            weights.push_back(weight);
            edges.push_back(b);
        }
    }

    return buildGrid(s, edges, weights, name);
}

QString AFloodRuntime::configureAxis(AFloodCoordSampler & s, const AFloodProfile & p, double size, const QString & name, QString * warnings)
{
    if (size < 0) return name + ": size cannot be negative";
    const double lo = -0.5 * size;
    const double hi =  0.5 * size;
    s.Lo = lo;
    s.Hi = hi;

    if (size == 0)
    {
        if (p.Type != AFloodProfile::Uniform) return name + ": a non-uniform profile needs a non-zero size";
        s.Mode = AFloodCoordSampler::Fixed;
        return "";
    }

    switch (p.Type)
    {
    case AFloodProfile::Uniform :
        s.Mode = AFloodCoordSampler::Uniform;
        return "";

    case AFloodProfile::Exponential :
        if (!(p.AttenuationLength > 0)) return name + ": AttenuationLength must be positive";
        s.Mode   = AFloodCoordSampler::Exponential;
        s.Lambda = p.AttenuationLength;
        s.Entry  = (p.Direction == AFloodProfile::Plus ? lo : hi);
        s.Sign   = (p.Direction == AFloodProfile::Plus ? 1.0 : -1.0);
        return "";

    case AFloodProfile::Gauss :
    {
        if (!(p.Sigma > 0)) return name + ": Sigma must be positive";
        s.Mu     = p.Mean;
        s.SigmaG = p.Sigma;
        s.PLo    = floodPhi((lo - p.Mean) / p.Sigma);
        s.PHi    = floodPhi((hi - p.Mean) / p.Sigma);
        if (s.PHi - s.PLo >= 1e-6)
        {
            s.Mode = AFloodCoordSampler::Gauss;
            return "";
        }
        const double mu = p.Mean, sig = p.Sigma;
        addWarning(warnings, name + ": the range is far in the tail of the Gaussian; using a tabulated distribution");
        return buildGridFromFunction(s, lo, hi, 0, [mu, sig](double x){const double z = (x - mu) / sig; return std::exp(-0.5 * z * z);}, name, warnings);
    }

    case AFloodProfile::Formula :
    {
        std::shared_ptr<TFormula> f;
        const QString err = compileFormula(p.FormulaText, name, f);
        if (!err.isEmpty()) return err;
        return buildGridFromFunction(s, lo, hi, p.SamplingBins, [f](double x){return formulaValue(f, x);}, name, warnings);
    }

    case AFloodProfile::Table :
        return buildGridFromTable(s, p, lo, hi, 1, name, warnings);
    }
    return name + ": unknown profile type";
}

QString AFloodRuntime::configureRadial(AFloodCoordSampler & s, const AFloodProfile & p, double rIn, double rOut, int dim, const QString & name, QString * warnings)
{
    s.Lo  = rIn;
    s.Hi  = rOut;
    s.Dim = dim;
    const bool distribution = (p.RadialMeaning == AFloodProfile::Distribution);
    const int  jDim = (distribution ? 1 : dim);

    switch (p.Type)
    {
    case AFloodProfile::Uniform :
        s.Mode = (distribution ? AFloodCoordSampler::Uniform : AFloodCoordSampler::ShellUniform);
        return "";

    case AFloodProfile::Exponential :
    {
        if (!(p.AttenuationLength > 0)) return name + ": AttenuationLength must be positive";
        if (distribution)
        {
            s.Mode   = AFloodCoordSampler::Exponential;
            s.Lambda = p.AttenuationLength;
            s.Entry  = rIn;
            s.Sign   = 1.0;
            return "";
        }
        const double lam = p.AttenuationLength;
        return buildGridFromFunction(s, rIn, rOut, 0, [lam, dim](double r){return std::exp(-r / lam) * jacobian(r, dim);}, name, warnings);
    }

    case AFloodProfile::Gauss :
    {
        if (distribution) return name + ": RadialMeaning \"distribution\" cannot be used with a Gaussian profile; use the density meaning or a formula";
        if (!(p.Sigma > 0)) return name + ": Sigma must be positive";
        const double sig = p.Sigma;
        const double a   = 0.5 * dim;
        s.SigmaG = sig;
        s.PLo    = TMath::Gamma(a, 0.5 * rIn  * rIn  / (sig * sig));
        s.PHi    = TMath::Gamma(a, 0.5 * rOut * rOut / (sig * sig));
        const double accept = s.PHi - s.PLo;
        if (accept >= FloodGaussMinAccept)
        {
            s.Mode     = AFloodCoordSampler::GaussRadialRejection;
            s.MaxTries = (int)std::ceil(50.0 / accept);
            return "";
        }
        addWarning(warnings, name + ": most of the Gaussian is outside of the shape; using a tabulated distribution");
        return buildGridFromFunction(s, rIn, rOut, 0, [sig, dim](double r){return std::exp(-0.5 * r * r / (sig * sig)) * jacobian(r, dim);}, name, warnings);
    }

    case AFloodProfile::Formula :
    {
        std::shared_ptr<TFormula> f;
        const QString err = compileFormula(p.FormulaText, name, f);
        if (!err.isEmpty()) return err;
        return buildGridFromFunction(s, rIn, rOut, p.SamplingBins, [f, jDim](double r){return formulaValue(f, r) * jacobian(r, jDim);}, name, warnings);
    }

    case AFloodProfile::Table :
        return buildGridFromTable(s, p, rIn, rOut, jDim, name, warnings);
    }
    return name + ": unknown profile type";
}

QString AFloodRuntime::configure(const AFloodSettings & set, QString * warnings)
{
    clear();

    if (set.Number < 1) return "Number of bombs must be at least 1";

    Shape = set.Shape;
    Axis  = set.Axis;
    Center[0] = set.X0;
    Center[1] = set.Y0;
    Center[2] = set.Z0;

    QString err;
    switch (set.Shape)
    {
    case AFloodSettings::Box :
        err = configureAxis(C1, set.ProfileX, set.SizeX, "ProfileX", warnings);  if (!err.isEmpty()) return err;
        err = configureAxis(C2, set.ProfileY, set.SizeY, "ProfileY", warnings);  if (!err.isEmpty()) return err;
        err = configureAxis(C3, set.ProfileZ, set.SizeZ, "ProfileZ", warnings);  if (!err.isEmpty()) return err;
        break;

    case AFloodSettings::Cylinder :
    case AFloodSettings::Sphere :
    {
        if (!(set.OuterDiameter > 0)) return "OuterDiameter must be positive";
        if (set.InnerDiameter < 0 || set.InnerDiameter >= set.OuterDiameter) return "InnerDiameter must be non-negative and smaller than OuterDiameter";
        const int dim = (set.Shape == AFloodSettings::Cylinder ? 2 : 3);
        err = configureRadial(C1, set.ProfileRadial, 0.5 * set.InnerDiameter, 0.5 * set.OuterDiameter, dim, "ProfileRadial", warnings);
        if (!err.isEmpty()) return err;
        if (set.Shape == AFloodSettings::Cylinder)
        {
            if (set.Length < 0) return "Length cannot be negative";
            err = configureAxis(C2, set.ProfileAxial, set.Length, "ProfileAxial", warnings);
            if (!err.isEmpty()) return err;
        }
        break;
    }
    }

    Ready = true;
    return "";
}

bool AFloodRuntime::sampleGaussVector(const AFloodCoordSampler & s, double * v) const
{
    ARandomHub & RH = ARandomHub::getInstance();
    const double lo2 = s.Lo * s.Lo;
    const double hi2 = s.Hi * s.Hi;
    for (int iTry = 0; iTry < s.MaxTries; iTry++)
    {
        double r2 = 0;
        for (int i = 0; i < s.Dim; i++)
        {
            v[i] = RH.gauss(0, s.SigmaG);
            r2 += v[i] * v[i];
        }
        if (r2 >= lo2 && r2 <= hi2) return true;
    }
    return false;
}

bool AFloodRuntime::generatePosition(double * r) const
{
    if (!Ready) return false;
    ARandomHub & RH = ARandomHub::getInstance();

    double local[3] = {0, 0, 0};

    switch (Shape)
    {
    case AFloodSettings::Box :
        local[0] = C1.sample();
        local[1] = C2.sample();
        local[2] = C3.sample();
        break;

    case AFloodSettings::Cylinder :
    {
        double t[2];
        if (C1.Mode == AFloodCoordSampler::GaussRadialRejection)
        {
            if (!sampleGaussVector(C1, t)) return false;
        }
        else
        {
            const double rho = C1.sample();
            const double phi = FloodTwoPi * RH.uniform();
            t[0] = rho * std::cos(phi);
            t[1] = rho * std::sin(phi);
        }
        const double a = C2.sample();
        switch (Axis)
        {
        case AFloodSettings::AxisX : local[0] = a;    local[1] = t[0]; local[2] = t[1]; break;
        case AFloodSettings::AxisY : local[0] = t[1]; local[1] = a;    local[2] = t[0]; break;
        default                    : local[0] = t[0]; local[1] = t[1]; local[2] = a;    break;
        }
        break;
    }

    case AFloodSettings::Sphere :
        if (C1.Mode == AFloodCoordSampler::GaussRadialRejection)
        {
            if (!sampleGaussVector(C1, local)) return false;
        }
        else
        {
            const double rho      = C1.sample();
            const double cosTheta = 2.0 * RH.uniform() - 1.0;
            const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
            const double phi      = FloodTwoPi * RH.uniform();
            local[0] = rho * sinTheta * std::cos(phi);
            local[1] = rho * sinTheta * std::sin(phi);
            local[2] = rho * cosTheta;
        }
        break;
    }

    for (int i = 0; i < 3; i++) r[i] = Center[i] + local[i];
    return true;
}
