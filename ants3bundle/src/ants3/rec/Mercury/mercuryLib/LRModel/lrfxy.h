#ifndef LRFXY_H
#define LRFXY_H

#include "lrf.h"
#include <cmath>

class Bspline2d;
class BSfit2D;

class LRFxy : public LRF
{
public:
    LRFxy(double xmin, double xmax, int nintx, double ymin, double ymax, int ninty);
    LRFxy(const Json &json);
    LRFxy(std::string &json_str);
    ~LRFxy();

    virtual LRFxy* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isReady () const;
    virtual double getRmax() const;
    int getNintX() const {return nintx;}
    int getNintY() const {return ninty;}
    virtual double eval(double x, double y, double z=0.) const;
    virtual double evalDrvX(double x, double y, double z=0.) const;
    virtual double evalDrvY(double x, double y, double z=0.) const;

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData();

//    const Bspline2d *getSpline() const;
    virtual std::string type() const { return std::string("XY"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetNonNegative(bool val) {non_negative = val;}
    void SetTopDown(bool val, double x, double y) {top_down = val; x0 = x; y0 = y;}

    ProfileHist *GetHist();

// relative gain calculation
    double GetRatio(LRF* other) const; 

protected:
    void Init();
    BSfit2D *InitFit();

protected:
// rectangular domain
    int nintx, ninty;	// intervals
    Bspline2d *bsr = 0; 	// 2D spline
    BSfit2D *bsfit2d = 0;
    bool non_negative = false;
    double x0 = 0.; // coordinates of the maximum for top-down
    double y0 = 0.;
    bool top_down = false;
    double rmax;
};

#endif // LRFXY_H
