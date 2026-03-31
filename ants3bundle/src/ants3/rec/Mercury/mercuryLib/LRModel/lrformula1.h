#ifndef LRFORMULA1_H
#define LRFORMULA1_H

#include "lrf.h"
#include "profileHist.h"
#include <cmath>

#include <Eigen/Core>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <vector>

enum Func {
    Gauss = 0,
    Sech,
    Sech2,
    Cauchy
}; 

class LRFormula1 : public LRF
{
public:
 LRFormula1(double x0, double y0, double rmax);
 LRFormula1(const Json &json);
 LRFormula1(std::string &json_str);    
//    LRFormula1(const LRFormula1 &obj); // copy constructor
    LRFormula1();

    virtual LRFormula1* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isValid () const {return valid;}
    virtual bool isReady () const;
    virtual double getRmax() const { return rmax; }
    virtual double eval(double x, double y, double z=0.) const;
    double evalAxial(double r) const;
    double evalDrv(double r) const;
    virtual double evalDrvX(double x, double y, double z=0.) const;
    virtual double evalDrvY(double x, double y, double z=0.) const;

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData() { if (h1) h1->Clear(); }

    virtual std::string type() const { return std::string("Formula1"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetOrigin(double x, double y) {x0 = x; y0 = y; Init();}
    void SetRmin(double r) { rmin = std::max(r, 0.); Init();}
    void SetRmax(double r) { rmax = r; Init();}
    void SetParameters(double a_, double c_, double s_) {a = a_; c = c_; s = s_;}

// calculation of radius + provision for compression
    double R(double x, double y) const {return sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));}
    double R2(double x, double y) const {return (x-x0)*(x-x0)+(y-y0)*(y-y0);}

// public getters
    double GetRmin() const {return rmin;}
    double GetOriginX() const {return x0;}
    double GetOriginY() const {return y0;}
    ProfileHist *GetHist() {return h1;}
    std::vector<double> GetParameters() const {return {a, c, s};}

// relative gain calculation
    double GetRatio(LRF* other) const; 

protected:
    void Init();

protected:
    double x0 = 0., y0 = 0.;  // center
    double rmin = 0.;    // domain
    double rmax = 0.;	// domain
    double rmin2;   // domain
    double rmax2;	// domain

// prof. histogram used for binned fitting
    ProfileHist1D *h1 = nullptr; 
    int nbins = 20;

// formula type and parameters
    Func ftype = Gauss;
    double c = 0.;   // additive constant
    double s = 1.;   // width
    double a = 0.;   // amplitude
    // as this is an axial response, mean is always 0

// safeguards
    double rzerod = 1e-6; // derivatives are assumed to be zero below this radius 
    bool valid = false;
    bool ready = false;
};

#endif // LRFORMULA1_H
