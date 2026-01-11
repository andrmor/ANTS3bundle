#include "lrformulaxy.h"
#include "json11.hpp"
#include "profileHist.h"

#include "functor.h"
#include "wformula.h"

#include <stdexcept>
#include <algorithm>

// Universal functor
// 3 - number of fixed constants in vf
// ToDo: calculate it automatically (probably add this to WFormula)
struct FunctorXY : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Eigen::VectorXd a;
    WFormula *vf;

    FunctorXY(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Eigen::VectorXd& a_, WFormula *vf_)
        : Functor<double>(vf_->GetConstCount()-3, x_.size()), x(x_), y(y_), a(a_), vf(vf_){}

    // Compute residuals: f(p) = model(p) - a
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        for (size_t i=3; i<vf->GetConstCount(); i++)
            vf->SetConstant(i, p[i-3]);

        fvec = vf->Eval(x.array(), y.array()) - a.array();
        return 0;
    }
};

// Universal functor with weights
struct FunctorXYW : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Eigen::VectorXd a;
    Eigen::VectorXd w;
    WFormula *vf;

    FunctorXYW(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Eigen::VectorXd& a_,
        const Eigen::VectorXd& w_, WFormula *vf_)
        : Functor<double>(vf_->GetConstCount()-1, x_.size()), x(x_), y(y_), a(a_), w(w_), vf(vf_){}

    // Compute weighted residuals: f(p) = (model(p) - a) * sqrt(w)
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        for (size_t i=3; i<vf->GetConstCount(); i++)
            vf->SetConstant(i, p[i-3]);

        fvec = (vf->Eval(x.array(), y.array()) - a.array()) * sqrt(w.array());
        return 0;
    }
};

LRFormulaXY::LRFormulaXY(double xmin, double xmax, double ymin, double ymax)
{
    this->xmin = xmin; this->xmax = xmax;
    this->ymin = ymin; this->ymax = ymax;
    Init();
}

LRFormulaXY* LRFormulaXY::clone() const 
{ 
    LRFormulaXY *copy = new LRFormulaXY(*this);
    copy->vf = vf ? new WFormula(*vf) : nullptr;
    return copy;
}

void LRFormulaXY::Init()
{
    double rmax2 = std::max({xmax*xmax+ymax*ymax, xmin*xmin+ymax*ymax, 
                             xmin*xmin+ymin*ymin, xmax*xmax+ymin*ymin});
    rmax = sqrt(rmax2);
    thread_safe = false;
}

LRFormulaXY::LRFormulaXY(const Json &json)
{
    if (!json["xmin"].is_number() || !json["xmax"].is_number() ||
        !json["ymin"].is_number() || !json["ymax"].is_number())
        return;
    xmin = json["xmin"].number_value();
    xmax = json["xmax"].number_value();
    ymin = json["ymin"].number_value();
    ymax = json["ymax"].number_value();
    if (xmax <= xmin || ymax<=ymin)
        return;
    Init();

// x0 or y0 key not present in JSON object defaults to 0
    x0 = json["x0"].number_value();
    y0 = json["y0"].number_value();
    
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

std::string LRFormulaXY::InitVF()
{
    delete vf;
    vf = new WFormula();
    vf -> AddConstant("x0", x0);
    vf -> AddConstant("y0", y0);
//    std::cout << "InitVF\n";
    for (size_t i = 0; i<parnames.size(); i++) {
//        std::cout << "name: " << parnames[i] << " = " << parvals[i] << std::endl;
        vf->AddConstant(parnames[i], parvals[i]);
    }

    vf->AddVariable("x");
    vf->AddVariable("y");

    int errpos = vf->ParseExpr(expression);
    if (errpos != 1024)
        return("Parsing error at " + std::to_string(errpos));
    if (!vf->Validate())
        return("Validation error: " + vf->GetErrorString());

    valid = true;    
    return std::string("");
}

LRFormulaXY::LRFormulaXY(std::string &json_str) : LRFormulaXY(Json::parse(json_str, json_err)) {}

LRFormulaXY::~LRFormulaXY()
{
    delete vf;
}

bool LRFormulaXY::isReady() const
{
    return valid;
}

bool LRFormulaXY::inDomain(double x, double y, double /*z*/) const
{
    return x>xmin && x<xmax && y>ymin && y<ymax;
}

double LRFormulaXY::getRmax() const
{
    return rmax;
}

double LRFormulaXY::eval(double x, double y, double /*z*/) const
{
    if (!isReady())
        return 0;
//    std::cout << r << std::endl;
//    std::cout << Eigen::ArrayXd::Constant(1, r) << std::endl;
//    std::cout << vf->Eval(Eigen::ArrayXd::Constant(1, r)) << std::endl;

    return vf->Eval(Eigen::ArrayXd::Constant(1, x), Eigen::ArrayXd::Constant(1, y))[0];
}

void LRFormulaXY::SetParameter(std::string name, double val)
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

double LRFormulaXY::GetParameter(std::string name)
{
    const auto it = std::find(parnames.begin(), parnames.end(), name);
    return it != parnames.end() ? parvals[it - parnames.begin()] : nan("");
}

bool LRFormulaXY::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vx, vy, va;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) &&  d.good) )
            continue;
        vx.push_back(d.x);
        vy.push_back(d.y);
        va.push_back(d.val);
    }

    //  map std::vectors to ArrayXd objects
    // caveat: the data memory is shared between two objects!
    Eigen::Map<Eigen::VectorXd> x(vx.data(), static_cast<Eigen::Index>(vx.size()));
    Eigen::Map<Eigen::VectorXd> y(vy.data(), static_cast<Eigen::Index>(vy.size()));
    Eigen::Map<Eigen::VectorXd> a(va.data(), static_cast<Eigen::Index>(va.size()));

    // use current parameter set as initial guess
    Eigen::VectorXd p(parvals.size());
    for (size_t i=0; i<parvals.size(); i++)
        p(i) = parvals[i];

    // Wrap with numerical differentiation
    FunctorXY functor(x, y, a, vf);

    Eigen::NumericalDiff<FunctorXY> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<FunctorXY>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = maxfev;   // max iterations
    lm.parameters.ftol = ftol;
    lm.parameters.xtol = xtol;

    auto status = lm.minimize(p);
    fit_status = status;
    if (status < 1 || status > 4) {
        error_msg = std::string("FormulaV: LM fit failed with status ") + std::to_string(status);
//        throw std::runtime_error(std::string("FormulaXY: LM fit failed with status ") + std::to_string(status));
        return false;
    }

    for (size_t i=0; i<parvals.size(); i++)
        parvals[i] = p(i);

    return true;
}

bool LRFormulaXY::addData(const std::vector <LRFdata> &data)
{
    if (!h1)
        h1 = new ProfileHist2D(nbinsx, xmin, xmax, nbinsy, ymin, ymax);

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) && d.good) )
            continue;
        h1->Fill(d.x, d.y, d.val);
    }
    return true;
}

bool LRFormulaXY::addDataPy(const std::vector <Vec4data> &data){
    if (!h1)
        h1 = new ProfileHist2D(nbinsx, xmin, xmax, nbinsy, ymin, ymax);

    for (auto d : data) {
        if ( !inDomain(d[0], d[1]) )
            continue;
        h1->Fill(d[0], d[1], d[3]);
    }
    return true;
}

bool LRFormulaXY::doFit()
{
    if (!h1 || h1->GetEntriesTotal() == 0. ) {
//        error_msg = std::string("FormulaV: No binned data to fit. Call AddData() first");
        throw std::runtime_error(std::string("FormulaXY: No binned data to fit. Call AddData() first"));
        return false;
    }

    std::vector<double> vx, vy, va, vw;
    // extract the accumulated binned data from the profile histogram
    for (int ix=0; ix<nbinsx; ix++)
        for (int iy=0; iy<nbinsy; iy++) {
        double w = h1->GetBinEntries(ix, iy);
        if (w > 0.5) {
            vx.push_back(h1->GetBinCenterX(ix));
            vy.push_back(h1->GetBinCenterY(iy));
            va.push_back(h1->GetBinMean(ix, iy));
            vw.push_back(w);
        }
    }

    int nbins = vw.size(); //nbinsx * nbinsy;
    Eigen::Map<Eigen::VectorXd> x(vx.data(), nbins);
    Eigen::Map<Eigen::VectorXd> y(vy.data(), nbins);
    Eigen::Map<Eigen::VectorXd> a(va.data(), nbins);
    Eigen::Map<Eigen::VectorXd> w(vw.data(), nbins);

    // use current parameter set as initial guess
    Eigen::VectorXd p(parvals.size());
    for (size_t i=0; i<parvals.size(); i++)
        p(i) = parvals[i];

    // Wrap with numerical differentiation
    FunctorXYW functor(x, y, a, w, vf);

    Eigen::NumericalDiff<FunctorXYW> numDiff(functor);
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<FunctorXYW>> lm(numDiff);

    // Run LM optimization
    lm.parameters.maxfev = maxfev;   // max iterations
    lm.parameters.ftol = ftol;
    lm.parameters.xtol = xtol;
    qtol = lm.parameters.xtol;

    auto status = lm.minimize(p);
    fit_status = status;
    fvec = lm.fvec;
    fjac = lm.fjac;

    if (status < 1 || status > 4) {
        error_msg = std::string("FormulaXY: weighted LM fit failed with status ") + std::to_string(status);
//        throw std::runtime_error(std::string("FormulaXY: weighted LM fit failed with status ") + std::to_string(status));
        return false;
    }

    for (size_t i=0; i<parvals.size(); i++)
        parvals[i] = p(i);

    return true;
}

void LRFormulaXY::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["xmin"] = xmin;
    json["xmax"] = xmax;
    json["ymin"] = ymin;
    json["ymax"] = ymax;
    json["x0"] = x0;
    json["y0"] = y0;

    json["expression"] = expression;
    json["parnames"] = parnames;
    json["parvals"] = parvals;

}

double LRFormulaXY::GetRatio(LRF* other_base) const
{
    LRFormulaXY *other = dynamic_cast<LRFormulaXY*>(other_base);
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

