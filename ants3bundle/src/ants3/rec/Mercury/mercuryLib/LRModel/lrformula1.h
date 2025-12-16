#ifndef LRFORMULA_H
#define LRFORMULA_H

#include "lrf.h"
#include "profileHist.h"
#include <cmath>

#include <Eigen/Core>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <vector>

// Define a functor for LM optimization
// this particualar structure is needed by LM optimizer
// plus some additional stuff required by numerical differentiation module
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor {
    typedef _Scalar Scalar;
    enum {
        InputsAtCompileTime = NX,
        ValuesAtCompileTime = NY
    };
    typedef Eigen::Matrix<Scalar, NX, 1> InputType;
    typedef Eigen::Matrix<Scalar, NY, 1> ValueType;
    typedef Eigen::Matrix<Scalar, NY, NX> JacobianType;

    int m_inputs, m_values;
    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}
    int inputs() const { return m_inputs; }
    int values() const { return m_values; }
};

enum Func {
    Gauss = 0,
    Sech,
    Sech2,
    Cauchy
};  

// parameter naming scheme
// a - amplitude
// s - width
// c - additive constant
// mean is always 0

// Universal functor
struct UniFunctor : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Func type;

    UniFunctor(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Func type_)
        : Functor<double>(3, x_.size()), x(x_), y(y_), type(type_) {}

    // Compute residuals: f(p) = model(p) - y
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        double a = p[0];
        double s = p[1];
        double c = p[2];

        Eigen::ArrayXd dx(x), t;

        switch (type) {
            case Gauss:
                fvec = a*exp(-dx*dx/2/s/s) + c - y.array();
                break;
            case Sech:
                fvec = a/cosh(dx/s) + c - y.array();
                break;
            case Sech2:
                t = 1/cosh(dx/s);
                fvec = a*t*t + c - y.array();
                break;
            case Cauchy:
                fvec = a/(1 + dx*dx/s/s) + c - y.array();

        }

        return 0;
    }
};

// Universal functor with weights
struct UniFunctorW : Functor<double> {
    Eigen::VectorXd x;
    Eigen::VectorXd y;
    Eigen::VectorXd w;
    Func type;

    UniFunctorW(const Eigen::VectorXd& x_, const Eigen::VectorXd& y_, const Eigen::VectorXd& w_, const Func type_)
        : Functor<double>(3, x_.size()), x(x_), y(y_), w(w_), type(type_) {}

    // Compute residuals: f(p) = model(p) - y
    int operator()(const Eigen::VectorXd& p, Eigen::VectorXd& fvec) const {
        double a = p[0];
        double s = p[1];
        double c = p[2];

        Eigen::ArrayXd dx(x), t, f;
        dx /= s;

        switch (type) {
            case Gauss:
                f = exp(-dx*dx/2);
                break;
            case Sech:
                f = 1/cosh(dx);
                break;
            case Sech2:
                t = 1/cosh(dx);
                f = t*t;
                break;
            case Cauchy:
                f = 1/(1 + dx*dx);
        }
        fvec = (f*a + c - y.array()) * sqrt(w.array());

        return 0;
    }
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
    bool init_done = false;

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
};

#endif // LRFORMULA_H
