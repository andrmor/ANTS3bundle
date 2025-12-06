#ifndef LRFAXIAL_H
#define LRFAXIAL_H

#include "lrf.h"
#include <cmath>

class Bspline1d;
class BSfit1D;
class Compress1d;

class LRFaxial : public LRF
{
public:
    LRFaxial(double rmax, int nint);
    LRFaxial(const Json &json);
    LRFaxial(std::string &json_str);    
//    LRFaxial(const LRFaxial &obj); // copy constructor
    ~LRFaxial();

    virtual LRFaxial* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isReady () const;
    virtual double getRmax() const { return rmax; }
    int getNint() const { return nint; }
    std::vector <double> GetNodes() const;
    virtual double eval(double x, double y, double z=0.) const;
    virtual double evalraw(double x, double y, double z=0.) const;
    double evalAxial(double r) const;
    virtual double evalDrvX(double x, double y, double z=0.) const;
    virtual double evalDrvY(double x, double y, double z=0.) const;
//    double fitRData(int npts, const double *r, const double *data);

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData();

    const Bspline1d *getSpline() const;
    virtual std::string type() const { return std::string("Axial"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetOrigin(double x0, double y0);
    void SetRmin(double rmin);
    void SetRmax(double rmax);
    void SetCompression(Compress1d *compress);
// temporary hack to import old LRFs
    void SetSpline(Bspline1d *spline);

    void SetFlatTop(bool val) {flattop = val;}
    void SetNonIncreasing(bool val) {non_increasing = val;}

// calculation of radius + provision for compression
    double R(double x, double y) const {return sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));}
    double R2(double x, double y) const {return (x-x0)*(x-x0)+(y-y0)*(y-y0);}
    double Rho(double r) const;
    double Rho(double x, double y) const;
    double RhoDrvX(double x, double y) const;
    double RhoDrvY(double x, double y) const;

// public getters
    double GetRmin() const {return rmin;}
    double GetOriginX() const {return x0;}
    double GetOriginY() const {return y0;}
    ProfileHist *GetHist();

// relative gain calculation
    double GetRatio(LRF* other) const;    

protected:
    void Init();
    BSfit1D *InitFit();

protected:
    double x0 = 0., y0 = 0.;  // center
    double rmin = 0.;    // domain
    double rmax = 0.;	// domain
    double rmin2;   // domain
    double rmax2;	// domain
    int nint;		// intervals
    Bspline1d *bsr = nullptr; 	// spline describing radial dependence
    Compress1d *compress = nullptr; // optional compression

    BSfit1D *bsfit = nullptr;     // object used in fitting
// constraints    
    bool flattop = false;   // set to true if you want to have zero derivative at the origin
    bool non_increasing = false;

    bool init_done = false;
};

#endif // LRFAXIAL_H
