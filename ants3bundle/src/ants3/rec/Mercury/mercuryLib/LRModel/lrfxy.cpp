#include "lrfxy.h"
#include "bspline123d.h"
#include "bsfit123.h"
#include "json11.hpp"
#include "profileHist.h"

LRFxy::LRFxy(double xmin, double xmax, int nintx, double ymin, double ymax, int ninty): 
    nintx(nintx), ninty(ninty)
{
    this->xmin = xmin; this->xmax = xmax;
    this->ymin = ymin; this->ymax = ymax;
    bsr = new Bspline2d(xmin, xmax, nintx, ymin, ymax, ninty);
    Init();
}

LRFxy* LRFxy::clone() const 
{ 
    LRFxy *copy = new LRFxy(*this);
    copy->bsr = bsr ? new Bspline2d(*bsr) : nullptr;
    copy->bsfit2d = bsfit2d ? bsfit2d->clone() : nullptr;
    return copy;
}

void LRFxy::Init()
{
    double rmax2 = std::max({xmax*xmax+ymax*ymax, xmin*xmin+ymax*ymax, 
                             xmin*xmin+ymin*ymin, xmax*xmax+ymin*ymin});
    rmax = sqrt(rmax2);
}

LRFxy::LRFxy(const Json &json)
{
    if (!json["xmin"].is_number() || !json["xmax"].is_number() ||
        !json["ymin"].is_number() || !json["ymax"].is_number())
        return;
    xmin = json["xmin"].number_value();
    xmax = json["xmax"].number_value();
    ymin = json["ymin"].number_value();
    ymax = json["ymax"].number_value();
    nintx = json["nintx"].number_value();
    ninty = json["ninty"].number_value();
    if (xmax <= xmin || ymax<=ymin)
        return;
    
    if (json["constraints"].is_array()) {
        Json::array cstr = json["constraints"].array_items();
        for (size_t i=0; i<cstr.size(); i++) {
            std::string name(cstr[i].string_value());
            if (name == "non-negative") non_negative = true;
        }
    }

// read response
    if (json["response"].is_object()) {
        bsr = new Bspline2d(json["response"]);
        if (bsr->isInvalid()) {
            json_err = std::string("LRFxy: invalid response");
            return;            
        }
        nintx = bsr->GetNintX();
        ninty = bsr->GetNintY();
    } else if (nintx < 1 || ninty < 1) {
        json_err = std::string("LRFxy: nintx or ninty is invalid or missing");
        return;
    } else {
        bsr = new Bspline2d(xmin, xmax, nintx, ymin, ymax, ninty);
    }

    valid = true;
}

LRFxy::LRFxy(std::string &json_str) : LRFxy(Json::parse(json_str, json_err)) {}

LRFxy::~LRFxy()
{
    delete bsr;
}

bool LRFxy::isReady() const
{
    return true; // bsr && bsr->IsReady();
}

bool LRFxy::inDomain(double x, double y, double /*z*/) const
{
    return x>xmin && x<xmax && y>ymin && y<ymax;
}

// TODO: Check WTF is Rmax for
double LRFxy::getRmax() const
{
    return rmax;
}

double LRFxy::eval(double x, double y, double /*z*/) const
{
    return isReady() ? bsr->Eval(x, y) : 0.;
}

double LRFxy::evalDrvX(double x, double y, double /*z*/) const
{
    return isReady() ? bsr->EvalDrvX(x, y) : 0.;
}

double LRFxy::evalDrvY(double x, double y, double /*z*/) const
{
    return isReady() ? bsr->EvalDrvY(x, y) : 0.;
}

BSfit2D *LRFxy::InitFit()
{
    if (!non_negative && !top_down)
        return new BSfit2D(bsr);

    ConstrainedFit2D *cf = new ConstrainedFit2D(bsr);
    if (non_negative) cf->ForceNonNegative();
    if (top_down) cf->ForceTopDown(x0, y0);
    return cf;
}

bool LRFxy::fitData(const std::vector <LRFdata> &data)
{
    std::vector <double> vx, vy, va;
    bool status;

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) &&  d.good) )
            continue;
        vx.push_back(d.x);
        vy.push_back(d.y);
        va.push_back(d.val);
    }

    BSfit2D *F = InitFit();
    if (binned) {
        F->AddData(vx, vy, va);
        status = F->BinnedFit();
    } else {
        status = F->Fit(vx, vy, va);
    }

    if (status) {
        delete bsr;
        bsr = F->MakeSpline();
    } 

    delete F;
    valid = status;
    return status;
}

bool LRFxy::addData(const std::vector <LRFdata> &data)
{
    if (!bsfit2d)
        bsfit2d = InitFit();

    for (auto d : data) {
        if ( !(inDomain(d.x, d.y) &&  d.good) )
            continue;
        bsfit2d->AddData(d.x, d.y, d.val);
    }
    return true;
}

bool LRFxy::doFit()
{
    if (!bsfit2d)
        return false;

    if (bsfit2d->BinnedFit()) {
        delete bsr;
        bsr = bsfit2d->MakeSpline();
        valid = true;
        return true;        
    } else {
        valid = false;
        return false;
    }
}

void LRFxy::clearData()
{
    if (bsfit2d)
        bsfit2d->ClearData();
}

ProfileHist *LRFxy::GetHist() 
{
    if (!bsfit2d)
        bsfit2d = InitFit();
    return bsfit2d->GetHist(); 
}

void LRFxy::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());
    json["xmin"] = xmin;
    json["xmax"] = xmax;
    json["ymin"] = ymin;
    json["ymax"] = ymax;

    std::vector <std::string> cstr;
    if (non_negative) cstr.push_back("non-negative");
    if (cstr.size() > 0)
        json["constraints"] = cstr;

    if (bsr) 
        json["response"] = bsr->GetJsonObject();

}

double LRFxy::GetRatio(LRF* other_base) const
{
    LRFxy *other = dynamic_cast<LRFxy*>(other_base);
    if (!other || !(other->bsfit2d))
        return -1;
    
    ProfileHist *h0 = bsfit2d->GetHist();
    ProfileHist *h1 = other->bsfit2d->GetHist();

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
