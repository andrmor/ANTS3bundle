#ifndef AVECTOR_H
#define AVECTOR_H

class AVector2
{
public:
    AVector2(const double * pos);
    AVector2(double x, double y);
    AVector2() {}

    double & operator[](int index) {return r[index];}
    const double & operator[](int index) const {return r[index];}

    AVector2 operator + (const AVector2 & vec) const;
    AVector2 operator - (const AVector2 & vec) const;
    AVector2 operator * (double factor) const;

    bool operator == (const AVector2 & vec) const;

    double r[2];
};

class AVector3
{
public:
    AVector3(const double * pos);
    AVector3(double x, double y, double z);
    //AVector3(const AVector3 &) = default;
    AVector3() {}

    double * data() {return r;}
    const double * data() const {return r;}

    double & operator[](int index) {return r[index];}
    const double & operator[](int index) const {return r[index];}

    AVector3 & operator *= (double factor);
    AVector3 & operator += (const AVector3 & vec);

    AVector3   operator + (const AVector3 & vec) const;
    AVector3   operator - (const AVector3 & vec) const;
    AVector3   operator * (double factor) const;

    double mag2() const;
    double dot(const AVector3 & vec) const;

    void rotateX(double angle);
    void rotateY(double angle);
    void rotateZ(double angle);

    void rotateUz(const AVector3 & NewUzVector); // NewUzVector must be unitary vector

    AVector3 vectorProduct(const AVector3 & vector) const;

    void rotate(double angle, const AVector3 & aroundVector);

    double angle(const AVector3 & vec) const;

    AVector3 & toUnitVector();

    double r[3];
};

class ALine3D
{
public:
    ALine3D(const AVector3 & p1, const AVector3 & p2);

    bool getIntersect(const ALine3D & otherLine, AVector3 & result) const;

    AVector3 p;
    AVector3 d;
};

class ALine2D
{
public:
    ALine2D(const AVector2 & p1, const AVector2 & p2);

    bool getIntersect(const ALine2D & otherLine, AVector2 & result) const;

    AVector2 p; // offset
    AVector2 d; // slope
};

#endif // AVECTOR_H
