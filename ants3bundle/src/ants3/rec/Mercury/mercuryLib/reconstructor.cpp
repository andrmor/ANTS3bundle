#include "reconstructor.h"
#include "lrmodel.h"
//#include "TROOT.h"
#include <iostream>
//#include <fstream>
// #include "eiquadprog.hpp"
#include <cmath>

#include <Eigen/Dense>
#include "TMath.h"
#include "lrmodel.h"

Reconstructor::Reconstructor(LRModel *lrm)
{
    this->lrm = lrm;
    nsensors = lrm->GetSensorCount();
    sensor.resize(nsensors);
    Active.reserve(nsensors);
    for (int i=0; i<nsensors; i++) {
        sensor[i].x = lrm->GetX(i);
        sensor[i].y = lrm->GetY(i);
        sensor[i].gain = 1.;
        sensor[i].on = !(lrm->IsDisabled(i));
    }
    A.resize(nsensors);
    sat.resize(nsensors);
}

Reconstructor::Reconstructor(std::string json_str) : Reconstructor(new LRModel(json_str))
{
    external_lrm = false;    
}

Reconstructor::~Reconstructor()
{
    if (!external_lrm) delete lrm;
}

bool Reconstructor::ProcessEvent(std::vector <double> &a)
{
    std::vector <bool> sat(a.size(), false);
    return ProcessEvent(a, sat);
}

bool Reconstructor::ProcessEvent(std::vector <double> &a, std::vector <bool> &sat, std::vector <double> &guess)
{
    std::cout << "Reconstructor::ProcessEvent() was called with guess vector supplied" << std::endl;
    return false;
}

// Check for static and dynamic passives + saturation
void Reconstructor::checkActive()
{
    bool act;
    Active.clear();
    int maxid = getMaxSignalID();
    double cutoff = std::max(rec_abs_cutoff, A[maxid]*rec_rel_cutoff);
    for (int i=0; i<nsensors; i++) {
// sensor is active if it's (enabled) AND (NOT saturated) AND (not below the cutoff)
// AND (within cutoff radius)
        if (sensor[i].on && !sat[i] && A[i] >= cutoff 
            && getDistFromSensor(i, guess_x, guess_y) <= rec_cutoff_radius)

            Active.push_back(i);
    }
}

int Reconstructor::getMaxSignalID()
{
    auto strongest = std::max_element(std::begin(A), std::end(A));
    return std::distance(std::begin(A), strongest);
}

void Reconstructor::guessByMax()
{
    int maxid = getMaxSignalID();
    guess_x = sensor[maxid].x;
    guess_y = sensor[maxid].y;
}

void Reconstructor::guessByCOG()
{
    int maxid = getMaxSignalID();
    double cutoff = std::max(cog_abs_cutoff, A[maxid]*cog_rel_cutoff);
    double sum_x, sum_y, sum_dn;
    sum_x = sum_y = sum_dn = 0.;
    for (int i=0; i<nsensors; i++)
        if (A[i] >= cutoff) {
            sum_x += sensor[i].x*A[i];
            sum_y += sensor[i].y*A[i];
            sum_dn += A[i];
    }
    guess_x = sum_x/sum_dn;
    guess_y = sum_y/sum_dn;
}

double Reconstructor::getSumSignal()
{
    double sum = 0.;
    for (int i=0; i<nsensors; i++)
        sum += A[i];
    return sum;
}

double Reconstructor::getSumLRF(double x, double y, double z)
{
    double sum = 0.;
    for (int i=0; i<nsensors; i++)
        sum += lrm->Eval(i, x, y, z);
    return sum;
}

double Reconstructor::getSumActiveSignal()
{
    double sum = 0.;
    for (int i : Active)
        sum += A[i];
    return sum;
}

double Reconstructor::getSumActiveLRF(double x, double y, double z)
{
    double sum = 0.;
    for (int i : Active)  
        sum += lrm->Eval(i, x, y, z);
    return sum;
}

std::string Reconstructor::getLRModelJson()
{
    return lrm->GetJsonString();
}

double Reconstructor::getDistFromSensor(int id, double x, double y)
{
  return hypot(x-sensor[id].x, y-sensor[id].y);
}

// ============= Cost functions =================

double Reconstructor::getChi2(double x, double y, double z, double energy, bool weighted)
{
    double sum = 0;
    double r[3];
    r[0] = x; r[1] = y; r[2] = z;
//    std::cout << "X:" << x << "    Y:" << y << "   E:" << energy << std::endl;

    for (int i : Active) {
        double LRFhere = lrm->Eval(i, r)*energy; // LRF(X, Y, Z) * energy;
//        std::cout << i << " " << LRFhere << std::endl;
        if (LRFhere <= 0.)
            return LastMiniValue *= 1.25; //if LRFs are not defined for this coordinates

        double delta = (LRFhere - A[i]);
        sum += weighted ? delta*delta/LRFhere : delta*delta;
    }
//    std::cout << "Sum: " << sum << std::endl;
    return LastMiniValue = sum;
}

double Reconstructor::getChi2autoE(double x, double y, double z, bool weighted)
{
//    std::cout << "X:" << x << "    Y:" << y << "   E:" << energy << std::endl;    
    double sum = 0;
    double sumsig = 0;
    double sumlrf = 0;
    size_t len = Active.size();

    std::vector <double> lr = lrm->EvalList(Active, x, y, z);
    for (size_t i=0; i<len; i++) {
        sumsig += A[Active[i]];
        sumlrf += lr[i];
    }
    double energy = sumsig/sumlrf;

    for (size_t j=0; j<len; j++) {
        double LRFhere = lr[j]*energy; // LRF(X, Y, Z) * energy;
        if (LRFhere <= 0.)
            return LastMiniValue *= 1.25; //if LRFs are not defined for this coordinates

        double delta = (LRFhere - A[Active[j]]);
        sum += weighted ? delta*delta/LRFhere : delta*delta;
    }

    return LastMiniValue = sum;
}

double Reconstructor::getLogLH(double x, double y, double z, double energy)
{
    double sum = 0;
    double r[3];
    r[0] = x; r[1] = y; r[2] = z;
//    std::cout << "X:" << x << "    Y:" << y << "   E:" << energy << std::endl;

    for (int i : Active) {
        double LRFhere = lrm->Eval(i, r)*energy; // LRF(X, Y, Z) * energy;
        if (LRFhere <= 0.)
            return LastMiniValue *= 1.25; //if LRFs are not defined for this coordinates

        sum -= A[i]*log(LRFhere) - LRFhere; // measures probability
    }

    return LastMiniValue = sum;
}

double Reconstructor::getLogLHautoE(double x, double y, double z)
{
//    std::cout << "X:" << x << "    Y:" << y << "   E:" << energy << std::endl;    
    double sum = 0;
    double sumsig = 0;
    double sumlrf = 0;
    size_t len = Active.size();

    std::vector <double> lr = lrm->EvalList(Active, x, y, z);
    for (size_t i=0; i<len; i++) {
        sumsig += A[Active[i]];
        sumlrf += lr[i];
    }
    double energy = sumsig/sumlrf;
    
    for (size_t j=0; j<len; j++) {
        double LRFhere = lr[j]*energy; // LRF(X, Y, Z) * energy;
        if (LRFhere <= 0.)
            return LastMiniValue *= 1.25; //if LRFs are not defined for this coordinates

        sum -= A[Active[j]]*log(LRFhere) - LRFhere; // measures probability
    }

    return LastMiniValue = sum;
}

// ========= Cost function wrappers ==========
/*
double CostLS::operator()(const double *p) // 0-x, 1-y, 2-energy
{
    double energy = rec->fAutoE ? rec->getSumActiveSignal()/rec->getSumActiveLRF(p[0], p[1], 0.) : p[2];
    return rec->getChi2(p[0], p[1], 0., energy, rec->fWeighted);
}

double CostML::operator()(const double *p) // 0-x, 1-y, 2-energy
{
    double energy = rec->fAutoE ? rec->getSumActiveSignal()/rec->getSumActiveLRF(p[0], p[1], 0.) : p[2];
    return rec->getLogLH(p[0], p[1], 0., energy);
}
*/
// =============== Minuit 2 ==================
RecMinuit::RecMinuit(LRModel *lrm) : Reconstructor(lrm)
{
    RootMinimizer = new ROOT::Minuit2::Minuit2Minimizer(ROOT::Minuit2::kMigrad);
    //RootMinimizer = new ROOT::Minuit2::Minuit2Minimizer(ROOT::Minuit2::kSimplex);
    RootMinimizer->SetMaxFunctionCalls(RMmaxFuncCalls);
    RootMinimizer->SetMaxIterations(RMmaxIterations);
    RootMinimizer->SetTolerance(RMtolerance);

// Set Minuit2 and ROOT verbosity level
    RootMinimizer->SetPrintLevel(MinuitPrintLevel);
    gErrorIgnoreLevel = RootPrintLevel;

//    std::cout << "w:" << fWeighted << ", e: " << fAutoE << std::endl;
}

RecMinuit::RecMinuit(std::string json_str) : RecMinuit(new LRModel(json_str))
{
    external_lrm = false;   
}

RecMinuit::~RecMinuit()
{
    delete RootMinimizer;
    if (init_done)
        delete FunctorLSML;
}

void RecMinuit::setMinuitVerbosity(int val) 
{
    MinuitPrintLevel = val; 
    if (RootMinimizer)
        RootMinimizer->SetPrintLevel(val);
}

bool RecMinuit::ProcessEvent(std::vector <double> &a, std::vector <bool> &sat)
{
    std::vector <double> t(0);
    return ProcessEvent(a, sat, t);
}

// initial guess options:
// size 0: x,y from CoG, z=0, energy auto
// size 3: coordinates supplied, energy auto
// size 4: all supplied
bool RecMinuit::ProcessEvent(std::vector <double> &a, std::vector <bool> &sat, std::vector <double> &guess)
{
    if (!init_done) {
        InitCostFunction();
        RootMinimizer->SetFunction(*FunctorLSML);
        init_done = true;
    }

    if (a.size() < nsensors || sat.size() < nsensors)
        return false;
    for (int i=0; i<nsensors; i++) {
        A[i] = a[i]/sensor[i].gain;
        this->sat[i] = sat[i];
    }

// initial guess
    if (guess.size() == 0) {
        guessByCOG();
        guess_z = fFixedZ ? fixedZ : 0.;
        guess_e = getSumSignal()/getSumLRF(guess_x, guess_y, guess_z);
    } else if (guess.size() == 3) {
        guess_x = guess[0];
        guess_y = guess[1];
        guess_z = fFixedZ ? fixedZ :guess[2];
        guess_e = getSumSignal()/getSumLRF(guess_x, guess_y, guess_z);
    } else if (guess.size() == 4) {
        guess_x = guess[0];
        guess_y = guess[1];
        guess_z = fFixedZ ? fixedZ :guess[2];
        guess_e = guess[3];        
    } else {
        rec_status = 7;
        return false;
    }

// set initial variables to minimize
    RootMinimizer->SetVariable(0, "x", guess_x, RMstepX);
    RootMinimizer->SetVariable(1, "y", guess_y, RMstepY);
    if (fFixedZ) 
        RootMinimizer->SetFixedVariable(2, "z", fixedZ);
    else
        RootMinimizer->SetVariable(2, "z", guess_z, RMstepZ);
    double step_e = RMstepEnergy > 0 ? RMstepEnergy : guess_e*0.2;
    if (!fAutoE)
        RootMinimizer->SetLowerLimitedVariable(3, "e", guess_e, step_e, 1.0e-6);
    int ndim = RootMinimizer->NDim();

// determine active sensors and see if there are enough for reconstruction

    checkActive();
    rec_dof = Active.size() - ndim;
    if (rec_dof < 1) {
        rec_status = 6;
        return false;
    }

    // do the minimization
    bool fOK = false;
    fOK = RootMinimizer->Minimize();

    if (fOK) {
        rec_status = 0 ;		// Reconstruction successfull

        const double *xs = RootMinimizer->X();
        rec_x = xs[0];
        rec_y = xs[1];
        rec_z = fFixedZ ? fixedZ : xs[2];
        rec_e = fAutoE ? getSumActiveSignal()/getSumActiveLRF(rec_x, rec_y, rec_z) : xs[3];
        rec_min = RootMinimizer->MinValue();

    // Calc Hessian matrix and get status
        double cov[ndim*ndim];
        RootMinimizer->Hesse();
        RootMinimizer->GetCovMatrix(cov);
        cov_xx = cov[0]; // first column first row
        cov_yy = cov[ndim+1]; // second column second row
        cov_xy = cov[1];      // second column first row
        return true;
    } else {
        rec_status = RootMinimizer->Status(); // reason why it has failed
        return false;
    }
}

// ============== Least Squares ===============
RecLS::RecLS(LRModel *lrm, bool weighted) : RecMinuit(lrm)
{
    fWeighted = weighted;
}

RecLS::RecLS(std::string json_str, bool weighted) : RecLS(new LRModel(json_str)) {}

void RecLS::InitCostFunction()
{
//    RecCostLS = new CostLS(this);
    FunctorLSML = new ROOT::Math::Functor(this, &RecLS::Cost, fAutoE ? 3 : 4);
    LastMiniValue = 1.e6; //reset for the new event
}

double RecLS::Cost(const double *p) // 0-x, 1-y, 2-energy
{
//    double energy = fAutoE ? getSumActiveSignal()/getSumActiveLRF(p[0], p[1], 0.) : p[2];
    return fAutoE ? getChi2autoE(p[0], p[1], p[2], fWeighted) : getChi2(p[0], p[1], p[2], p[3], fWeighted);
}

// ============== Maximum Likelihood ===============
RecML::RecML(LRModel *lrm) : RecMinuit(lrm) {}

RecML::RecML(std::string json_str) : RecML(new LRModel(json_str)) {}

void RecML::InitCostFunction()
{
//    RecCostML = new CostML(this);
    FunctorLSML = new ROOT::Math::Functor(this, &RecML::Cost, fAutoE ? 3 : 4);
    LastMiniValue = 1.e100; // reset for the new event
}

double RecML::Cost(const double *p) // 0-x, 1-y, 2-energy
{
//    double energy = fAutoE ? getSumActiveSignal()/getSumActiveLRF(p[0], p[1], 0.) : p[2];
    return fAutoE ? getLogLHautoE(p[0], p[1], p[2]) : getLogLH(p[0], p[1], p[2], p[3]);
}

// ============== Center of Gravity ===============
RecCoG::RecCoG(LRModel *lrm) : Reconstructor(lrm) {}

RecCoG::RecCoG(std::string json_str) : RecCoG(new LRModel(json_str)) {}

bool RecCoG::ProcessEvent(std::vector <double> &a, std::vector <bool> &sat)
{
    if (a.size() < nsensors || sat.size() < nsensors)
        return false;
    for (int i=0; i<nsensors; i++) {
        A[i] = a[i]/sensor[i].gain;
        this->sat[i] = sat[i];
    }
    guessByCOG();

    rec_x = guess_x;
    rec_y = guess_y;
    rec_status = 0;
    return true;
}
