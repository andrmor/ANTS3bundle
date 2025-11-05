#ifndef LRFCOMP_H
#define LRFCOMP_H

#include "lrf.h"

class ProfileHist;

// -------------------------------------
class LRFcomp : public LRF
{
public:
    LRFcomp();
    LRFcomp(const Json &json);
    LRFcomp(std::string &json_str);
    ~LRFcomp();

    virtual LRFcomp* clone() const;

    virtual bool inDomain(double x, double y, double z=0.) const;
    virtual bool isValid () const;
    virtual double getRmax() const;
    virtual double eval(double x, double y, double z=0.) const;
    virtual double evalDrvX(double x, double y, double z=0.) const;
    virtual double evalDrvY(double x, double y, double z=0.) const;

    virtual bool fitData(const std::vector <LRFdata> &data);
    virtual bool addData(const std::vector <LRFdata> &data);
    virtual bool doFit();
    virtual void clearData();

//    const Bspline2d *getSpline() const;
    virtual std::string type() const { return std::string("Composite"); }
    virtual void ToJsonObject(Json_object &json) const;

    ProfileHist *GetHist();

// relative gain calculation
    double GetRatio(LRF* other) const; 

// specific functions
    size_t GetCount() const {return stack.size();}
    bool CheckNotEmpty();
    bool SetActiveLayer(int id);
    double evalActive(double x, double y, double z=0.) const;
    LRF* GetLayer(int id) const;
    void AddLayer(LRF* lrf);

//protected:
//    void Init();
    
protected:
    std::vector <LRF*> stack;
    int active_layer = -1;
};

// -------------------------------------

#endif // LRFCOMP_H