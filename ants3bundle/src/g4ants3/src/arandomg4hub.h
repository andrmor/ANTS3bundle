#ifndef ARANDOMHUB_H
#define ARANDOMHUB_H

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
    void   init(int seed);

    double uniform();
    double exp(double tau);
    double gauss(double mean, double sigma);
    double poisson(double mean);

};

#endif // ARANDOMHUB_H
