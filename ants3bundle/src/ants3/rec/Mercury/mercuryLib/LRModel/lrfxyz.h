#ifndef LRFXYZ_H
#define LRFXYZ_H

#include "lrf.h"
#include <cmath>

class Bspline3d;
class BSfit3D;

class LRFxyz : public LRF
{
public:
    LRFxyz(double xmin, double xmax, int nintx, double ymin, double ymax, int ninty,
           double zmin, double zmax, int nintz);
    LRFxyz(const Json &json);
    LRFxyz(std::string &json_str);
    ~LRFxyz();

    virtual LRFxyz* clone() const;

    virtual bool inDomain(double x, double y, double z) const;
    virtual bool isReady () const;
    virtual double getRmax() const;
    int getNintX() const {return nintx;}
    int getNintY() const {return ninty;}
    int getNintZ() const {return nintz;}
    virtual double eval(double x, double y, double z) const;
    virtual double evalDrvX(double x, double y, double z) const;
    virtual double evalDrvY(double x, double y, double z) const;
    virtual double evalDrvZ(double x, double y, double z) const;

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData();

    void SetMinWeight(double val);
    void SetMissingFactor(double val);

//    const Bspline3d *getSpline() const;
    virtual std::string type() const { return std::string("XYZ"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetNonNegative(bool val) {non_negative = val;}

    ProfileHist *GetHist();

// relative gain calculation
    double GetRatio(LRF* other) const; 

protected:
    void Init();
    BSfit3D *InitFit();

protected:
// rectangular domain
    int nintx, ninty, nintz;	// intervals
    Bspline3d *bsr = 0; 	// 3D spline
    BSfit3D *bsfit3d = 0;
    bool non_negative = false;
    double rmax;

// how to treat low stats
    double min_weight = 1;  // minimal accepted bin count
    double missing_factor = 1; // multiplication factor for 2nd derivative
};

#endif // LRFXYZ_H
