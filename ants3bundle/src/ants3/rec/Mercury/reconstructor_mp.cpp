#include "reconstructor_mp.h"
#include "reconstructor.h"
#include "omp.h"

ReconstructorMP::ReconstructorMP(LRModel *lrm, int n_threads)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecCoG(lrm));
    }
}

ReconstructorMP::ReconstructorMP(std::string json_str, int n_threads)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecCoG(json_str));
    }
}

ReconstructorMP::~ReconstructorMP()
{
    for (size_t i=0; i<recs.size(); i++) {
        delete recs[i];
    }
}

void ReconstructorMP::ProcessEvents (std::vector <std::vector <double> > &A)
{
    int nevt = A.size();
    if (nevt) {
        int nsens = A[0].size();
        std::vector <bool> sat(nsens, false);
        std::vector <std::vector <bool> > Sat(nevt, sat);
        ProcessEvents(A, Sat);
    }
}

void ReconstructorMP::ProcessEvents (std::vector <std::vector <double> > &A, std::vector <std::vector <bool> > &Sat)
{
    int n_thr = recs.size();
    int n_evt = A.size();

// resize the output vectors
    rec_status.resize(n_evt);         // returned status of reconstruction
    rec_x.resize(n_evt);			// reconstructed X position
    rec_y.resize(n_evt);			// reconstructed Y position
    rec_z.resize(n_evt);			// reconstructed Z position
    rec_e.resize(n_evt);           // reconstructed energy
    rec_min.resize(n_evt);         // minimum found by reconstruction
    rec_chi2min.resize(n_evt);     // reduced (?) chi-squared from reconstruction
    rec_dof.resize(n_evt);			// degrees of freedom
    cov_xx.resize(n_evt);		// variance in x
    cov_yy.resize(n_evt);		// variance in y
    cov_xy.resize(n_evt);		// covariance xy

    int jmin[n_thr], jmax[n_thr];
    int chunk = n_evt/n_thr;
    for (int i=0; i<n_thr; i++) {
        jmin[i] = i*chunk;
        jmax[i] = (i+1)*chunk;
//        std::cout << jmin[i] << ", " << jmax[i] << std::endl;
    }
    jmax[n_thr-1] = n_evt;

    #pragma omp parallel for
    for (int i=0; i<n_thr; i++) {
        for (int j=jmin[i]; j<jmax[i]; j++) {
            recs[i]->ProcessEvent(A[j], Sat[j]);
            rec_status[j] = recs[i]->getRecStatus();
            rec_x[j] = recs[i]->getRecX();
            rec_y[j] = recs[i]->getRecY();
            rec_z[j] = recs[i]->getRecZ();
            rec_e[j] = recs[i]->getRecE();
            rec_min[j] = recs[i]->getRecMin();
            rec_chi2min[j] = recs[i]->getChi2(rec_x[j], rec_y[j], rec_z[j], rec_e[j]);
            rec_dof[j] = recs[i]->getDof();
            cov_xx[j] = recs[i]->getCovXX();
            cov_yy[j] = recs[i]->getCovYY();
            cov_xy[j] = recs[i]->getCovXY();
        }
    }
}

void ReconstructorMP::ProcessEvents (std::vector <std::vector <double> > &A, std::vector <std::vector <bool> > &Sat, std::vector <std::vector <double> > &Guess)
{
    int n_thr = recs.size();
    int n_evt = A.size();

// resize the output vectors
    rec_status.resize(n_evt);         // returned status of reconstruction
    rec_x.resize(n_evt);			// reconstructed X position
    rec_y.resize(n_evt);			// reconstructed Y position
    rec_z.resize(n_evt);			// reconstructed Z position
    rec_e.resize(n_evt);           // reconstructed energy
    rec_min.resize(n_evt);         // minimum found by reconstruction
    rec_chi2min.resize(n_evt);     // reduced (?) chi-squared from reconstruction
    rec_dof.resize(n_evt);			// degrees of freedom
    cov_xx.resize(n_evt);		// variance in x
    cov_yy.resize(n_evt);		// variance in y
    cov_xy.resize(n_evt);		// covariance xy

    int jmin[n_thr], jmax[n_thr];
    int chunk = n_evt/n_thr;
    for (int i=0; i<n_thr; i++) {
        jmin[i] = i*chunk;
        jmax[i] = (i+1)*chunk;
//        std::cout << jmin[i] << ", " << jmax[i] << std::endl;
    }
    jmax[n_thr-1] = n_evt;

    #pragma omp parallel for
    for (int i=0; i<n_thr; i++) {
        for (int j=jmin[i]; j<jmax[i]; j++) {
            recs[i]->ProcessEvent(A[j], Sat[j], Guess[j]);
            rec_status[j] = recs[i]->getRecStatus();
            rec_x[j] = recs[i]->getRecX();
            rec_y[j] = recs[i]->getRecY();
            rec_z[j] = recs[i]->getRecZ();
            rec_e[j] = recs[i]->getRecE();
            rec_min[j] = recs[i]->getRecMin();
            rec_chi2min[j] = recs[i]->getChi2(rec_x[j], rec_y[j], 0., rec_e[j]);
            rec_dof[j] = recs[i]->getDof();
            cov_xx[j] = recs[i]->getCovXX();
            cov_yy[j] = recs[i]->getCovYY();
            cov_xy[j] = recs[i]->getCovXY();
        }
    }
}

RecLS_MP::RecLS_MP(LRModel *lrm, int n_threads, bool weighted)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecLS(lrm, weighted));
    }
}
RecLS_MP::RecLS_MP(std::string json_str, int n_threads, bool weighted)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecLS(json_str, weighted));
    }
}

RecML_MP::RecML_MP(LRModel *lrm, int n_threads)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecML(lrm));
    }
}
RecML_MP::RecML_MP(std::string json_str, int n_threads)
{
    for (int i=0; i<n_threads; i++) {
        recs.push_back(new RecML(json_str));
    }
}
