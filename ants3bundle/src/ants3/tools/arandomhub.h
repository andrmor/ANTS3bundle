#ifndef ARANDOMHUB_H
#define ARANDOMHUB_H

class TRandom2;

class ARandomHub
{
public:
    static ARandomHub & getInstance();

private:
    ARandomHub();
    ~ARandomHub(){}

    ARandomHub(const ARandomHub&)            = delete;
    ARandomHub(ARandomHub&&)                 = delete;
    ARandomHub& operator=(const ARandomHub&) = delete;
    ARandomHub& operator=(ARandomHub&&)      = delete;

public:
    TRandom2 * RandGen = nullptr;

    double uniform();
    double exp(double tau);
};

#endif // ARANDOMHUB_H
