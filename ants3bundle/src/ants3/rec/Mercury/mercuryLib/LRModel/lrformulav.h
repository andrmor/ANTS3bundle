#ifndef LRFORMULAV_H
#define LRFORMULAV_H

#include "lrf.h"
#include "profileHist.h"
#include <cmath>

#include <Eigen/Core>
#include <cstddef>
#include <string>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <vector>
#include <map>

#include "wformula.h"

class LRFormulaV : public LRF
{
public:
 LRFormulaV(double x0, double y0, double rmax);
 LRFormulaV(const Json &json);
 LRFormulaV(std::string &json_str);    
//    LRFormula1(const LRFormula1 &obj); // copy constructor
    LRFormulaV();

    virtual LRFormulaV* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isReady () const;
    virtual double getRmax() const { return rmax; }
    virtual double eval(double x, double y, double z=0.) const;
    double evalAxial(double r) const;
//    double evalDrv(double r) const;
    virtual double evalDrvX(double x, double y, double z=0.) const {return 0.;}
    virtual double evalDrvY(double x, double y, double z=0.) const {return 0.;}

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData() { if (h1) h1->Clear(); }

    virtual std::string type() const { return std::string("FormulaV"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetOrigin(double x, double y) {x0 = x; y0 = y; Init();}
    void SetRmin(double r) { rmin = std::max(r, 0.); Init();}
    void SetRmax(double r) { rmax = r; Init();}

//  VFormula-related calls
    // setters
    void SetParameter(std::string name, double val);
    void SetExpression(std::string expr) {expression = expr;}
    // call this to update the virtual machine if parameter names of expression were changed
    std::string InitVF();  

    // getters
    double GetParameter(std::string name);
    std::vector<double> GetParVector() const {return parvals;}

// VM-debugging
    std::vector<std::string> GetPrg() {return vf->GetPrg();}
    std::vector<std::string> GetConstMap() {return vf->GetConstMap();}
    std::vector<std::string> GetVarMap() {return vf->GetVarMap();}
    std::vector<std::string> GetOperMap() {return vf->GetOperMap();}
    std::vector<std::string> GetFuncMap() {return vf->GetFuncMap();} 
    std::string GetVersion() {return std::string("1.0");}   

// calculation of radius + provision for compression
    double R(double x, double y) const {return sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));}
    double R2(double x, double y) const {return (x-x0)*(x-x0)+(y-y0)*(y-y0);}

// public getters
    double GetRmin() const {return rmin;}
    double GetOriginX() const {return x0;}
    double GetOriginY() const {return y0;}
    ProfileHist *GetHist() {return h1;}
    

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
    bool init_done = false;

// prof. histogram used for binned fitting
    ProfileHist1D *h1 = nullptr; 
    int nbins = 20;

// VFormula
    WFormula *vf = nullptr;
    std::string expression = std::string("");   // expression to parse
// vectors because parameters passed in a vector to functors
    std::vector<std::string> parnames;             // parameter names
    std::vector<double> parvals;                   // parameter values
    std::map<std::string, size_t> parmap;          // map for bookkeeping

// Minimizer parameters
    int maxfev = 200;   // max iterations
    double ftol = 1e-5;
    double xtol = 1e-5;

public: 
    void SetMaxFEV(int val) {maxfev = val;}
    void SetFtol(int val) {ftol = val;}
    void SetXtol(int val) {xtol = val;}

// safeguards
    double rzerod = 1e-6; // derivatives are assumed to be zero below this radius 
};

#endif // LRFORMULAV_H
