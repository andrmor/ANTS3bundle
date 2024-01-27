#ifndef AVECTOR_H
#define AVECTOR_H

class AVector2
{
public:
    AVector2(const double * pos);
    AVector2() {}

    double & operator[](int index) {return r[index];}

    double r[3];
};

class AVector3
{
public:
    AVector3(const double * pos);
    AVector3(double x, double y, double z);
    AVector3(const AVector3 &) = default;
    AVector3() {}

    double * data() {return r;}
    const double * data() const {return r;}

    double & operator[](int index) {return r[index];}
    const double & operator[](int index) const {return r[index];}

    AVector3 & operator *= (double factor);
    AVector3 & operator += (const AVector3 & vec);

    AVector3   operator + (const AVector3 & vec) const;
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

#endif // AVECTOR_H
