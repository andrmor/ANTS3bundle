#include "lrfaxial.h"
#include "bspline123d.h"
#include "bsfit123.h"
#include "compress.h"
#include "json11.hpp"
#include "profileHist.h"

//#include <iostream>
#include <string>

LRFaxial::LRFaxial(double rmax, int nint) :
    rmax(rmax), nint(nint)
{
    bsr = new Bspline1d(rmin, rmax, nint);
    Init();
}

LRFaxial* LRFaxial::clone() const 
{ 
    LRFaxial *copy = new LRFaxial(*this);
    copy->bsr = bsr ? new Bspline1d(*bsr) : nullptr;
    copy->compress = compress ? compress->clone() : nullptr;
    copy->bsfit = bsfit ? bsfit->clone() : nullptr;
    return copy;
}

void LRFaxial::Init()
{
    rmin2 = rmin*rmin;
    rmax2 = rmax*rmax;
    xmin = x0-rmax;
    xmax = x0+rmax;
    ymin = y0-rmax;
    ymax = y0+rmax;
    init_done = true;
}

void LRFaxial::SetOrigin(double x0, double y0)
{
    this->x0 = x0;
    this->y0 = y0;
    Init();
}

void LRFaxial::SetRmin(double val)
{
    rmin = std::max(val, 0.);
    rmin2 = rmin*rmin;
    delete bsr;
    bsr = new Bspline1d(Rho(rmin), Rho(rmax), nint);

    Init(); // Andr: to update xmin xmax etc
}

void LRFaxial::SetRmax(double val)
{
    rmax = val;
    rmax2 = rmax*rmax;
    delete bsr;
    bsr = new Bspline1d(Rho(rmin), Rho(rmax), nint);

    Init(); // Andr: to update xmin xmax etc
}

void LRFaxial::SetCompression(Compress1d *compress)
{
    delete this->compress;
    this->compress = compress;
    delete bsr;
    bsr = new Bspline1d(Rho(rmin), Rho(rmax), nint);
}

void LRFaxial::SetSpline(Bspline1d *spline) 
{
    delete bsr; 
    bsr = spline;
}

LRFaxial::LRFaxial(const Json &json)
{
    if (!json["rmax"].is_number()) {
        json_err = std::string("rmax missing or not a number");
        return;
    }

// if x0, y0 or rmin key is not present in JSON object it defaults to 0
// providing compatibility with previous version
    x0   = json["x0"].number_value();
    y0   = json["y0"].number_value();
    rmin = json["rmin"].number_value();
    rmax = json["rmax"].number_value();
    nint = json["nint"].number_value();
    if (rmax <= rmin) {
        json_err = std::string("rmax <= rmin");
        return;
    }
    if (json["compression"].is_object())
        compress = Compress1d::Factory(json["compression"]);
    if (json["constraints"].is_array()) {
        Json::array cstr = json["constraints"].array_items();
        for (int i=0; i<cstr.size(); i++) {
            std::string name(cstr[i].string_value());
            if (name == "non-negative") non_negative = true;
            if (name == "non-increasing") non_increasing = true;
            if (name == "flattop") flattop = true;
        }
    }

    Init();

// up to this point shared with axial3d

    if (json["response"]["bspline3"].is_object()) {
        bsr = new Bspline1d(json["response"]["bspline3"]);
        if (bsr->isInvalid()) {
            json_err = std::string("LRFaxial: invalid response");
            return;
        }
        nint = bsr->GetNint();
    } else if (nint < 1) {
        json_err = std::string("LRFaxial: nint is invalid or missing");
        return;
    } else {
        bsr = new Bspline1d(Rho(rmin), Rho(rmax), nint);
    }

    valid = true;
}

LRFaxial::LRFaxial(std::string &json_str) : LRFaxial(Json::parse(json_str, json_err)) {}

LRFaxial::~LRFaxial()
{
    delete bsr;
    delete compress;
    delete bsfit;
}

std::vector <double> LRFaxial::GetNodes() const
{
    std::vector <double> nodes = bsr->GetNodes();
    if (compress) 
        for (auto &node : nodes)
            node = compress->Rho2R(node);
    return nodes;
}

bool LRFaxial::isReady() const
{
    return true; // bsr && bsr->IsReady();
}

bool LRFaxial::inDomain(double x, double y, double /*z*/) const
{
    double r2 = R2(x,y);
    return (r2 < rmax2) && (r2 > rmin2);
}

double LRFaxial::Rho(double r) const
{
    return ( compress ? compress->Rho(r)
                      : r );
}

double LRFaxial::Rho(double x, double y) const
{
    return ( compress ? compress->Rho(R(x, y))
                      : R(x, y) );
}

double LRFaxial::RhoDrvX(double x, double y) const
{
    double drdx = (x-x0)/R(x, y);
    return ( compress ? compress->RhoDrv(R(x, y))*drdx
                      : drdx );
}

double LRFaxial::RhoDrvY(double x, double y) const
{
    double drdy = (y-y0)/R(x, y);
    return ( compress ? compress->RhoDrv(R(x, y))*drdy
                      : drdy );
}

double LRFaxial::eval(double x, double y, double /*z*/) const
{
    return ( isReady() ? bsr->Eval(Rho(x, y))
                       : 0 );
}

double LRFaxial::evalraw(double x, double y, double /*z*/) const
{
    return ( isReady() ? bsr->Eval(x)
                       : 0 );
}

double LRFaxial::evalAxial(double r) const
{
    return isReady() ? bsr->Eval(Rho(r)) : 0;
}

double LRFaxial::evalDrvX(double x, double y, double /*z*/) const
{
    return ( isReady() ? bsr->EvalDrv(Rho(x, y))*RhoDrvX(x, y)
                       : 0 );
}

double LRFaxial::evalDrvY(double x, double y, double /*z*/) const
{
    return ( isReady() ? bsr->EvalDrv(Rho(x, y))*RhoDrvY(x, y)
                       : 0 );
}

BSfit1D *LRFaxial::InitFit()
{
    if (!flattop && !non_negative && !non_increasing)
        return new BSfit1D(bsr);

    ConstrainedFit1D *cf = new ConstrainedFit1D(bsr);
    if (non_increasing) cf->ForceNonIncreasing();
    if (non_negative) cf->ForceNonNegative();
    if (flattop) cf->FixDrvLeft(0.);
    return cf;
}

bool LRFaxial::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vr, va;
    bool status;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) && d.good) )
            continue;
        vr.push_back(Rho(d.x, d.y));
        va.push_back(d.val);
    }

    BSfit1D *F = InitFit();
    if (binned) {
        F->AddData(vr, va);
        status = F->BinnedFit();
    } else {
        status = F->Fit(vr, va);
    }

    if (status) {
        delete bsr;
        bsr = F->MakeSpline();
    } 

    delete F;
    valid = status;
    return status;
}

bool LRFaxial::addData(const std::vector <LRFdata> &data)
{
    if (!bsfit)
        bsfit = InitFit();

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) && d.good) )
            continue;
        bsfit->AddData(Rho(d.x, d.y), d.val);
    }
    return true;
}

bool LRFaxial::doFit()
{
    if (!bsfit)
        return false;

    if (bsfit->BinnedFit()) {
        delete bsr;
        bsr = bsfit->MakeSpline();
        valid = true;
        return true;        
    } else {
        valid = false;
        return false;
    }
}

void LRFaxial::clearData()
{
    if (bsfit)
        bsfit->ClearData();
}

ProfileHist *LRFaxial::GetHist()
{
    if (!bsfit)
        bsfit = InitFit();

    return bsfit->GetHist(); 
}

double LRFaxial::GetRatio(LRF* other_base) const
{
    LRFaxial *other = dynamic_cast<LRFaxial*>(other_base);
    if (!other || !(other->bsfit))
        return -1;
    
    ProfileHist *h0 = bsfit->GetHist();
    ProfileHist *h1 = other->bsfit->GetHist();

    int nbins = h0->GetBinsTotal();
    if (h1->GetBinsTotal() != nbins)
        return -1;

    double sumxy = 0.;
    double sumxx = 0.;

    for (int i=0; i<nbins; i++) {
        if (h0->GetFlatBinEntries(i) && h1->GetFlatBinEntries(i))  { // must have something in both bins
            double z0 = h0->GetFlatBinMean(i);
            sumxy += z0*h1->GetFlatBinMean(i);
            sumxx += z0*z0;
        }
    }

    return ( sumxx > 0. ? sumxy/sumxx
                        : -1 );
}

/* double LRFaxial::fitRData(int npts, const double *r, const double *data)
{
    std::vector <double> vr;
    std::vector <double> va;

    for (int i=0; i<npts; i++) {
        vr.push_back(Rho(r[i]));
        va.push_back(data[i]);
    }

    bsr = new Bspline1d(0., Rho(rmax), nint);

    BSfit1D F(bsr);
    if (flattop) F.SetConstraintEven();
    if (non_negative) F.SetConstraintNonNegative();
    if (non_increasing) F.SetConstraintNonIncreasing();

    F.Fit(va.size(), &vr[0], &va[0]);
    valid = bsr->IsReady();
    return F.GetResidual();
} */

// TODO: make it safer (convert into reference?)
const Bspline1d *LRFaxial::getSpline() const
{
    return bsr;
}

void LRFaxial::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["rmax"] = rmax;
    json["rmin"] = rmin;
    json["x0"] = x0;
    json["y0"] = y0;

    std::vector <std::string> cstr;
    if (non_negative) cstr.push_back("non-negative");
    if (non_increasing) cstr.push_back("non-increasing");
    if (flattop) cstr.push_back("flattop");
    if (cstr.size() > 0)
        json["constraints"] = cstr;

    if (bsr) {
        Json_object json1;
        json1["bspline3"] = bsr->GetJsonObject();
        json["response"] = json1;
    } else {
        json["nint"] = nint;
    }
    
    if (compress) 
        json["compression"] = compress->GetJsonObject();
}
