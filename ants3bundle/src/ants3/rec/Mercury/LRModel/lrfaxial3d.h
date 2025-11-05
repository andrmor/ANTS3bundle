#ifndef LRFAXIAL3D_H
#define LRFAXIAL3D_H

#include "lrfaxial.h"

class Bspline2d;
class BSfit2D;

class LRFaxial3d : public LRFaxial
{
public:
    LRFaxial3d(double rmax, int nint, double zmin, double zmax, int nintz);
    LRFaxial3d(const Json &json);
    LRFaxial3d(std::string &json_str);    
    ~LRFaxial3d();

    virtual LRFaxial3d* clone() const;

    virtual bool inDomain(double x, double y, double z) const;
    virtual bool isReady () const;

    int getNintZ() const {return nintz;}
    virtual double eval(double x, double y, double z) const;
    virtual double evalraw(double x, double y, double z) const;
    virtual double evalDrvX(double x, double y, double z) const;
    virtual double evalDrvY(double x, double y, double z) const;

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData();

    const Bspline2d *getSpline() const;
    virtual std::string type() const { return std::string("Axial3D"); }
    virtual void ToJsonObject(Json_object &json) const;

    void SetRmin(double rmin);
    void SetRmax(double rmax);
    void SetCompression(Compress1d *compress);
    void SetCompZ(Compress1d *compress);
    double RhoZ(double z) const; // compression transform of Z

    void SetForcedZSlope(int val) {z_slope = val;}
    ProfileHist *GetHist();

// relative gain calculation
    double GetRatio(LRF* other) const;    

protected:
//   void Init();
    BSfit2D *InitFit();    

private:
    int nintz;	// intervals
    int z_slope = 0;
    Bspline2d *bs2r = nullptr; 	// 2D spline
    BSfit2D *bs2fit = nullptr;     // object used in fitting
    Compress1d *compress_z = nullptr; // optional Z compression
};

#endif // LRFAXIAL3D_H
