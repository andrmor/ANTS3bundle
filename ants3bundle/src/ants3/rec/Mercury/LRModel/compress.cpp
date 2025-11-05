#include "compress.h"
#include "json11.hpp"

#include <stdexcept>

Compress1d* Compress1d::Factory(const Json &json)
{
    if (json["method"].is_string() && json["method"].string_value() == "dualslope") {
        Compress1d *comp = new DualSlopeCompress(json);
        if (comp->fValid)
            return comp;
        else {
            delete comp;
            return nullptr;
        }
    } else if (json["method"].is_string() && json["method"].string_value() == "quadratic") {
        Compress1d *comp = new QuadraticCompress();
        return comp;
    } else {
        return nullptr;
    }
}

Compress2d* Compress2d::Factory(const Json &json)
{
    if (json["method"].is_string() && json["method"].string_value() == "dualslopez") {
        Compress2d *comp = new DualSlopeZCompress(json);
        if (comp->fValid)
            return comp;
        else {
            delete comp;
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

void DualSlopeCompress::Init()
{
    if (r0 < 0. || k <= 0.)
        return;

    a = (k+1)/(k-1);
    lam2 = std::max(lam*lam, 1e-6); // hack to avoid division by zero when lam=0
    b = sqrt(r0*r0+lam2)+a*r0;

    fValid = true;
}

DualSlopeCompress::DualSlopeCompress(double k, double r0, double lam) :
    k(k), r0(r0), lam(lam)
{
    Init();
}

DualSlopeCompress::DualSlopeCompress(const Json &json)
{
    if (!json["r0"].is_number() || !json["k"].is_number() || !json["lam"].is_number())
        return;

    r0 = json["r0"].number_value();
    k = json["k"].number_value();
    lam = json["lam"].number_value();

    Init();
}

double DualSlopeCompress::Rho(double r) const
{
    double dr = r - r0;
    double val = b + dr*a - sqrt(dr*dr + lam2);
    return std::max(0., k>=1 ? val : -val);
}

double DualSlopeCompress::RhoDrv(double r) const
{
    double dr = r - r0;
// Andrey has spotted that this can lead to division by zero if lam2=0
// Mitigated by adding a small positive lower limit to lam2 in Init() 
    return dr/sqrt(dr*dr + lam2) + a;
}

void  DualSlopeCompress::ToJsonObject(Json_object &json) const
{
    json["method"] = "dualslope";
    json["r0"] = r0;
    json["lam"] = lam;
    json["k"] = k;
}

void  QuadraticCompress::ToJsonObject(Json_object &json) const
{
    json["method"] = "quadratic";
}

// ============ DualSlopeZ ============

DualSlopeZCompress::DualSlopeZCompress(std::vector <double> z, std::vector <double> k, std::vector <double> r0, std::vector <double> lam)
{
    if (r0[0] < 0. || k[0] <= 1. || r0[1] < 0. || k[1] <= 1.)
        throw std::runtime_error(std::string("DualSlopeZCompress: bad parameter in constructor"));

    zmin = z[0]; dz = z[1] - z[0];
    this->k = k[0]; dk = k[1] - k[0];
    this->r0 = r0[0]; dr0 = r0[1] - r0[0];
    this->lam = lam[0]; dlam = lam[1] - lam[0];  

    fValid = true;
}

DualSlopeZCompress::DualSlopeZCompress(const Json &json)
{
    if (!json["r0"].is_number() || !json["r0max"].is_number() || 
        !json["k"].is_number() || !json["kmax"].is_number() ||
        !json["lam"].is_number() || !json["lammax"].is_number() ||
        !json["zmin"].is_number() || !json["zmax"].is_number())
        throw std::runtime_error(std::string("DualSlopeZCompress: JSON error in constructor"));

    zmin = json["zmin"].number_value();
    dz = json["zmax"].number_value() - zmin;
    r0 = json["r0"].number_value();
    dr0 = json["r0max"].number_value() - r0;
    k = json["k"].number_value();
    dk = json["kmax"].number_value() - k;
    lam = json["lam"].number_value();
    dlam = json["lammax"].number_value() - lam;

    if (r0 < 0. || k <= 1. || r0+dr0 < 0. || k+dk <= 1.)
        throw std::runtime_error(std::string("DualSlopeZCompress: bad JSON parameter in constructor"));

    fValid = true; 
}

double DualSlopeZCompress::Rho(double r, double z) const
{
// make linear interpolation of r0, k and lam 
    double p = (z-zmin)/dz; // interpolation factor
    double R0 = r0 + dr0*p;
    double K = k + dk*p;
    double Lam = lam + dlam*p;
// now R0, K and Lam store the interpolated values

    double a = (K+1)/(K-1);
    double lam2 = std::max(Lam*Lam, 1e-6); // hack to avoid division by zero when Lam=0
    double b = sqrt(R0*R0+lam2)+a*R0;

    double dr = r - R0;
    return std::max(0., b + dr*a - sqrt(dr*dr + lam2));
}

/*
double DualSlopeZCompress::RhoDrv(double r, double z) const
{
    double dr = r - r0;
// Andrey has spotted that this can lead to division by zero if lam2=0
// Mitigated by adding a small positive lower limit to lam2 in Init() 
    return dr/sqrt(dr*dr + lam2) + a;
}
*/

void  DualSlopeZCompress::ToJsonObject(Json_object &json) const
{
    json["method"] = "dualslopez";
    json["zmin"] = zmin;
    json["zmax"] = zmin + dz;
    json["r0"] = r0;
    json["r0max"] = r0 + dr0;
    json["lam"] = lam;
    json["lammax"] = lam + dlam;
    json["k"] = k;
    json["kmax"] = k + dk;
}
