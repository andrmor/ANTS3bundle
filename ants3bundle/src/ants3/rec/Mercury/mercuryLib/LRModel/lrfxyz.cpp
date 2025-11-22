#include "lrfxyz.h"
#include "bspline123d.h"
#include "bsfit123.h"
#include "json11.hpp"
#include "profileHist.h"

#include <iostream>

LRFxyz::LRFxyz(double xmin, double xmax, int nintx, double ymin, double ymax, int ninty,
                double zmin, double zmax, int nintz): 
    nintx(nintx), ninty(ninty), nintz(nintz)
{
    this->xmin = xmin; this->xmax = xmax;
    this->ymin = ymin; this->ymax = ymax;
    this->zmin = zmin; this->zmax = zmax;
    bsr = new Bspline3d(xmin, xmax, nintx, ymin, ymax, ninty, zmin, zmax, nintz);
    Init();
}

LRFxyz* LRFxyz::clone() const 
{ 
    LRFxyz *copy = new LRFxyz(*this);
    copy->bsr = bsr ? new Bspline3d(*bsr) : nullptr;
    copy->bsfit3d = bsfit3d ? bsfit3d->clone() : nullptr;
    return copy;
}

void LRFxyz::Init()
{
    double rmax2 = std::max({xmax*xmax+ymax*ymax, xmin*xmin+ymax*ymax, 
                             xmin*xmin+ymin*ymin, xmax*xmax+ymin*ymin});
    rmax = sqrt(rmax2);
}

LRFxyz::LRFxyz(const Json &json)
{
    if (!json["xmin"].is_number() || !json["xmax"].is_number() ||
        !json["ymin"].is_number() || !json["ymax"].is_number() || 
        !json["zmin"].is_number() || !json["zmax"].is_number())
        return;
    xmin = json["xmin"].number_value();
    xmax = json["xmax"].number_value();
    ymin = json["ymin"].number_value();
    ymax = json["ymax"].number_value();
    zmin = json["zmin"].number_value();
    zmax = json["zmax"].number_value();
    nintx = json["nintx"].number_value();
    ninty = json["ninty"].number_value();
    nintz = json["nintz"].number_value();
    if (xmax <= xmin || ymax<=ymin || zmax<=zmin)
        return;

    if (json["constraints"].is_array()) {
        Json::array cstr = json["constraints"].array_items();
        for (int i=0; i<cstr.size(); i++) {
            std::string name(cstr[i].string_value());
            if (name == "non-negative") non_negative = true;
        }
    }

// read response
    if (json["response"].is_object()) {
        bsr = new Bspline3d(json["response"]);
        if (bsr->isInvalid()) {
            json_err = std::string("LRFxyz: invalid response");
            return;            
        }
        nintx = bsr->GetNintX();
        ninty = bsr->GetNintY();
        nintz = bsr->GetNintZ();
    } else if (nintx < 1 || ninty < 1 || nintz < 1) {
        json_err = std::string("LRFxyz: nintx/ninty/nintz is invalid or missing");
        return;
    } else {
        bsr = new Bspline3d(xmin, xmax, nintx, ymin, ymax, ninty, zmin, zmax, nintz);
    }

    valid = true;
}

LRFxyz::LRFxyz(std::string &json_str) : LRFxyz(Json::parse(json_str, json_err)) {}

LRFxyz::~LRFxyz()
{
    delete bsr;
}

bool LRFxyz::isReady() const
{
    return true; // bsr && bsr->IsReady();
}

bool LRFxyz::inDomain(double x, double y, double z) const
{
    return x>xmin && x<xmax && y>ymin && y<ymax && z>zmin && z<zmax;
}

// TODO: Check WTF is Rmax for
double LRFxyz::getRmax() const
{
    return rmax;
}

double LRFxyz::eval(double x, double y, double z) const
{
    return isReady() ? bsr->Eval(x, y, z) : 0.;
}

double LRFxyz::evalDrvX(double x, double y, double z) const
{
    return isReady() ? bsr->EvalDrvX(x, y, z) : 0.;
}

double LRFxyz::evalDrvY(double x, double y, double z) const
{
    return isReady() ? bsr->EvalDrvY(x, y, z) : 0.;
}

double LRFxyz::evalDrvZ(double x, double y, double z) const
{
    return isReady() ? bsr->EvalDrvZ(x, y, z) : 0.;
}

BSfit3D *LRFxyz::InitFit()
{
    if (!non_negative) {
        BSfit3D *f = new BSfit3D(bsr);
        f->SetMinWeight(min_weight);
        f->SetMissingFactor(missing_factor);
        return f;
    }

    ConstrainedFit3D *cf = new ConstrainedFit3D(bsr);
    if (non_negative) cf->ForceNonNegative();
    cf->SetMinWeight(min_weight);
    cf->SetMissingFactor(missing_factor);
    return cf;
}

void LRFxyz::SetMinWeight(double val)
{
    min_weight = val;
    if (bsfit3d)
        bsfit3d->SetMinWeight(val); 
}

void LRFxyz::SetMissingFactor(double val)
{
    missing_factor = val;
    if (bsfit3d)
        bsfit3d->SetMissingFactor(val);
}

bool LRFxyz::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vx, vy, vz, va;
    bool status;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y, d.z) && d.good) )
            continue;
        vx.push_back(d.x);
        vy.push_back(d.y);
        vz.push_back(d.z);
        va.push_back(d.val);
    }

    BSfit3D *F = InitFit();
    if (binned) {
        F->AddData(vx, vy, vz, va);
        status = F->BinnedFit();
    } else {
        status = F->Fit(vx, vy, vz, va);
    }

    if (status) {
        delete bsr;
        bsr = F->MakeSpline();
    } 

    delete F;
    valid = status;
    return status;
}

bool LRFxyz::addData(const std::vector <LRFdata> &data)
{
    if (!bsfit3d)
        bsfit3d = InitFit();

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y, d.z) && d.good) )
            continue;
        bsfit3d->AddData(d.x, d.y, d.z, d.val);
    }
    return true;
}

bool LRFxyz::doFit()
{
    if (!bsfit3d)
        return false;

    if (bsfit3d->BinnedFit()) {
        delete bsr;
        bsr = bsfit3d->MakeSpline();
        valid = true;
        return true;        
    } else {
        valid = false;
        return false;
    }
}

void LRFxyz::clearData()
{
    if (bsfit3d)
        bsfit3d->ClearData();
}

ProfileHist *LRFxyz::GetHist() 
{
    if (!bsfit3d)
        bsfit3d = InitFit();
    return bsfit3d->GetHist(); 
}

void LRFxyz::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["xmin"] = xmin;
    json["xmax"] = xmax;
    json["ymin"] = ymin;
    json["ymax"] = ymax;
    json["zmin"] = zmin;
    json["zmax"] = zmax;

    std::vector <std::string> cstr;
    if (non_negative) cstr.push_back("non-negative");
    if (cstr.size() > 0)
        json["constraints"] = cstr;

    if (bsr) 
        json["response"] = bsr->GetJsonObject();
}

double LRFxyz::GetRatio(LRF* other_base) const
{
    LRFxyz *other = dynamic_cast<LRFxyz*>(other_base);
    if (!other || !(other->bsfit3d))
        return -1;
    
    ProfileHist *h0 = bsfit3d->GetHist();
    ProfileHist *h1 = other->bsfit3d->GetHist();

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
