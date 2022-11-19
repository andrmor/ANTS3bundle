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
    void   setSeed(int seed);

    double uniform();
    double exp(double tau);
    double gauss(double mean, double sigma);
    double poisson(double mean);
    double gamma(double a, double b);

private:
    TRandom2 * RandGen = nullptr;

    double getPosUniRand() const;
};

#endif // ARANDOMHUB_H
