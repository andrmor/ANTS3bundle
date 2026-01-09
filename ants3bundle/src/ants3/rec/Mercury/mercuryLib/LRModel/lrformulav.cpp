#include "lrformulav.h"
//#include "Eigen/src/Core/Matrix.h"
#include "json11.hpp"
#include "lrf.h"
#include "profileHist.h"

//#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>

#include "functor.h"

// Universal functor
struct FunctorV : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    WFormula *vf;

    FunctorV(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, WFormula *vf_)
        : Functor<double>(vf_->GetConstCount()-1, x_.size()), x(x_), y(y_), vf(vf_){}

    // Compute residuals: f(p) = model(p) - y
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        for (size_t i=1; i<vf->GetConstCount(); i++)
            vf->SetConstant(i, p[i-1]);

        fvec = vf->Eval(x.array()) - y.array();
        return 0;
    }
};

// Universal functor with weights
struct FunctorVW : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Eigen::VectorXd w;
    WFormula *vf;

    FunctorVW(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Eigen::VectorXd& w_, WFormula *vf_)
        : Functor<double>(vf_->GetConstCount()-1, x_.size()), x(x_), y(y_), w(w_), vf(vf_){}

    // Compute weighted residuals: f(p) = (model(p) - y) * sqrt(w)
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        for (size_t i=1; i<vf->GetConstCount(); i++)
            vf->SetConstant(i, p[i-1]);

        fvec = (vf->Eval(x.array()) - y.array()) * sqrt(w.array());
        return 0;
    }
};

LRFormulaV::LRFormulaV(double x0, double y0, double rmax) :
    x0(x0), y0(y0), rmax(rmax)
{
    Init();
}

LRFormulaV* LRFormulaV::clone() const 
{ 
    LRFormulaV *copy = new LRFormulaV(*this);
    copy->vf = vf ? new WFormula(*vf) : nullptr;
    return copy;
}

void LRFormulaV::Init()
{
    rmin2 = rmin*rmin;
    rmax2 = rmax*rmax;
    xmin = x0-rmax;
    xmax = x0+rmax;
    ymin = y0-rmax;
    ymax = y0+rmax;
    init_done = true;
}

LRFormulaV::LRFormulaV(const Json &json)
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

// get WFormula expression and parameters

    parnames.clear();
    parvals.clear();
    Json::array names, vals;
    if (json["parnames"].is_array())
        names = json["parnames"].array_items();
    if (json["parvals"].is_array())
        vals = json["parvals"].array_items();
    for (size_t i = 0; i<names.size(); i++) {
        parnames.push_back(names[i].string_value());
        parvals.push_back(vals[i].number_value());
    }
    
    if (json["expression"].is_string())
        expression = json["expression"].string_value();

    InitVF();
}

LRFormulaV::LRFormulaV(std::string &json_str) : LRFormulaV(Json::parse(json_str, gjson_err)) {}

std::string LRFormulaV::InitVF()
{
    delete vf;
    vf = new WFormula();
//    std::cout << "InitVF\n";
    for (size_t i = 0; i<parnames.size(); i++) {
//        std::cout << "name: " << parnames[i] << " = " << parvals[i] << std::endl;
        vf->AddConstant(parnames[i], parvals[i]);
    }

    vf->AddVariable("r");

    int errpos = vf->ParseExpr(expression);
    if (errpos != 1024)
        return("Parsing error at " + std::to_string(errpos));
    if (!vf->Validate())
        return("Validation error: " + vf->GetErrorString());

    valid = true;    
    return std::string("");
}

bool LRFormulaV::isReady() const
{
    return valid; // bsr && bsr->IsReady();
}

bool LRFormulaV::inDomain(double x, double y, double /*z*/) const
{
    double r2 = R2(x,y);
    return (r2 < rmax2) && (r2 > rmin2);
}

double LRFormulaV::eval(double x, double y, double /*z*/) const
{
    return evalAxial(R(x, y));
}

double LRFormulaV::evalAxial(double r) const
{
    if (!isReady())
        return 0;
//    std::cout << r << std::endl;
//    std::cout << Eigen::ArrayXd::Constant(1, r) << std::endl;
//    std::cout << vf->Eval(Eigen::ArrayXd::Constant(1, r)) << std::endl;
    return vf->Eval(Eigen::ArrayXd::Constant(1, r))[0];
}
/*
double LRFormulaV::evalDrv(double r) const
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
*/
// double LRFormulaV::evalDrvX(double x, double y, double /*z*/) const
// {
//     double r = R(x,y);
//     return r < rzerod ? 0 : evalDrv(r)*(x-x0)/r;
// }

// double LRFormulaV::evalDrvY(double x, double y, double /*z*/) const
// {
//     double r = R(x,y);
//     return r < rzerod ? 0 : evalDrv(r)*(y-y0)/r;
// }

void LRFormulaV::SetParameter(std::string name, double val)
{
    const auto it = std::find(parnames.begin(), parnames.end(), name);
    if (it == parnames.end()) {
        parnames.push_back(name);
        parvals.push_back(val);
    } else {
        size_t n = it - parnames.begin();
        parnames[n] = name;
        parvals[n] = val;
    }
}

double LRFormulaV::GetParameter(std::string name)
{
    const auto it = std::find(parnames.begin(), parnames.end(), name);
    return it != parnames.end() ? parvals[it - parnames.begin()] : nan("");
}

//std::vector<double> LRFormulaV::GetParVector() {return parvals;}

bool LRFormulaV::fitData(const std::vector <LRFdata> &data)
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
    Eigen::Map<Eigen::VectorXd> x(vr.data(), static_cast<Eigen::Index>(vr.size()));
    Eigen::Map<Eigen::VectorXd> y(va.data(), static_cast<Eigen::Index>(va.size()));

// use current parameter set as initial guess
    Eigen::VectorXd p(parvals.size());
    for (size_t i=0; i<parvals.size(); i++)
        p(i) = parvals[i];

    // Wrap with numerical differentiation
    FunctorV functor(x, y, vf);

    Eigen::NumericalDiff<FunctorV> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<FunctorV>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = maxfev;   // max iterations
    lm.parameters.ftol = ftol;
    lm.parameters.xtol = xtol;

    auto status = lm.minimize(p);
    fit_status = status;
    if (status < 1 || status > 4) {
//        error_msg = std::string("FormulaV: LM fit failed with status ") + std::to_string(status);
        throw std::runtime_error(std::string("FormulaV: LM fit failed with status ") + std::to_string(status));
        return false;
    }

    for (size_t i=0; i<parvals.size(); i++)
        parvals[i] = p(i);

    return true;
}

bool LRFormulaV::addData(const std::vector <LRFdata> &data)
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

bool LRFormulaV::addDataPy(const std::vector <Vec4data> &data)
{
    if (!h1)
        h1 = new ProfileHist1D(nbins, 0, rmax);

    for (auto d : data) {
        if ( !inDomain(d[0], d[1]) )
            continue;
        h1->Fill(R(d[0], d[1]), d[3]);
    }
    return true;
}

bool LRFormulaV::doFit()
{
    if (!h1 || h1->GetEntriesTotal() == 0. ) {
//        error_msg = std::string("FormulaV: No binned data to fit. Call AddData() first");
        throw std::runtime_error(std::string("FormulaV: No binned data to fit. Call AddData() first"));
        return false;
    }

    std::vector<double> vx, va, vw;
    // extract the accumulated binned data from the profile histogram
    for (int ix=0; ix<nbins; ix++) {
        double w = h1->GetBinEntries(ix);
        if (w > 0.5) {
            vx.push_back(h1->GetBinCenterX(ix));
            va.push_back(h1->GetBinMean(ix));
            vw.push_back(w);
        }
    }

    Eigen::Map<Eigen::VectorXd> x(vx.data(), static_cast<Eigen::Index>(vx.size()));
    Eigen::Map<Eigen::VectorXd> y(va.data(), static_cast<Eigen::Index>(va.size()));
    Eigen::Map<Eigen::VectorXd> w(vw.data(), static_cast<Eigen::Index>(vw.size()));

    // use current parameter set as initial guess
    Eigen::VectorXd p(parvals.size());
    for (size_t i=0; i<parvals.size(); i++)
        p(i) = parvals[i];

    // Wrap with numerical differentiation
    FunctorVW functor(x, y, w, vf);

    Eigen::NumericalDiff<FunctorVW> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<FunctorVW>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = maxfev;   // max iterations
    lm.parameters.ftol = ftol;
    lm.parameters.xtol = xtol;

    auto status = lm.minimize(p);
    fit_status = status;
    if (status < 1 || status > 4) {
//        error_msg = std::string("FormulaV: LM fit failed with status ") + std::to_string(status);
        throw std::runtime_error(std::string("FormulaV: LM fit failed with status ") + std::to_string(status));
        return false;
    }

    for (size_t i=0; i<parvals.size(); i++)
        parvals[i] = p(i);

    return true;
}

double LRFormulaV::GetRatio(LRF* other_base) const
{
    LRFormulaV *other = dynamic_cast<LRFormulaV*>(other_base);
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

void LRFormulaV::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["rmax"] = rmax;
    json["rmin"] = rmin;
    json["x0"] = x0;
    json["y0"] = y0;

    json["expression"] = expression;

    // we store names and values of the parameters in separate arrays
    // another option vould be an array of objects {name, val} 
    // not sure which way is better...

    json["parnames"] = parnames;
    json["parvals"] = parvals;
}

