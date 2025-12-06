#ifndef RECONSTRUCTOR_H
#define RECONSTRUCTOR_H

#include <vector>
#include <string>
#include <Eigen/Dense>
//#include "TMath.h"
#include "Math/Functor.h"
#include "Minuit2/Minuit2Minimizer.h"
#include "lrmodel.h"

class LRModel;

struct RecSensor
{
    double x;
    double y;
    double gain;
    bool on;
};

class Reconstructor
{
public:
    Reconstructor(LRModel *lrm);
    Reconstructor(std::string json_str);
    virtual ~Reconstructor();

    virtual bool ProcessEvent(std::vector <double> &a);
    virtual bool ProcessEvent(std::vector <double> &a, std::vector <bool> &sat) = 0;
    virtual bool ProcessEvent(std::vector <double> &a, std::vector <bool> &sat, std::vector <double> &guess);

// cost functions
    double getChi2(double x, double y, double z, double energy, bool weighted = true);
    double getChi2autoE(double x, double y, double z, bool weighted);
    double getLogLH(double x, double y, double z, double energy);
    double getLogLHautoE(double x, double y, double z);

// tracking of minimized value
    double LastMiniValue;

// public interface
public:
    std::string getVersion() {return "Pereira+CERN";}
    double getGuessX() {return guess_x;}
    double getGuessY() {return guess_y;}
    double getGuessZ() {return guess_z;}
    double getGuessE() {return guess_e;}
    int getRecStatus() {return rec_status;}
    int getDof() {return rec_dof;}
    double getRecX() {return rec_x;}
    double getRecY() {return rec_y;}
    double getRecZ() {return rec_z;}
    double getRecE() {return rec_e;}
    double getRecMin() {return rec_min;}
    double getChi2Min() {return getChi2(rec_x, rec_y, rec_z, rec_e);}
    double getCovXX() {return cov_xx;}
    double getCovYY() {return cov_yy;}
    double getCovXY() {return cov_xy;}
    void setCogAbsCutoff(double val) {cog_abs_cutoff = val;}
    void setCogRelCutoff(double val) {cog_rel_cutoff = val;}
    void setRecAbsCutoff(double val) {rec_abs_cutoff = val;}
    void setRecRelCutoff(double val) {rec_rel_cutoff = val;}
    void setRecCutoffRadius(double val) {rec_cutoff_radius = val;}
    void setEnergyCalibration(double val) {ecal = val;}
//    void setGain(int id, double val) {sensor.at(id).gain = val;}
//    void setGuessPosition(double x, double y, double z=0.) 
//            {guess_x = x; guess_y = y; guess_z = z; guess_pos_auto = false;}
//    void setGuessPositionAuto() {guess_pos_auto = true;}
//    void setGuessEnergy(double e) {guess_e = e; guess_e_auto = false;}
//    void setGuessEnergyAuto() {guess_e_auto = true;}

    double getSumSignal();
    double getSumLRF(double x, double y, double z);
    double getSumActiveSignal();
    double getSumActiveLRF(double x, double y, double z);

    LRModel *getLRModel() {return lrm;}
    std::string getLRModelJson() {return lrm->GetJsonString();}

protected:    
    void checkActive();
    int getMaxSignalID();
    void guessByMax();
    void guessByCOG();
    double getDistFromSensor(int id, double x, double y);

protected:
    LRModel *lrm;
    bool external_lrm = true;
    int nsensors = 0;
// cached sensor parameters
    std::vector <RecSensor> sensor;
    std::vector <int> Active;
// cached input parameters
    std::vector <double> A;
    std::vector <bool> sat;

// CoG
    double cog_abs_cutoff = 0.;
    double cog_rel_cutoff = 0.;

// initial guess
    bool guess_pos_auto = true;
    bool guess_e_auto = true;
    double guess_x;
    double guess_y;
    double guess_z;
    double guess_e;
    double ecal = 3.75e-5; // approximate scaling factor between SumSignal and energy

// dynamic passives
    double rec_cutoff_radius = 1.0e12; // all by default
    double rec_abs_cutoff = 0.;
    double rec_rel_cutoff = 0.;

// reconstruction result
    int rec_status;         // returned status of reconstruction
    double rec_x;			// reconstructed X position
    double rec_y;			// reconstructed Y position
    double rec_z;			// reconstructed Z position
    double rec_e;           // reconstructed energy
    double rec_min;         // reduced best chi-squared from reconstruction
    int rec_dof;			// degrees of freedom
    double cov_xx;		// variance in x
    double cov_yy;		// variance in y
    double cov_xy;		// covariance xy
};

class RecMinuit : public Reconstructor
{
public:
    RecMinuit(LRModel *lrm);
    RecMinuit(std::string json_str);
    virtual ~RecMinuit();
    virtual void InitCostFunction() = 0;
    using Reconstructor::ProcessEvent; // explicitly use ProcessEvent from the base class
    virtual bool ProcessEvent(std::vector <double> &a, std::vector <bool> &sat);
    virtual bool ProcessEvent(std::vector <double> &a, std::vector <bool> &sat, std::vector <double> &guess);

    void setRMstepX(double val) {RMstepX = val;}
    void setRMstepY(double val) {RMstepY = val;}
    void setRMstepZ(double val) {RMstepZ = val;}
    void setRMstepEnergy(double val) {RMstepEnergy = val;}
    void setRMmaxFuncCalls(int val) {RootMinimizer->SetMaxFunctionCalls(RMmaxFuncCalls = val);}
    void setRMmaxIterations(int val) {RootMinimizer->SetMaxIterations(RMmaxIterations = val);}
    void setRMtolerance(double val) {RootMinimizer->SetTolerance(RMtolerance = val);}
    void setMinuitVerbosity(int val);
    void setAutoE(bool val) {fAutoE = val;}
    void setFixedZ(double val) {fFixedZ = true; fixedZ = val;}
    void setFreeZ() {fFixedZ = false;}

protected:
// ROOT/Minuit stuff
    ROOT::Math::Functor *FunctorLSML = nullptr;
    ROOT::Minuit2::Minuit2Minimizer *RootMinimizer = nullptr;
// initial steps
    double RMstepX = 1.;
    double RMstepY = 1.;
    double RMstepZ = 1.;
    double RMstepEnergy = 0.;
// control over MINUIT2 stopping conditions
    int RMmaxFuncCalls = 500;       // Max function calls
    int RMmaxIterations = 1000; 	// Max iterations
    double RMtolerance = 0.001;		// Iteration stops when the function is within <tolerance> from the (estimated) min/max
// control over ROOT/MINUIT2 output
    int MinuitPrintLevel = 0;       // MINUIT2 messages
    int RootPrintLevel = 1001;      // ROOT messsages   

public:
    bool init_done = false;
    bool fAutoE = false;
    bool fFixedZ = true;
    double fixedZ = 0.;
};

class RecLS : public RecMinuit
{
public:
    RecLS(LRModel *lrm, bool weighted = true);
    RecLS(std::string json_str, bool weighted = true);
    virtual void InitCostFunction();
    double Cost(const double *p);
    bool fWeighted = true;
};

class RecML : public RecMinuit
{
public:
    RecML(LRModel *lrm);
    RecML(std::string json_str);
    virtual void InitCostFunction();
    double Cost(const double *p);
}; 

class RecCoG : public Reconstructor
{
public:
    RecCoG(LRModel *lrm);
    RecCoG(std::string json_str);
    virtual bool ProcessEvent(std::vector <double> &a, std::vector <bool> &sat);    
};

#endif // RECONSTRUCTOR_H
