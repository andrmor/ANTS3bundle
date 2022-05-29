#ifndef AGRIDHUB_H
#define AGRIDHUB_H

#include <vector>

class AGeoObject;
class AGridElementRecord;

class AGridHub
{
public:
    static AGridHub & getInstance();
    static const AGridHub & getConstInstance();

private:
    AGridHub(){}
    ~AGridHub();

    AGridHub(const AGridHub&)            = delete;
    AGridHub(AGridHub&&)                 = delete;
    AGridHub& operator=(const AGridHub&) = delete;
    AGridHub& operator=(AGridHub&&)      = delete;

public:
    std::vector<AGridElementRecord*> GridRecords;

    void clear();

    AGridElementRecord * createGridRecord(AGeoObject * obj);
    void                 convertObjToGrid(AGeoObject * obj);
    void                 shapeGrid(AGeoObject * obj, int shape, double p0, double p1, double p2, int wireMat);
    //parallel - 0, pitch, length, wireDiameter
    //mesh - 1, pitchX, pitchY, wireDiameter
    //hexa - 2, outer circle diameter, inner circle diameter, full height

};

#endif // AGRIDHUB_H
