#include "lrformula1.h"
#include "Eigen/src/Core/Matrix.h"
#include "json11.hpp"
#include "profileHist.h"

//#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "functor.h"

// parameter naming scheme
// a - amplitude
// s - width
// c - additive constant
// mean is always 0

// Universal functor
struct UniFunctor : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Func type;

    UniFunctor(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Func type_)
        : Functor<double>(3, x_.size()), x(x_), y(y_), type(type_) {}

    // Compute residuals: f(p) = model(p) - y
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        double a = p[0];
        double s = p[1];
        double c = p[2];

        Eigen::ArrayXd dx(x), t;

        switch (type) {
            case Gauss:
                fvec = a*exp(-dx*dx/2/s/s) + c - y.array();
                break;
            case Sech:
                fvec = a/cosh(dx/s) + c - y.array();
                break;
            case Sech2:
                t = 1/cosh(dx/s);
                fvec = a*t*t + c - y.array();
                break;
            case Cauchy:
                fvec = a/(1 + dx*dx/s/s) + c - y.array();

        }

        return 0;
    }
};

// Universal functor with weights
struct UniFunctorW : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Eigen::VectorXd w;
    Func type;

    UniFunctorW(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Eigen::VectorXd& w_, const Func type_)
        : Functor<double>(3, x_.size()), x(x_), y(y_), w(w_), type(type_) {}

    // Compute residuals: f(p) = model(p) - y
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        double a = p[0];
        double s = p[1];
        double c = p[2];

        Eigen::ArrayXd dx(x), t, f;
        dx /= s;

        switch (type) {
            case Gauss:
                f = exp(-dx*dx/2);
                break;
            case Sech:
                f = 1/cosh(dx);
                break;
            case Sech2:
                t = 1/cosh(dx);
                f = t*t;
                break;
            case Cauchy:
                f = 1/(1 + dx*dx);
        }
        fvec = (f*a + c - y.array()) * sqrt(w.array());

        return 0;
    }
};

LRFormula1::LRFormula1(double x0, double y0, double rmax) :
    x0(x0), y0(y0), rmax(rmax)
{
    Init();
}

LRFormula1* LRFormula1::clone() const 
{ 
    LRFormula1 *copy = new LRFormula1(*this);
    return copy;
}

void LRFormula1::Init()
{
    rmin2 = rmin*rmin;
    rmax2 = rmax*rmax;
    xmin = x0-rmax;
    xmax = x0+rmax;
    ymin = y0-rmax;
    ymax = y0+rmax;
    init_done = true;
}

LRFormula1::LRFormula1(const Json &json)
{
    if (!json["rmax"].is_number()) {
        json_err = std::string("rmax missing or not a number");
        return;
    }

// x0, y0 or rmin key not present in JSON object defaults to 0
    x0   = json["x0"].number_value();
    y0   = json["y0"].number_value();
    rmin = json["rmin"].number_value();

    rmax = json["rmax"].number_value();
    if (rmax <= rmin) {
        json_err = std::string("rmax <= rmin");
        return;
    }

    Init();

// get function name and parameters
    if (!json["function"].is_string()) {
        json_err = std::string("function name missing or isn't a string");
        return;
    }

    std::string fname = json["function"].string_value();

    if (fname == "Gauss")
        ftype = Gauss;
    else if (fname == "Sech")
        ftype = Sech;
    else if (fname == "Sech2")
        ftype = Sech2;
    else if (fname == "Cauchy")
        ftype = Cauchy;
    else {
        json_err = std::string("unknown function name");
        return;
    }

    if (!json["a"].is_number() || !json["c"].is_number() || !json["s"].is_number()) {
        json_err = std::string("one or more of function parameters is missing or not a number");
        return;
    }
    a = json["a"].number_value();
    c = json["c"].number_value();
    s = json["s"].number_value();

    valid = true;
}

LRFormula1::LRFormula1(std::string &json_str) : LRFormula1(Json::parse(json_str, gjson_err)) {}

//LRFormula1::~LRFormula1() {;}

bool LRFormula1::isReady() const
{
    return true; // bsr && bsr->IsReady();
}

bool LRFormula1::inDomain(double x, double y, double /*z*/) const
{
    double r2 = R2(x,y);
    return (r2 < rmax2) && (r2 > rmin2);
}

double LRFormula1::eval(double x, double y, double /*z*/) const
{
    return evalAxial(R(x, y));
}

double LRFormula1::evalAxial(double r) const
{
    if (!isReady())
        return 0;

    double t = r/s;
    double sch;
    
    switch (ftype) {
        case Gauss:
            return a*exp(-t*t/2) + c;
        case Sech:
            return a/cosh(t) + c;
        case Sech2:
            sch = 1/cosh(t);
            return a*sch*sch + c;
        case Cauchy:
            return a/(1+t*t) + c;
    }
}

double LRFormula1::evalDrv(double r) const
{
    if (!isReady())
        return 0;

    double t = r/s;
    double sch;
    
    switch (ftype) {
        case Gauss:
            return -a*exp(-t*t/2)*t/s;
        case Sech:
            return -a/tanh(t)/cosh(t)/s;
        case Sech2:
            sch = 1/cosh(t);
            return -2*a*sch*sch*tanh(t)/s;
        case Cauchy:
            t = s*s + r*r;
            return -2*a*s*s*r/t/t;
    }
}

double LRFormula1::evalDrvX(double x, double y, double /*z*/) const
{
    double r = R(x,y);
    return r < rzerod ? 0 : evalDrv(r)*(x-x0)/r;
}

double LRFormula1::evalDrvY(double x, double y, double /*z*/) const
{
    double r = R(x,y);
    return r < rzerod ? 0 : evalDrv(r)*(y-y0)/r;
}

bool LRFormula1::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vr, va;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) && d.good) )
            continue;
        vr.push_back(R(d.x, d.y));
        va.push_back(d.val);
    }

 //  map std::vectors to ArrayXd objects
 // caveat: the data memory is shared between two objects!
    Eigen::Map<Eigen::ArrayXd> x(vr.data(), static_cast<Eigen::Index>(vr.size()));
    Eigen::Map<Eigen::ArrayXd> y(va.data(), static_cast<Eigen::Index>(va.size()));

// initial guess: a, s, c
    Eigen::VectorXd p(3);
    double amax = *std::max_element(va.begin(), va.end());
    p << amax, 1., 0.;

    // Wrap with numerical differentiation
    UniFunctor functor(x, y, ftype);

    Eigen::NumericalDiff<UniFunctor> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<UniFunctor>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = 200;   // max iterations
    lm.parameters.ftol = 1e-10;
    lm.parameters.xtol = 1e-10;

    auto status = lm.minimize(p);
    if (status != 1) {
//        error_msg = std::string("Formula1: LM fit failed with status ") + std::to_string(status);
        throw std::runtime_error(std::string("Formula1: LM fit failed with status ") + std::to_string(status));
        return false;
    }

    a = p(0);
    s = p(1);
    c = p(2);
    return true;
}

bool LRFormula1::addData(const std::vector <LRFdata> &data)
{
    if (!h1)
        h1 = new ProfileHist1D(nbins, 0, rmax);

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) && d.good) )
            continue;
        h1->Fill(R(d.x, d.y), d.val);
    }
    return true;
}

bool LRFormula1::doFit()
{
    if (!h1 || h1->GetEntriesTotal() == 0. ) {
//        error_msg = std::string("Formula1: No binned data to fit. Call AddData() first");
        throw std::runtime_error(std::string("Formula1: No binned data to fit. Call AddData() first"));
        return false;
    }

    std::vector<double> vx(nbins), vdata(nbins), vw(nbins);
// extract the accumulated binned data from the profile histogram
    for (int ix=0; ix<nbins; ix++) {
        vx[ix] = (h1->GetBinCenterX(ix));
        vdata[ix] = (h1->GetBinMean(ix));
        vw[ix] = (h1->GetBinEntries(ix));
    }

    Eigen::Map<Eigen::ArrayXd> x(vx.data(), nbins);
    Eigen::Map<Eigen::ArrayXd> y(vdata.data(), nbins);
    Eigen::Map<Eigen::ArrayXd> w(vw.data(), nbins);

    // vector with the fit parameters (a, s, c)
    Eigen::VectorXd p(3);

    // starting point for optimization 
    if (a == 0.) { // make a guess
        double amax = y.maxCoeff();
        double sigma = sqrt((x*x*y).sum()/y.sum());
        p << amax, sigma, 0;
    } else {       // use current values
        p << a, s, c; 
    }

    // Wrap with numerical differentiation
    UniFunctorW functor(x, y, w, ftype);

    Eigen::NumericalDiff<UniFunctorW> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<UniFunctorW>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = 200;   // max iterations
    lm.parameters.ftol = 1e-10;
    lm.parameters.xtol = 1e-10;

    auto status = lm.minimize(p);
    if (status != 1) {
//        error_msg = std::string("Formula1: LM fit failed with status ") + std::to_string(status);
        throw std::runtime_error(std::string("Formula1: LM fit failed with status ") + std::to_string(status));
        return false;
    }

    a = p(0);
    s = p(1);
    c = p(2);
    return true;
}

double LRFormula1::GetRatio(LRF* other_base) const
{
    LRFormula1 *other = dynamic_cast<LRFormula1*>(other_base);
    if (!other || !(other->h1))
        return -1;
    
    ProfileHist *ha = h1;
    ProfileHist *hb = other->h1;

    int nbins = ha->GetBinsTotal();
    if (hb->GetBinsTotal() != nbins)
        return -1;

    double sumxy = 0.;
    double sumxx = 0.;

    for (int i=0; i<nbins; i++) {
        if (ha->GetFlatBinEntries(i) && hb->GetFlatBinEntries(i))  { // must have something in both bins
            double z0 = ha->GetFlatBinMean(i);
            sumxy += z0*hb->GetFlatBinMean(i);
            sumxx += z0*z0;
        }
    }

    return ( sumxx > 0. ? sumxy/sumxx
                        : -1 );
}

void LRFormula1::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["rmax"] = rmax;
    json["rmin"] = rmin;
    json["x0"] = x0;
    json["y0"] = y0;

    if (ftype == Gauss)
        json["function"] = "Gauss";
    else if (ftype == Sech)
        json["function"] = "Sech";
    else if (ftype == Sech2)
        json["function"] = "Sech2";
    else if (ftype == Cauchy)
        json["function"] = "Cauchy";
    else
        throw std::runtime_error("Formula1: trying to export a function of unknown type to JSON");

    json["a"] = a;
    json["c"] = c;
    json["s"] = s; 
}

