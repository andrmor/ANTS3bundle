#ifndef RECONSTRUCTOR_MP_H
#define RECONSTRUCTOR_MP_H

#include <vector>
#include <string>
//#include "TMath.h"
//#include "Math/Functor.h"
//#include "Minuit2/Minuit2Minimizer.h"

#include "reconstructor.h"

class LRModel;

class ReconstructorMP
{
public:
    ReconstructorMP() {;}
    ReconstructorMP(LRModel *lrm, int n_threads);
    ReconstructorMP(std::string json_str, int n_threads);
    virtual ~ReconstructorMP();

    void ProcessEvents (std::vector <std::vector <double> > &A);
    void ProcessEvents (std::vector <std::vector <double> > &A, std::vector <std::vector <bool> > &Sat);
    void ProcessEvents (std::vector <std::vector <double> > &A, std::vector <std::vector <bool> > &Sat, std::vector <std::vector <double> > &Guess);

// public interface
public:
    std::vector <int> getRecStatus() {return rec_status;}
    std::vector <int> getDof() {return rec_dof;}
    std::vector <double> getRecX() {return rec_x;}
    std::vector <double> getRecY() {return rec_y;}
    std::vector <double> getRecZ() {return rec_z;}
    std::vector <double> getRecE() {return rec_e;}
    std::vector <double> getRecMin() {return rec_min;}
    std::vector <double> getChi2Min() {return rec_chi2min;}
    std::vector <double> getCovXX() {return cov_xx;}
    std::vector <double> getCovYY() {return cov_yy;}
    std::vector <double> getCovXY() {return cov_xy;}

    void setCogAbsCutoff(double val) {for (auto r : recs) r->setCogAbsCutoff(val);}
    void setCogRelCutoff(double val) {for (auto r : recs) r->setCogRelCutoff(val);}
    void setRecAbsCutoff(double val) {for (auto r : recs) r->setRecAbsCutoff(val);}
    void setRecRelCutoff(double val) {for (auto r : recs) r->setRecRelCutoff(val);}
    void setRecCutoffRadius(double val) {for (auto r : recs) r->setRecCutoffRadius(val);}
    void setEnergyCalibration(double val) {for (auto r : recs) r->setEnergyCalibration(val);}

// reconstruction result
    std::vector <int> rec_status;         // returned status of reconstruction
    std::vector <double> rec_x;			// reconstructed X position
    std::vector <double> rec_y;			// reconstructed Y position
    std::vector <double> rec_z;			// reconstructed Z position
    std::vector <double> rec_e;           // reconstructed energy
    std::vector <double> rec_min;         // minimum found by reconstruction
    std::vector <double> rec_chi2min;     // reduced (?) chi-squared from reconstruction
    std::vector <int> rec_dof;			// degrees of freedom
    std::vector <double> cov_xx;		// variance in x
    std::vector <double> cov_yy;		// variance in y
    std::vector <double> cov_xy;		// covariance xy

// reconstructors
    std::vector <Reconstructor*> recs;    
};

class RecMinuitMP : public ReconstructorMP
{
public:
    using ReconstructorMP::ReconstructorMP;
//    RecMinuitMP(LRModel *lrm, int n_threads);
//    RecMinuitMP(std::string json_str, int n_threads);
//    virtual ~RecMinuitMP();

    void setRMstepX(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMstepX(val);}
    void setRMstepY(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMstepY(val);}
    void setRMstepZ(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMstepZ(val);}
    void setRMstepEnergy(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMstepEnergy(val);}
    void setRMmaxFuncCalls(int val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMmaxFuncCalls(val);}
    void setRMmaxIterations(int val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMmaxIterations(val);}
    void setRMtolerance(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setRMtolerance(val);}
    void setMinuitVerbosity(int val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setMinuitVerbosity(val);}
    void setAutoE(bool val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setAutoE(val);}
    void setFixedZ(double val) {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setFixedZ(val);}
    void setFreeZ() {for (Reconstructor* r : recs) dynamic_cast<RecMinuit*>(r)->setFreeZ();}
};

class RecLS_MP : public RecMinuitMP
{
public:
    RecLS_MP(LRModel *lrm, int n_threads, bool weighted = true);
    RecLS_MP(std::string json_str, int n_threads, bool weighted = true);
};

class RecML_MP : public RecMinuitMP
{
public:
    RecML_MP(LRModel *lrm, int n_threads);
    RecML_MP(std::string json_str, int n_threads);
}; 

#endif // RECONSTRUCTOR_MP_H
