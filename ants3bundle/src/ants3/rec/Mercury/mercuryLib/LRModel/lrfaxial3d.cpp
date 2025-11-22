#include "lrfaxial3d.h"
#include "bspline123d.h"
#include "bsfit123.h"
#include "compress.h"
#include "json11.hpp"
#include "profileHist.h"

#include <iostream>

// ToDo: this gets pretty ugly, think about inheriting directly from LRF

LRFaxial3d::LRFaxial3d(double rmax, int nint, double zmin, double zmax,
            int nintz) : LRFaxial(rmax, nint)
{
//    this->rmax = rmax;
//    this->nint = nint;
    this->zmin = zmin;
    this->zmax = zmax;
    this->nintz = nintz;
    bs2r = new Bspline2d(rmin, rmax, nint, zmin, zmax, nintz);
//    Init();
}

LRFaxial3d* LRFaxial3d::clone() const 
{ 
    LRFaxial3d *copy = new LRFaxial3d(*this);
    copy->bs2r = bs2r ? new Bspline2d(*bs2r) : nullptr;
    copy->compress = compress ? compress->clone() : nullptr;
    copy->compress_z = compress_z ? compress_z->clone() : nullptr;
    copy->bs2fit = bs2fit ? bs2fit->clone() : nullptr;
// an example of an ugly part: need keep track of unused stuff 
    copy->bsr = nullptr;
    copy->bsfit = nullptr;
    return copy;
}

void LRFaxial3d::SetRmin(double val)
{
    rmin = val;
    rmin2 = rmin*rmin;
    delete bs2r;
    bs2r = new Bspline2d(Rho(rmin), Rho(rmax), nint, RhoZ(zmin), RhoZ(zmax), nintz);

    Init(); // Andr: to update xmin xmax etc
}

void LRFaxial3d::SetRmax(double val)
{
    rmax = val;
    rmax2 = rmax*rmax;
    delete bs2r;
    bs2r = new Bspline2d(Rho(rmin), Rho(rmax), nint, RhoZ(zmin), RhoZ(zmax), nintz);

    Init(); // Andr: to update xmin xmax etc
}

void LRFaxial3d::SetCompression(Compress1d *compress)
{
    this->compress = compress;
    delete bs2r;
    bs2r = new Bspline2d(Rho(rmin), Rho(rmax), nint, RhoZ(zmin), RhoZ(zmax), nintz);
}

double LRFaxial3d::RhoZ(double z) const
{
    return ( compress_z ? compress_z->Rho(z) : z );
}

void LRFaxial3d::SetCompZ(Compress1d *compress)
{
    this->compress_z = compress;
    delete bs2r;
    bs2r = new Bspline2d(Rho(rmin), Rho(rmax), nint, RhoZ(zmin), RhoZ(zmax), nintz);    
}

LRFaxial3d::LRFaxial3d(const Json &json) : LRFaxial(json)
{
    if (!json["zmin"].is_number() || !json["zmax"].is_number())
        return;  
    zmin = json["zmin"].number_value();
    zmax = json["zmax"].number_value();
    nintz = json["nintz"].number_value();
    if (json["compress_z"].is_object())
        compress_z = Compress1d::Factory(json["compress_z"]);
      
    if (json["response"]["tpspline3"].is_object()) {
        bs2r = new Bspline2d(json["response"]["tpspline3"]);
        if (bs2r->isInvalid()) {
            json_err = std::string("LRFaxial3D: invalid response");
            return;
        }
        nint = bs2r->GetNintX();
        nintz = bs2r->GetNintY();
    } else if (nint < 1 || nintz < 1) {
        json_err = std::string("LRFaxial3D: nint or nintz is invalid or missing");
        return;
    } else {
        std::cout << zmin << " , " << zmax << std::endl;
        std::cout << RhoZ(zmin) << " , " << RhoZ(zmax) << std::endl;
        bs2r = new Bspline2d(Rho(rmin), Rho(rmax), nint, RhoZ(zmin), RhoZ(zmax), nintz);
    }

    valid = true;
}

LRFaxial3d::~LRFaxial3d()
{
    delete bs2r;
    delete bs2fit;
}

bool LRFaxial3d::isReady() const
{
    return true; // (bs2r != 0) && bs2r->IsReady();
}

bool LRFaxial3d::inDomain(double x, double y, double z) const
{
    return LRFaxial::inDomain(x, y) && z>zmin && z<zmax;
}

double LRFaxial3d::eval(double x, double y, double z) const
{
    return isReady() ? bs2r->Eval(Rho(x, y), RhoZ(z)) : 0.;
}

// ToDo: this function doesn't make sense here 
double LRFaxial3d::evalraw(double x, double y, double z) const
{
    return isReady() ? bs2r->Eval(x, y) : 0.;
}

double LRFaxial3d::evalDrvX(double x, double y, double z) const
{
    return isReady() ? bs2r->EvalDrvX(Rho(x, y), RhoZ(z))*RhoDrvX(x, y) : 0.;
}

double LRFaxial3d::evalDrvY(double x, double y, double z) const
{
    return isReady() ? bs2r->EvalDrvX(Rho(x, y), RhoZ(z))*RhoDrvY(x, y) : 0.;
}

BSfit2D *LRFaxial3d::InitFit()
{
    if (!flattop && !non_negative && z_slope == 0)
        return new BSfit2D(bs2r);

    ConstrainedFit2D *cf = new ConstrainedFit2D(bs2r);
    if (non_negative) cf->ForceNonNegative();
    if (flattop) cf->ForceFlatTopX();
    if (z_slope != 0) cf->SetSlopeY(z_slope);
    return cf;
}

bool LRFaxial3d::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vr, vz, va;
    bool status;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y, d.z) && d.good) )
            continue;
        vr.push_back(Rho(d.x, d.y));
        vz.push_back(RhoZ(d.z));
        va.push_back(d.val);
    }

    BSfit2D *F = InitFit();
    if (binned) {
        F->AddData(vr, vz, va);
        status = F->BinnedFit();
    } else {
        status = F->Fit(vr, vz, va);
    }

    if (status) {
        delete bs2r;
        bs2r = F->MakeSpline();
    } 

    delete F;
    valid = status;
    return status;
}

bool LRFaxial3d::addData(const std::vector <LRFdata> &data)
{
    if (!bs2fit)
        bs2fit = InitFit();

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y, d.z) && d.good) )
            continue;
        bs2fit->AddData(Rho(d.x, d.y), RhoZ(d.z), d.val);
    }
    return true;
}

bool LRFaxial3d::doFit()
{
    if (!bs2fit)
        return false;

    if (bs2fit->BinnedFit()) {
        delete bs2r;
        bs2r = bs2fit->MakeSpline();
        valid = true;
        return true;        
    } else {
        valid = false;
        return false;
    }
}

void LRFaxial3d::clearData()
{
    if (bs2fit)
        bs2fit->ClearData();
}

// TODO: make it safer (convert into reference?)
const Bspline2d *LRFaxial3d::getSpline() const
{
    return bs2r;
}

ProfileHist *LRFaxial3d::GetHist() 
{
    if (!bs2fit)
        bs2fit = InitFit();
    return bs2fit->GetHist(); 
}

double LRFaxial3d::GetRatio(LRF* other_base) const
{
    LRFaxial3d *other = dynamic_cast<LRFaxial3d*>(other_base);
    if (!other || !(other->bs2fit))
        return -1;
    
    ProfileHist *h0 = bs2fit->GetHist();
    ProfileHist *h1 = other->bs2fit->GetHist();

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

void LRFaxial3d::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["rmin"] = rmin;
    json["rmax"] = rmax;
    json["x0"] = x0;
    json["y0"] = y0;
    json["zmin"] = zmin;
    json["zmax"] = zmax;

    std::vector <std::string> cstr;
    if (non_negative) cstr.push_back("non-negative");
    if (flattop) cstr.push_back("flattop");
    if (cstr.size() > 0)
        json["constraints"] = cstr;

    if (bs2r) {
        Json_object json1;
        json1["tpspline3"] = bs2r->GetJsonObject();
        json["response"] = json1;
    }

    if (compress) 
        json["compression"] = compress->GetJsonObject();
    if (compress_z) 
        json["compress_z"] = compress_z->GetJsonObject();
}