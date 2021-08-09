#ifndef APHOTONTRACKRECORD_H
#define APHOTONTRACKRECORD_H

#include <vector>

class AVector3
{
public:
    AVector3(const double * pos) {for (int i=0; i<3; i++) r[i] = pos[i];}
    AVector3() {}

    double & operator[](int index) {return r[index];}

    double r[3];
};

class APhotonTrackRecord
{
public:
    bool HitSensor;
    bool SecondaryScint;
    std::vector<AVector3> Positions;
};

#endif // APHOTONTRACKRECORD_H
