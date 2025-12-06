#ifndef LRMODEL_H
#define LRMODEL_H

#include <vector>
#include <set>
#include <string>
#include <cmath>
#include "lrf.h"
#include "lrfio.h"
#include "profileHist.h"

class Transform;

class LRSensor
{
friend class LRModel;
public:
    double GetRadius() const;
    double GetPhi() const;
    double GetDistance(double x1, double y1) const
        {return sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y));}
    bool operator < (const LRSensor& s) const {
        return (GetRadius() < s.GetRadius());
    }
    static bool Compare_R(const LRSensor &a, const LRSensor &b)
        { return (a.GetRadius() < b.GetRadius()); }
    static bool Compare_Phi(const LRSensor &a, const LRSensor &b)
        { return (a.GetPhi() < b.GetPhi()); }
    static double Distance(const LRSensor &a, const LRSensor &b)
        { return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)); }

protected:
    int id = -1;        // must coincide with vector element
    int group_id = -1;  // no group by default
    double x, y;        // PMT position
//  double z = 0.;      // later
//  double normal[3];   // later
    bool disabled = false; // state, enabled by default 
    double gain = 1.0;  // relative gain
    Transform *tr = nullptr;
    LRF *lrf = nullptr;
};

struct LRGroup
{
    int id = -1;
    std::set <int> members;
    double x, y;        // coordinates of the reference point
//  double z = 0.;      // later
//  double normal[3];   // later
    LRF *glrf = nullptr;
};

struct LCcorr
{
    double xref = 0;
    double yref = 0;
    double zref = 0;
// the entries below will be used to speed up calculation
//    bool ready = false;
//    double refsum;
};

class LRModel
{
    enum UngroupPolicy {
        KeepLRF,       // clone the group LRF and keep it
        ResetLRF       // clone the default LRF
    };

public:
    LRModel(int n);
    LRModel(int n, LRF *default_lrf);
    LRModel(const Json &json);
    LRModel(std::string json_str);
    ~LRModel();

    std::string Version() {return "1.01";}

// enabled/disabled sensors
    void SetDisabled(int id) {Sensor.at(id).disabled = true;}
    void SetEnabled(int id) {Sensor.at(id).disabled = false;}
    bool IsDisabled(int id) {return Sensor.at(id).disabled;}
    void SetDisabledList(std::vector <int> list) {for (int id : list) SetDisabled(id);}
    void SetEnabledList(std::vector <int> list) {for (int id : list) SetEnabled(id);}
    std::vector <int> GetDisabledList();
    std::vector <int> GetEnabledList();

    void SetGain(int id, double gain) {Sensor.at(id).gain = gain;}
    void SetGroup(int id, int group) {Sensor.at(id).group_id = group;}
//    void SetLRFid(int id, int lrfid) {Sensor.at(id).lrfid = lrfid;}
//    void SetGroupLRFid(int gid, int lrfid) {Group.at(gid).lrfid = lrfid;}
    void SetTransform(int id, Transform *tr);
    bool SetTransform(int id, std::string &json_tr); // for python

    double GetGain(int id) const {return Sensor.at(id).gain;}
    int GetGroup(int id) const {return Sensor.at(id).group_id;}
//    int  GetLRFid(int id) const {return Sensor.at(id).lrfid;}
//    int  GetGroupLRFid(int gid) const {return Group.at(gid).lrfid;}
    Transform *GetTransform(int id) const {return Sensor.at(id).tr;}
    std::string GetJsonTransform(int id) const; // for python
    double GetX(int id) const {return Sensor.at(id).x;}
    double GetY(int id) const {return Sensor.at(id).y;}
    std::vector <double> GetAllX() const;
    std::vector <double> GetAllY() const;
    double GetGroupX(int gid) const {return Group.at(gid).x;}
    double GetGroupY(int gid) const {return Group.at(gid).y;}
    double GetRadius(int id) const {return Sensor.at(id).GetRadius();}
    double GetPhi(int id) const {return Sensor.at(id).GetPhi();}
    double GetDistance(int ida, int idb) const
        {return LRSensor::Distance(Sensor.at(ida), Sensor.at(idb));}

    std::set <int> &GroupMembers(int gid) {return Group.at(gid).members;}

    int GetSensorCount() const {return Sensor.size();}
    int GetGroupCount() const {return Group.size();}
    int GetGroupMembersCount(int gid) const {return Group.at(gid).members.size();}
    bool SensorExists(int id) const {return id>=0 && id<Sensor.size();}
    bool GroupExists(int gid) const {return gid>=0 && gid<Group.size();}
//    int GetLRFCount() const {return Lrf.size();}

    void ClearAll();
    void ResetGroups();
    void AddSensor(int id, double x, double y);

    int CreateGroup();
    bool DissolveGroup(int gid);
    bool AddToGroup(int id, int gid, Transform *tr);
    bool AddToGroup(int id, int gid, std::string &json_tr);
    bool RemoveFromGroup(int id, UngroupPolicy policy = KeepLRF);

    void MakeGroupsCommon();
    void MakeGroupsByRadius();
    void MakeGroupsRectangle() { MakeGroupsByTransform(MakeVtrRectangle()); }
    void MakeGroupsSquare() { MakeGroupsByTransform(MakeVtrSquare()); }
    void MakeGroupsHexagon() { MakeGroupsByTransform(MakeVtrHexagon()); }
    void MakeGroupsNgon(int n) { MakeGroupsByTransform(MakeVtrNgon(n)); }

    void MakeRotGroup(std::vector <LRSensor> &ring);
    void MakeGroupsByTransform(std::vector <Transform*> vtr);
    std::vector <Transform*> MakeVtrRectangle();
    std::vector <Transform*> MakeVtrSquare();
    std::vector <Transform*> MakeVtrHexagon();
    std::vector <Transform*> MakeVtrNgon(int n);
    void SetTolerance(double tolerance) {tol = tolerance;}

// Access to LRFs
    void SetLRF(int id, LRF *lrfptr);
    void SetJsonLRF(int id, std::string json_str) {SetLRF(id, LRF::mkFromJson(json_str));}
    LRF *GetLRF(int id);
    std::string GetJsonLRF(int id) {LRF *p; return (p = GetLRF(id)) ? p->GetJsonString() : "";}
    void SetGroupLRF(int gid, LRF *lrfptr);
    void SetGroupJsonLRF(int gid, std::string json_str) {SetGroupLRF(gid, LRF::mkFromJson(json_str));}
    LRF *GetGroupLRF(int gid);
    std::string GetGroupJsonLRF(int gid) {LRF *p; return (p = GetGroupLRF(gid)) ? p->GetJsonString() : "";}
    void SetDefaultLRF(LRF *default_lrf) {DefaultLRF = default_lrf;}
    void SetDefaultJsonLRF(std::string json_str) {DefaultLRF = LRF::mkFromJson(json_str);}

// Access to Profile Histograms
    std::vector <int> GetHistBins(int id);
    int GetHistNdim(int id);
    int GetHistBinsX(int id);
    int GetHistBinsY(int id);
    int GetHistBinsZ(int id);    
    std::vector <double> GetHistMeans(int id);
    std::vector <double> GetHistSigmas(int id);
    std::vector <double> GetHistWeights(int id);
    std::vector <double> GetHistXCenters(int id);
    std::vector <double> GetHistYCenters(int id);
    std::vector <double> GetHistZCenters(int id);
    // this one returns LRF evaluated at bin centers
    std::vector <double> GetHistFit(int id);


// Evaluation
    bool InDomain(int id, double *pos_world);
// ToDo:    void DoTransform(int id, double *pos_world) {};
    double Eval(int id, double *pos_world);
    double EvalLocal(int id, double *pos_local) { return GetLRF(id)->eval(pos_local)*GetGain(id); }
    double EvalDrvX(int id, double *pos_world);
    double EvalDrvY(int id, double *pos_world);
// direct coordinate versions
    bool InDomain(int id, double x, double y, double z);
    double Eval(int id, double x, double y, double z);
    double EvalDrvX(int id, double x, double y, double z);
    double EvalDrvY(int id, double x, double y, double z);
// direct coordinate vector data versions
//    bool InDomain(int id, std::vector<double> &x, std::vector<double> &y, std::vector<double> &z);
    std::vector<double> Eval(int id, const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z);
//    double EvalDrvX(int id, std::vector<double> &x, std::vector<double> &y, std::vector<double> &z);
//    double EvalDrvY(int id, std::vector<double> &x, std::vector<double> &y, std::vector<double> &z);
// utility (plotting aid)
    double EvalAxial(int id, double r);
// direct coordinate multiple sensor versions
    std::vector <bool> InDomainAll(double x, double y, double z);
    std::vector <bool> InDomainList(std::vector <int> list, double x, double y, double z);
    std::vector <double> EvalAll(double x, double y, double z);
    std::vector <double> EvalList(std::vector <int> list, double x, double y, double z);   
    std::vector <double> EvalDrvXAll(double x, double y, double z);
    std::vector <double> EvalDrvXList(std::vector <int> list, double x, double y, double z);
    std::vector <double> EvalDrvYAll(double x, double y, double z);
    std::vector <double> EvalDrvYList(std::vector <int> list, double x, double y, double z);

// Fitting
    // direct (not binned)
    bool FitNotBinnedData(int id, const std::vector <Vec4data> &data);
    // binned
    bool AddFitData(int id, const std::vector <Vec4data> &data);
    bool AddFitRawData(int id, const std::vector <Vec3data> &xyz, const std::vector <double> &a, const std::vector <bool> &good);
    void FitSensor(int id);
    void FitGroup(int gid);
    void ClearSensorFitData(int id);
    void ClearGroupFitData(int gid);
    void ClearAllFitData();

// Correction factors (light collection)
    bool SetRefPoint(double x, double y, double z=0);
    double GetLRFSum(double x, double y, std::vector <bool> *mask=new std::vector <bool>);
    double GetLRFSum(double x, double y, double z, std::vector <bool> *mask=new std::vector <bool>);
    std::vector <double> GetLRFSum(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > *mask=new std::vector<std::vector <bool> >);
    std::vector <double> GetLRFSum(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > *mask=new std::vector<std::vector <bool> >);
    double GetLRFSumList(std::vector <int> &list, double x, double y, double z);

    double GetCorrFactor(double x, double y, std::vector <bool> *mask=new std::vector <bool>);
    double GetCorrFactor(double x, double y, double z, std::vector <bool> *mask=new std::vector <bool>);
    std::vector <double> GetCorrFactor(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > *mask=new std::vector<std::vector <bool> >);
    std::vector <double> GetCorrFactor(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > *mask=new std::vector<std::vector <bool> >);
    double GetCorrFactorList(std::vector <int> &list, double x, double y, double z);

/*
//    std::vector <double> GetLRFSum(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z);
//    std::vector <double> GetLRFSum2D(std::vector <double> &x, std::vector <double> &y);
    double GetCorrFactor(double x, double y, double z);
    std::vector <double> GetCorrFactor(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z);
    std::vector <double> GetCorrFactor2D(std::vector <double> &x, std::vector <double> &y);

//    double GetLRFSumMask(double x, double y, double z, std::vector <bool> &mask);
//    std::vector <double> GetLRFSumMask(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > &mask);
//    std::vector <double> GetLRFSumMask2D(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > &mask);
    double GetCorrFactorList(double x, double y, double z, std::vector <int> &list);
    double GetCorrFactorMask(double x, double y, double z, std::vector <bool> &mask);
    std::vector <double> GetCorrFactorMask(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > &mask);
    std::vector <double> GetCorrFactorMask2D(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > &mask);
*/
    double GetCorrectedArea(std::vector <double> &area, std::vector <bool> &mask, double x, double y, double z);
    std::vector <double> GetCorrectedArea(std::vector <std::vector <double> > &area, std::vector <std::vector <bool> > &mask, std::vector <double> &x, std::vector <double> &y, std::vector <double> &z);


// Save and Load
    Json_object SensorGetJsonObject(int id) const;
    Json_object GroupGetJsonObject(int gid) const;
    void ToJsonObject(Json_object &json) const;
    Json_object GetJsonObject() const;
    std::string GetJsonString() const;
    std::string SensorGetJsonString(int id) const;
    std::string GroupGetJsonString(int gid) const;

    void ReadSensor(const Json &json);
    void ReadGroup(const Json &json);

// Reporting
    std::string GetLRFError() {return LRF::gjson_err;}
    std::string GetError() {return lrm_err;}     

// Utility
    double GetMaxR(int id, const std::vector <LRFdata> &data) const;
    double GetGroupMaxR(int gid, const std::vector <LRFdata> &data) const;

protected:
    std::vector <LRSensor> Sensor;
    std::vector <LRGroup> Group;
//    std::vector <LRF*> Lrf;
    LRF *DefaultLRF = nullptr;
    std::string json_err;
    std::string lrm_err;
    double tol = 1.0e-4;
    std::vector <double> empty_vector;
// LC correction
    LCcorr corr;
};

class GainEstimator
{
public:
    GainEstimator(LRModel *lrm) {Init(lrm);}
    GainEstimator(std::string lrmtxt);
    ~GainEstimator();
    bool AddData(int id, const std::vector <Vec4data> &data);
    bool AddRawData(int id, const std::vector <Vec3data> &xyz, const std::vector <double> &a, const std::vector <bool> &good);
    double GetRelativeGain(int id, int refid);
    std::vector <double> GetRelativeGainsList(std::vector <int> ids, int refid);
//    std::vector <int> GetAllSensors();

protected:
    void Init(LRModel *lrm);
    
protected:
    LRModel *M;
    int size;
};

#endif // LRMODEL_H
