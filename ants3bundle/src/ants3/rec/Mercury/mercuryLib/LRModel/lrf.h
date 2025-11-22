#ifndef LRF_H
#define LRF_H

#include "lrfio.h"
#include <array>
#include <vector>
#include <string>

typedef std::array <double, 3> Vec3data; // x, y, z
typedef std::array <double, 4> Vec4data; // x, y, z, value

class LRFdata
{
public:
    LRFdata(double X, double Y, double Z, double V) : x(X), y(Y), z(Z), val(V) {;}
    double x;
    double y;
    double z;
    double val;
    bool good = true;
};

class BSfit;
class ProfileHist;

class LRF : public LRF_IO
{
public:
    LRF() {}
    virtual ~LRF() {}

    virtual LRF* clone() const = 0;

    bool inDomain(double *pos) const {return inDomain(pos[0], pos[1], pos[2]);}
    double eval(double *pos) const {return eval(pos[0], pos[1], pos[2]);}
    double evalDrvX(double *pos) const {return evalDrvX(pos[0], pos[1], pos[2]);}
    double evalDrvY(double *pos) const {return evalDrvY(pos[0], pos[1], pos[2]);}

    void setBinned(bool val) {binned = val;}
    void setNonNegative(bool val) {non_negative = val;}

    virtual bool inDomain(double x, double y, double z=0.) const = 0;

    virtual double eval(double x, double y, double z=0.) const = 0;
    virtual double evalraw(double x, double y, double z=0.) const {return eval(x, y, z);}
    virtual double evalAxial(double r) const {return 0.;}
    virtual double evalDrvX(double x, double y, double z=0.) const = 0;
    virtual double evalDrvY(double x, double y, double z=0.) const = 0;

    virtual bool fitData(const std::vector <LRFdata> &data) = 0;
    virtual bool addData(const std::vector <LRFdata> &data) = 0;
    virtual bool doFit() = 0;
    virtual void clearData() = 0;

    virtual std::string type() const = 0;
    virtual bool isValid() const { return valid; }
//    virtual bool isReady () const;

    virtual double getRmax() const = 0;
    virtual double getXmin() const {return xmin;}
    virtual double getXmax() const {return xmax;}
    virtual double getYmin() const {return ymin;}
    virtual double getYmax() const {return ymax;}
    virtual double getZmin() const {return zmin;}
    virtual double getZmax() const {return zmax;}

    virtual double GetRatio(LRF* other) const = 0;

    virtual ProfileHist *GetHist() = 0;

// functions control treatment of low-stat data
// currently only implemented for XYZ
    virtual void SetMinWeight(double val) {;}
    virtual void SetMissingFactor(double val) {;}

// factory
    static LRF* mkFromJson(const Json &json);
    static LRF* mkFromJson(std::string &json_str);
    static LRF* mkFromJson(std::string &&json_str);

protected:
    bool valid = false; // indicates if the LRF can be used for reconstruction
    double xmin, xmax; 	// xrange
    double ymin, ymax; 	// yrange
    double zmin, zmax; 	// zrange
    bool binned = true;
    bool non_negative = false;
    std::string json_err;
public:       
    static std::string gjson_err;
};

#endif // LRF_H
