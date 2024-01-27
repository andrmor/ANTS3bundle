// implementation of rotations is taken from https://root.cern.ch/doc/master/TVector3_8cxx_source.html#l00263

#include "avector.h"

#include <cmath>

AVector2::AVector2(const double * pos)
{
    for (int i = 0; i < 2; i++) r[i] = pos[i];
}

AVector3::AVector3(const double * pos)
{
    for (int i = 0; i < 3; i++) r[i] = pos[i];
}

AVector3 & AVector3::operator *=(double factor)
{
    for (int i = 0; i < 3; i++) r[i] *= factor;
    return *this;
}

AVector3 & AVector3::operator +=(const AVector3 & vec)
{
    for (int i = 0; i < 3; i++) r[i] += vec[i];
    return *this;
}

AVector3 AVector3::operator +(const AVector3 & vec) const
{
    return AVector3(r[0]+vec[0], r[1]+vec[1], r[2]+vec[2]);
}

AVector3 AVector3::operator *(double factor) const
{
    return AVector3(r[0]*factor, r[1]*factor, r[2]*factor);
}

double AVector3::mag2() const
{
    double mag2 = 0;
    for (int i = 0; i < 3; i++) mag2 += r[i] * r[i];
    return mag2;
}

double AVector3::dot(const AVector3 & vec) const
{
    double dot = 0;
    for (int i = 0; i < 3; i++) dot += r[i] * vec[i];
    return dot;
}

AVector3::AVector3(double x, double y, double z)
{
    r[0] = x;
    r[1] = y;
    r[2] = z;
}

void AVector3::rotateX(double angle)
{
    const double s = sin(angle);
    const double c = cos(angle);

    const double yy = r[1];

    r[1] = c * yy - s * r[2];
    r[2] = s * yy + c * r[2];
}

void AVector3::rotateY(double angle)
{
    const double s = sin(angle);
    const double c = cos(angle);

    const double zz = r[2];

    r[2] = c * zz - s * r[0];
    r[0] = s * zz + c * r[0];
}

void AVector3::rotateZ(double angle)
{
    const double s = sin(angle);
    const double c = cos(angle);

    const double xx = r[0];

    r[0] = c * xx - s * r[1];
    r[1] = s * xx + c * r[1];
}

void AVector3::rotateUz(const AVector3 & NewUzVector)
{
    double u1 = NewUzVector.r[0];
    double u2 = NewUzVector.r[1];
    double u3 = NewUzVector.r[2];

    double up = u1 * u1 + u2 * u2;

    if (up)
    {
        up = sqrt(up);
        const double px = r[0];
        const double py = r[1];
        const double pz = r[2];
        r[0] = (u1 * u3 * px - u2 * py + u1 * up * pz) / up;
        r[1] = (u2 * u3 * px + u1 * py + u2 * up * pz) / up;
        r[2] = (u3 * u3 * px -      px + u3 * up * pz) / up;
    }
    else if (u3 < 0.0)
    {
        r[0] = -r[0]; r[2] = -r[2]; // phi=0  teta=pi
    }
}

AVector3 AVector3::vectorProduct(const AVector3 & vector) const
{
    return AVector3(r[1] * vector.r[2] - r[2] * vector.r[1],
                    r[2] * vector.r[0] - r[0] * vector.r[2],
                    r[0] * vector.r[1] - r[1] * vector.r[0]);

/*
y * v.z - z * v.y,
z * v.x - x * v.z,
x * v.y - y * v.x);
*/
}

void AVector3::rotate(double angle, const AVector3 & aroundVector)
{
    /*
G4double cos = std::cos(angle);
G4double sin = std::sin(angle);
   (*this) = (*this) * cos + axis.vector(*this) * sin + axis * (axis.dot(*this)*(1.-cos));
    */
    //(*this) = (*this) * cos
    //        += axis.vector(*this) * sin
    //        += axis * (axis.dot(*this)*(1.-cos));

    const double cosA = cos(angle);
    const double sinA = sin(angle);

    /*
    (*this) *= cosA;

    const AVector3 v1 = aroundVector.vectorProduct(*this) * sinA;
    (*this) += v1;

    (*this) += aroundVector * ( aroundVector.dot(*this) * (1.0 - cosA) );
    */
    (*this) = (*this)*cosA + aroundVector.vectorProduct(*this) * sinA + aroundVector * ( aroundVector.dot(*this) * (1.0 - cosA) );

}

double AVector3::angle(const AVector3 & vec) const
{
       double ptot2 = mag2() * vec.mag2();
       if (ptot2 <= 0) return 0.0;
       else
       {
          double arg = dot(vec) / sqrt(ptot2);
          if (arg >  1.0) arg =  1.0;
          if (arg < -1.0) arg = -1.0;
          return acos(arg);
       }
}

AVector3 & AVector3::toUnitVector()
{
    const double m2 = mag2();
    double factor = ( (m2 > 0) ? 1.0/sqrt(m2) : 1.0 );
    for (size_t i = 0; i < 3; i++) r[i] *= factor;

    return *this;
}
