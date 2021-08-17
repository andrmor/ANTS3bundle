#ifndef AVECTOR_H
#define AVECTOR_H

class AVector2
{
public:
    AVector2(const double * pos) {for (int i=0; i<2; i++) r[i] = pos[i];}
    AVector2() {}

    double & operator[](int index) {return r[index];}

    double r[3];
};

class AVector3
{
public:
    AVector3(const double * pos) {for (int i=0; i<3; i++) r[i] = pos[i];}
    AVector3(double x, double y, double z) {r[0] = x; r[1] = y; r[2] = z;}
    AVector3() {}

    double * data() {return r;}
    const double * data() const {return r;}

    double & operator[](int index) {return r[index];}
    const double & operator[](int index) const {return r[index];}

    double r[3];
};

#endif // AVECTOR_H
