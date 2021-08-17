#ifndef APHOTONTRACKRECORD_H
#define APHOTONTRACKRECORD_H

#include "avector.h"
#include <vector>

class APhotonTrackRecord
{
public:
    bool HitSensor;
    bool SecondaryScint;
    std::vector<AVector3> Positions;
};

#endif // APHOTONTRACKRECORD_H
