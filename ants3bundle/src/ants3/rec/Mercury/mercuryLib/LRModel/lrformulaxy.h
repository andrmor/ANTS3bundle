#ifndef LRFORMULAXY_H
#define LRFORMULAXY_H

#include "lrf.h"
#include "profileHist.h"
#include <cmath>

#include <Eigen/Core>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

#include "wformula.h"

class LRFormulaXY : public LRF
{
public:
    LRFormulaXY(double xmin, double xmax, double ymin, double ymax);
    LRFormulaXY(const Json &json);
    LRFormulaXY(std::string &json_str);
    ~LRFormulaXY();

    virtual LRFormulaXY* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isValid() const {return vf;}
    virtual bool isReady () const;
    virtual double getRmax() const;
    virtual double eval(double x, double y, double z=0.) const;
    virtual double evalDrvX(double x, double y, double z=0.) const {return 0.;}
    virtual double evalDrvY(double x, double y, double z=0.) const {return 0.;}

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    bool addDataPy(const std::vector <Vec4data> &data);
    virtual bool doFit();
    virtual void clearData() { if (h1) h1->Clear(); }

    virtual std::string type() const { return std::string("FormulaXY"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetOrigin(double x, double y) {x0 = x; y0 = y;}

//  VFormula-related calls
    // setters
    void SetConstant(std::string name, double val);
    void SetParameter(std::string name, double val);
    void SetExpression(std::string expr) {expression = expr;}
    // call this to update the virtual machine if parameter names or expression were changed
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
    std::string GetVersion() {return std::string("0.01");} 

    ProfileHist *GetHist() {return h1;}

// relative gain calculation
    double GetRatio(LRF* other) const; 

protected:
    void Init();

protected:
    double x0 = 0., y0 = 0.;  // a point of reference in the XY-plane, normally a center of a sensor
    double rmax;

// prof. histogram used for binned fitting
    ProfileHist2D *h1 = nullptr; 
    int nbinsx = 20;
    int nbinsy = 20;

// VFormula
    WFormula *vf = nullptr;
    std::string expression = std::string("");   // expression to parse
    // vectors because parameters passed in a vector to functors
    std::vector<std::string> parnames;             // parameter names
    std::vector<double> parvals;                   // parameter values

public:
// Minimizer parameters
    int maxfev = 200;   // max iterations
    double ftol = 1e-5;
    double xtol = 1e-5;
    double qtol = -1.;
// Minimizer output
    Eigen::VectorXd fvec;
//    Eigen::VectorXd fjac;
    Eigen::MatrixXd fjac;

 
    void SetMaxFEV(int val) {maxfev = val;}
    void SetFtol(int val) {ftol = val;}
    void SetXtol(int val) {xtol = val;}

// safeguards
    bool ready = false;
};

#endif // LRFORMULAXY_H
