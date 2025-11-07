#ifndef COMPRESS_H
#define COMPRESS_H

#include <cmath>
#include <vector>
#include "lrfio.h"

class Compress1d : public LRF_IO
{
public:
    Compress1d() {}
    virtual ~Compress1d() {}
    virtual Compress1d* clone() const = 0;
    virtual double Rho(double r) const = 0;
    virtual double RhoDrv(double r) const = 0;
    virtual void ToJsonObject(Json_object &json) const = 0;
    virtual double Compress(double r) const {return r;}

    static Compress1d* Factory(const Json &json);

protected:
    bool fValid = false;
};

class Compress2d : public LRF_IO
{
public:
    Compress2d() {}
    virtual ~Compress2d() {}
    virtual Compress2d* clone() const = 0;
    virtual double Rho(double r, double z) const = 0;
// in future, it'd be nice to have the derivatives for completeness
//    virtual double RhoDrvR(double r, double z) const = 0;
//    virtual double RhoDrvZ(double r, double z) const = 0;
    virtual void ToJsonObject(Json_object &json) const = 0;

    static Compress2d* Factory(const Json &json);

protected:
    bool fValid = false;
};

// Quadratic

class QuadraticCompress : public Compress1d
{
public:
    QuadraticCompress() {}
    QuadraticCompress* clone() const { return new QuadraticCompress(*this); }
    double Rho(double r) const {return r*r;}
    double RhoDrv(double r) const {return r*2.;}
    void ToJsonObject(Json_object &json) const;
};

// Dual slope

class DualSlopeCompress : public Compress1d
{
public:
    DualSlopeCompress(double k, double r0, double lam);
    DualSlopeCompress(const Json &json);
    virtual DualSlopeCompress* clone() const { return new DualSlopeCompress(*this); }
    void Init();
    virtual double Rho(double r) const;
    virtual double RhoDrv(double r) const;
    virtual void ToJsonObject(Json_object &json) const;

    double Compress(double r) const override;

private:
    double k;
    double r0;
    double lam;

    double a;
    double b;
    double lam2;
};

// Dual slope with Z-dependence of the parameters

class DualSlopeZCompress : public Compress2d
{
public:
    DualSlopeZCompress(std::vector <double> z, std::vector <double> k, std::vector <double> r0, std::vector <double> lam);
    DualSlopeZCompress(const Json &json);
    virtual DualSlopeZCompress* clone() const { return new DualSlopeZCompress(*this); }
    virtual double Rho(double r, double z) const;
//    virtual double RhoDrv(double r) const;
    virtual void ToJsonObject(Json_object &json) const;

private:
    double zmin, dz;
    double k, dk;
    double r0, dr0;
    double lam, dlam;
};

#endif // COMPRESS_H
