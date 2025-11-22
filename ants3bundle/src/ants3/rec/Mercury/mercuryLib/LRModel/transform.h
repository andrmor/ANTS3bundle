#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cmath>
#include "lrfio.h"
#include <Eigen/Dense>

using Eigen::Matrix2d;
using Eigen::Vector2d;

class Transform : public LRF_IO
{
public:
    Transform() {}
    virtual ~Transform() {}
    virtual Transform* clone() const = 0;
    virtual void DoTransform(double *x, double *y, double *z) const = 0;
    virtual void DoInvTransform(double *x, double *y, double *z) const = 0;
    virtual void ToJsonObject(Json_object &json) const = 0;

    static Transform* Factory(const Json &json);

protected:
    bool fValid = false;
};

// translate

class TranslateLRF : public Transform
{
public:
    TranslateLRF(double dx, double dy);
    TranslateLRF(const Json &json);
    TranslateLRF* clone() const {return new TranslateLRF(*this);}
    virtual void DoTransform(double *x, double *y, double *z) const;
    virtual void DoInvTransform(double *x, double *y, double *z) const;
    virtual void ToJsonObject(Json_object &json) const;

private:
    double dx;
    double dy;
};

class RotateLRF : public Transform
{
public:
    RotateLRF(double phi);
    RotateLRF(const Json &json);
    RotateLRF* clone() const {return new RotateLRF(*this);}
    void Init();
    virtual void DoTransform(double *x, double *y, double *z) const;
    virtual void DoInvTransform(double *x, double *y, double *z) const;
    virtual void ToJsonObject(Json_object &json) const;
private:
    double phi;

    Matrix2d A; // forward matrix
    Matrix2d B; // inverse matrix

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

// NB: for reflections, forward and inverse transforms are the same
// so the transform matrix is its own inverse
class ReflectLRF : public Transform
{
public:
    ReflectLRF(double phi);
    ReflectLRF(const Json &json);
    ReflectLRF* clone() const {return new ReflectLRF(*this);}
    void Init();
    virtual void DoTransform(double *x, double *y, double *z) const;
    virtual void DoInvTransform(double *x, double *y, double *z) const;
    virtual void ToJsonObject(Json_object &json) const;

private:
    double phi;

    Matrix2d A; // forward and inverse matrix
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#endif // TRANSFORM_H


