#include "lrmodel.h"
#include "lrf.h"
#include "lrfaxial.h"
#include "transform.h"
#include "profileHist.h"
#include <cmath>
#include "json11.hpp"
//#include <complex>
#include <iostream>

double LRSensor::GetRadius() const
{
    return sqrt( x*x + y*y );
}

double LRSensor::GetPhi() const
{
    double phi = atan2 (y, x);
    // atan2 maps into [-pi;pi], make it [0;2*pi]
    if (phi < -1.0e-6) phi += 2.0*M_PI;
    return phi;
}

LRModel::LRModel(int n)
{
    Sensor.resize(n);
}

LRModel::LRModel(int n, LRF *default_lrf)
{
    Sensor.resize(n);
    DefaultLRF = default_lrf;
}

LRModel::~LRModel()
{
    ClearAll();
    delete DefaultLRF;
}

void LRModel::ClearAll()
{
    for (LRSensor s : Sensor) {
        delete s.tr;
        delete s.lrf;
    }
    for (LRGroup g : Group) {
        delete g.glrf;
    }
    Sensor.clear();
    Group.clear();
}

void LRModel::ResetGroups()
{
    for (int gid=0; gid<GetGroupCount(); gid++) {
        std::set <int> members = GroupMembers(gid);
        for (int i : members)
            RemoveFromGroup(i);
        delete Group.at(gid).glrf;
    }
    Group.clear();
}

void LRModel::AddSensor(int id, double x, double y)
{
    Sensor.at(id).id = id; // this is a canary to make sure that all sensors are initialized
    Sensor.at(id).x = x;
    Sensor.at(id).y = y;
//    Sensor.at(id).z = z;
    if (DefaultLRF)
        Sensor.at(id).lrf = DefaultLRF->clone();
}

std::vector <double> LRModel::GetAllX() const
{
    std::vector <double> tmp;
    tmp.reserve(GetSensorCount());
    for (LRSensor s : Sensor)
        tmp.push_back(s.x);
    return tmp;
}

std::vector <double> LRModel::GetAllY() const
{
    std::vector <double> tmp;
    tmp.reserve(GetSensorCount());
    for (LRSensor s : Sensor)
        tmp.push_back(s.y);
    return tmp;
}

int LRModel::CreateGroup()
{
    LRGroup g;
    if (DefaultLRF)
        g.glrf = DefaultLRF->clone();
    g.id = Group.size();
    Group.push_back(g);  
    return g.id;
}

bool LRModel::DissolveGroup(int gid)
{
    if (gid >= Group.size())
        return false;

    std::set <int> members = GroupMembers(gid);
    for (int i : members)
        RemoveFromGroup(i);

    // the group gets automatically erased from the Group vector with removal of the last sensor
    return true;
}

bool LRModel::AddToGroup(int id, int gid, Transform *tr)
{
    if (!SensorExists(id) || !GroupExists(gid))
        return false;
    if (GetGroup(id) != -1)
        return false; // already belongs to another group

    SetGroup(id, gid);
    SetTransform(id, tr);
    GroupMembers(gid).insert(id);
    return true;
}

bool LRModel::AddToGroup(int id, int gid, std::string &json_tr)
{
    Transform *tr = Transform::Factory(json_tr);
    if (tr) {
        return AddToGroup(id, gid, tr);
    }

    return false;    
}

bool LRModel::RemoveFromGroup(int id, UngroupPolicy policy)
{
    int gid = GetGroup(id);
    if (gid == -1)
        return false; // no group => nothing to do

    SetGroup(id, -1);
    delete Sensor.at(id).lrf;
    switch (policy) {
        case KeepLRF:
            Sensor.at(id).lrf = Group.at(gid).glrf ? Group.at(gid).glrf->clone() : nullptr;
            break;
        case ResetLRF:
            Sensor.at(id).lrf = DefaultLRF ? DefaultLRF->clone() : nullptr;
            SetTransform(id, nullptr);
            SetGain(id, 1.0);
            break;
    }

    GroupMembers(gid).erase(id);
    if (GroupMembers(gid).size() == 0) {
        delete Group.at(gid).glrf;
        Group.erase(Group.begin()+gid);
    }

    return true;
}

std::vector <int> LRModel::GetDisabledList()
{
    std::vector <int> tmp;
    for (LRSensor s : Sensor)
        if (s.disabled)
            tmp.push_back(s.id);
    return tmp;
}

std::vector <int> LRModel::GetEnabledList()
{
    std::vector <int> tmp;
    for (LRSensor s : Sensor)
        if (!s.disabled)
            tmp.push_back(s.id);
    return tmp;
}

void LRModel::SetTransform(int id, Transform *tr)
{
    delete Sensor.at(id).tr;
    Sensor.at(id).tr = tr;
}

bool LRModel::SetTransform(int id, std::string &json_tr)
{
    Transform *tr = Transform::Factory(json_tr);
    if (tr) {
        SetTransform(id, tr);
        return true;
    }

    return false;
}

std::string LRModel::GetJsonTransform(int id) const 
{
    Transform *tr = GetTransform(id);
    return tr ? tr->GetJsonString() : "";
}

void LRModel::SetLRF(int id, LRF *lrfptr)
{
    RemoveFromGroup(id);
    delete Sensor.at(id).lrf;
    Sensor.at(id).lrf = lrfptr;
}

LRF *LRModel::GetLRF(int id)
{
    int gid = GetGroup(id);
    return ( gid == -1 ? Sensor.at(id).lrf
                       : Group.at(gid).glrf );
}

void LRModel::SetGroupLRF(int gid, LRF *lrfptr)
{
    delete Group.at(gid).glrf;
    Group.at(gid).glrf = lrfptr;
}

LRF *LRModel::GetGroupLRF(int gid)
{
    return Group.at(gid).glrf;
}

// Evaluation: direct coordinates versions

bool LRModel::InDomain(int id, double x, double y, double z)
{
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    return GetLRF(id) ? GetLRF(id)->inDomain(x, y, z) : 0;
}

double LRModel::Eval(int id, double x, double y, double z)
{
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->eval(x, y, z)*GetGain(id) : 0;
}

double LRModel::EvalDrvX(int id, double x, double y, double z)
{
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->evalDrvX(x, y, z)*GetGain(id) : 0;
}

double LRModel::EvalDrvY(int id, double x, double y, double z)
{
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->evalDrvY(x, y, z)*GetGain(id) : 0;
}

double LRModel::EvalAxial(int id, double r)
{
    LRF *f = GetLRF(id);
    return f ? f->evalAxial(r)*GetGain(id) : 0;
}

// Evaluation: direct coordinate vector data versions

std::vector<double> LRModel::Eval(int id, const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z)
{
    size_t vlen = x.size();
    if (vlen != y.size() || vlen != z.size())
        throw std::length_error("Eval: Input vectors must be of the same length");
    
    LRF *f = GetLRF(id);
    if (!f)
        throw std::runtime_error(std::string("Eval: sensor with id = ") + std::to_string(id) + "does not exist or has no LRF");

    Transform *trf = GetTransform(id);
    double gain = GetGain(id);
    double xx, yy, zz;
    std::vector<double> res(vlen);
    
    for (size_t i=0; i<vlen; i++) {
        xx = x[i]; yy = y[i]; zz = z[i];
        if (trf)
            trf->DoTransform(&xx, &yy, &zz);
        res[i] = f->eval(xx, yy, zz)*gain;
    }

    return res;
}

// Evaluation: direct coordinate multiple sensor versions

std::vector <bool> LRModel::InDomainAll(double x, double y, double z)
{
    int len = GetSensorCount();
    std::vector <bool> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = InDomain(i, x, y, z);
    return tmp;
}

std::vector <bool> LRModel::InDomainList(std::vector <int> list, double x, double y, double z)
{
    int len = list.size();
    std::vector <bool> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = InDomain(list[i], x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalAll(double x, double y, double z)
{
    int len = GetSensorCount();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = Eval(i, x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalList(std::vector <int> list, double x, double y, double z)
{
    int len = list.size();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = Eval(list[i], x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalDrvXAll(double x, double y, double z)
{
    int len = GetSensorCount();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = EvalDrvX(i, x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalDrvXList(std::vector <int> list, double x, double y, double z)
{
    int len = list.size();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = EvalDrvX(list[i], x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalDrvYAll(double x, double y, double z)
{
    int len = GetSensorCount();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = EvalDrvY(i, x, y, z);
    return tmp;
}

std::vector <double> LRModel::EvalDrvYList(std::vector <int> list, double x, double y, double z)
{
    int len = list.size();
    std::vector <double> tmp(len);

    for (int i=0; i<len; i++)
        tmp[i] = EvalDrvY(list[i], x, y, z);
    return tmp;
}

// Evaluation: indirect coordinates versions

bool LRModel::InDomain(int id, double *pos_world)
{
    double x = pos_world[0];
    double y = pos_world[1];
    double z = pos_world[2];
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->inDomain(x, y, z) : false;
}

double LRModel::Eval(int id, double *pos_world)
{
    double x = pos_world[0];
    double y = pos_world[1];
    double z = pos_world[2];
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->eval(x, y, z)*GetGain(id) : 0.;
}

double LRModel::EvalDrvX(int id, double *pos_world)
{
    double x = pos_world[0];
    double y = pos_world[1];
    double z = pos_world[2];
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->evalDrvX(x, y, z)*GetGain(id) : 0.;
}

double LRModel::EvalDrvY(int id, double *pos_world)
{
    double x = pos_world[0];
    double y = pos_world[1];
    double z = pos_world[2];
    if (GetTransform(id))
        GetTransform(id)->DoTransform(&x, &y, &z);
    LRF *f = GetLRF(id);
    return f ? f->evalDrvY(x, y, z)*GetGain(id) : 0.;
}

bool LRModel::FitNotBinnedData(int id, const std::vector <Vec4data> &data)
{
    if (IsDisabled(id)) { // don't use disabled sensors in the fit
        lrm_err = "FitNotBinnedData: This sensor is disabled";
        return false;
    }

    std::vector <LRFdata> trdata;
    Transform *tr = GetTransform(id);
    double gain = GetGain(id);
    for (Vec4data v : data) {
        LRFdata d(v[0], v[1], v[2], v[3]);
        if (tr)
            tr->DoTransform(&d.x, &d.y, &d.z);
        d.val /= gain;
        trdata.push_back(d);
    }
    LRF *f = GetLRF(id);
    return f ? f->addData(trdata) : false;
}

bool LRModel::AddFitData(int id, const std::vector <Vec4data> &data)
{
    if (IsDisabled(id)) { // don't use disabled sensors in the fit
        lrm_err = "AddFitData: This sensor is disabled";
        return false;
    }

    std::vector <LRFdata> trdata;
    trdata.reserve(data.size());
    Transform *tr = GetTransform(id);
    double gain = GetGain(id);
    for (Vec4data v : data) {
        LRFdata d(v[0], v[1], v[2], v[3]);
        if (tr)
            tr->DoTransform(&d.x, &d.y, &d.z);
        d.val /= gain;
        trdata.push_back(d);
    }

    int gid = GetGroup(id);
    LRF *f = gid >= 0 ? GetGroupLRF(gid) : GetLRF(id);
    return f ? f->addData(trdata) : false;
}

bool LRModel::AddFitRawData(int id, const std::vector <Vec3data> &xyz, const std::vector <double> &a, const std::vector <bool> &good)
{
    if (IsDisabled(id)) { // don't use disabled sensors in the fit
        lrm_err = "AddFitRawData: This sensor is disabled";
        return false;
    }

    size_t len = xyz.size();
    if (a.size() != len || good.size() != len) {
        lrm_err = "AddFitRawData: Lengths of the input arrays aren't equal";
        return false;
    }

    std::vector <LRFdata> trdata;
    trdata.reserve(len);
    Transform *tr = GetTransform(id);
    double gain = GetGain(id);
    for (size_t i=0; i<len; i++) {
        if (!good[i])
            continue;
        LRFdata d(xyz[i][0], xyz[i][1], xyz[i][2], a[i]);
        if (tr)
            tr->DoTransform(&d.x, &d.y, &d.z);
        d.val /= gain;
        trdata.push_back(d);
    }

    int gid = GetGroup(id);
    LRF *f = gid >= 0 ? GetGroupLRF(gid) : GetLRF(id);
    return f ? f->addData(trdata) : false;
}

void LRModel::FitSensor(int id)
{
    LRF *f = GetLRF(id);
    if (f)
        f->doFit();
    else 
        throw std::runtime_error(std::string("FitSensor: sensor with id = ") + std::to_string(id) + "does not exist or has no LRF");
}

void LRModel::FitGroup(int gid)
{
    LRF *f = GetGroupLRF(gid);
    if (f)
        f->doFit();
    else 
        throw std::runtime_error(std::string("ClearGroupFitData: group with id = ") + std::to_string(gid) + "does not exist or has no LRF");
}

void LRModel::ClearSensorFitData(int id)
{
    LRF *f = GetLRF(id);
    if (f)
        f->clearData();
    else 
        throw std::runtime_error(std::string("ClearSensorFitData: sensor with id = ") + std::to_string(id) + "does not exist or has no LRF");
}

void LRModel::ClearGroupFitData(int gid)
{
    LRF *f = GetGroupLRF(gid);
    if (f)
        f->clearData();
    else 
        throw std::runtime_error(std::string("ClearGroupFitData: group with id = ") + std::to_string(gid) + "does not exist or has no LRF");
}

void LRModel::ClearAllFitData()
{
    for (LRSensor s : Sensor)
        if (s.group_id >= 0 && s.lrf)
            s.lrf->clearData();
    for (LRGroup g : Group)
        if (g.glrf)
            g.glrf->clearData();
}

void LRModel::MakeGroupsCommon()
{
//    Reset();
    int gid = CreateGroup();
    for (LRSensor s : Sensor) {
        Transform *tr = new TranslateLRF(-s.x, -s.y);
        AddToGroup(s.id, gid, tr);
    }
}

void LRModel::MakeGroupsByRadius()
{
//    Reset();
    double R = 0.;
    std::vector <LRSensor> tmp_group;
    std::vector <LRSensor> local_sensors = Sensor;
    std::sort (local_sensors.begin(), local_sensors.end(), LRSensor::Compare_R);

    for (LRSensor s : local_sensors) {
        if (fabs(R-s.GetRadius()) <= tol)
            tmp_group.push_back(s);
        else {
            if (tmp_group.size() >= 2)
                MakeRotGroup(tmp_group);
            tmp_group.clear();
            tmp_group.push_back(s);
            R = s.GetRadius();
        }
    }

    if (tmp_group.size() >= 2)
        MakeRotGroup(tmp_group);
}

void LRModel::MakeRotGroup(std::vector <LRSensor> &ring)
{
    int gid = CreateGroup();
    std::sort (ring.begin(), ring.end(), LRSensor::Compare_Phi);
    double phi0 = ring[0].GetPhi();
    Group[gid].x = ring[0].x;
    Group[gid].y = ring[0].y;
    AddToGroup(ring[0].id, gid, 0);

    for (unsigned int i=1; i<ring.size(); i++)
        AddToGroup(ring[i].id, gid, new RotateLRF(phi0 - ring[i].GetPhi()));
}

void LRModel::MakeGroupsByTransform(std::vector <Transform*> vtr)
{
    // using local copy (ls) of the Sensor vector to simplify the housekeeping
    // shallow copy is OK as only the sensor positions are used
    std::vector <LRSensor> ls = Sensor;
    std::sort (ls.begin(), ls.end(), LRSensor::Compare_R);
    while (ls.size() > 1) {
        int gid = CreateGroup();
        AddToGroup(ls[0].id, gid, 0);
        double x0 = Group[gid].x = ls[0].x;
        double y0 = Group[gid].y = ls[0].y;
        for (Transform *tr : vtr) {
            for (unsigned int i=1; i<ls.size(); i++) {
                double x1 = ls[i].x;
                double y1 = ls[i].y;
                double z1 = 0.;
                tr->DoTransform(&x1, &y1, &z1); // must be forward transform
                if (fabs(x1-x0) < tol && fabs(y1-y0) < tol) {
                    AddToGroup(ls[i].id, gid, tr->clone());
                    ls.erase(ls.begin() + i);
                    break;
                }
            }
        }
        ls.erase(ls.begin());
        if (GetGroupMembersCount(gid) < 2)
            DissolveGroup(gid);
    }
}

std::vector <Transform*> LRModel::MakeVtrRectangle()
{
    std::vector <Transform*> vtr;
    vtr.push_back(new ReflectLRF(0));
    vtr.push_back(new ReflectLRF(M_PI/2));
    vtr.push_back(new RotateLRF(M_PI));
    return vtr;
}

std::vector <Transform*> LRModel::MakeVtrSquare()
{
    std::vector <Transform*> vtr;
    for (int i=0; i<4; i++)
        vtr.push_back(new ReflectLRF(i*M_PI/4));
    for (int i=1; i<4; i++)
        vtr.push_back(new RotateLRF(i*M_PI/2));
    return vtr;
}

std::vector <Transform*> LRModel::MakeVtrHexagon()
{
    std::vector <Transform*> vtr;
    for (int i=0; i<6; i++)
        vtr.push_back(new ReflectLRF(i*M_PI/6));
    for (int i=1; i<6; i++)
        vtr.push_back(new RotateLRF(i*M_PI/3));
    return vtr;
}

std::vector <Transform*> LRModel::MakeVtrNgon(int n)
{
    std::vector <Transform*> vtr;
    for (int i=0; i<n; i++)
        vtr.push_back(new ReflectLRF(i*M_PI/n));
    for (int i=1; i<n; i++)
        vtr.push_back(new RotateLRF(i*2*M_PI/n));
    return vtr;
}

// Correction factors (light collection)
bool LRModel::SetRefPoint(double x, double y, double z)
{
    size_t len = Sensor.size();
    for (size_t id = 0; id < len; id++)
        if (!IsDisabled(id) && !InDomain(id, x, y, z)) { 
            lrm_err = "SetRefPoint: reference point is not in domain of one or more sensors";
            return false;
    } // NB: be careful with re-enabling sensors after this  

    corr.xref = x;
    corr.yref = y;
    corr.zref = z;

    return true;
}

double LRModel::GetLRFSum(double x, double y, double z, std::vector <bool> *mask)
{
    double sum(0);

    if (mask->size() == 0) {
        for (int id : GetEnabledList())
            sum += Eval(id, x, y, z);
    } else {
        if (mask->size() != GetSensorCount())
            throw std::length_error("GetLRFSum: mask length differs from the sensor count");

        for (int id : GetEnabledList())
            if (!(*mask)[id])
                sum += Eval(id, x, y, z);
    }

    return sum;
}

double LRModel::GetLRFSum(double x, double y, std::vector <bool> *mask)
{
    return GetLRFSum(x, y, 0, mask);
}

std::vector <double> LRModel::GetLRFSum(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > *mask)
{
    size_t vlen = x.size();
    if (vlen != y.size() || vlen != z.size())
        throw std::length_error("GetLRFSum: Input vectors must be of the same length");
    
    int nsensors = GetSensorCount();
    std::vector <double> lrfsum(vlen);
    std::vector <int> enabled = GetEnabledList();
    
    if (mask->size() == 0) {
        for (size_t i=0; i<vlen; i++) {
            double sum = 0;
            for (int id : enabled)
                sum += Eval(id, x[i], y[i], z[i]);
            lrfsum[i] = sum;
        }
    } else {
        if (vlen != mask->size())
            throw std::length_error("GetLRFSum: Input vectors must be of the same length with the mask");

        for (size_t i=0; i<vlen; i++) {
            if ((*mask)[i].size() != nsensors)
                throw std::length_error("GetLRFSumMask: mask length differs from the sensor count");
            double sum = 0;
            for (int id : enabled)
                if (!(*mask)[i][id])
                    sum += Eval(id, x[i], y[i], z[i]);
            lrfsum[i] = sum;
        }
    }
    return lrfsum;
}

std::vector <double> LRModel::GetLRFSum(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > *mask)
{
    if (x.size() != y.size())
        throw std::length_error("GetLRFSum: Input vectors must be of the same length");
    std::vector <double> z(x.size(), 0);
    return GetLRFSum(x, y, z, mask);
}

double LRModel::GetLRFSumList(std::vector <int> &list, double x, double y, double z)
{
    size_t len = Sensor.size();
    double sum(0);

    for (int id : list)
        if (!IsDisabled(id) && id >= 0 && id < len)
            sum += Eval(id, x, y, z);

    return sum;    
}

double LRModel::GetCorrFactor(double x, double y, double z, std::vector <bool> *mask)
{
    return GetLRFSum(corr.xref, corr.yref, corr.zref, mask)/GetLRFSum(x, y, z, mask);
}

double LRModel::GetCorrFactor(double x, double y, std::vector <bool> *mask)
{
    return GetCorrFactor(x, y, 0, mask);
}

std::vector <double> LRModel::GetCorrFactor(std::vector <double> &x, std::vector <double> &y, std::vector <double> &z, std::vector<std::vector <bool> > *mask)
{
    size_t vlen = x.size();
    if (vlen != y.size() || vlen != z.size())
        throw std::length_error("GetCorrFactor: Input vectors must be of the same length");

    if (mask->size() == 0) {
        std::vector <double> lrfsum = GetLRFSum(x, y, z, mask);
        double refsum = GetLRFSum(corr.xref, corr.yref, corr.zref);
        for (size_t i=0; i<vlen; i++)
            lrfsum[i] = refsum/lrfsum[i];

        return lrfsum;
    } else {
        int nsensors = GetSensorCount();
        std::vector <int> enabled = GetEnabledList();        
        std::vector <double> cf(vlen);
        for (size_t i=0; i<vlen; i++) {
            if ((*mask)[i].size() != nsensors)
                throw std::length_error("GetCorrFactorMask: mask length differs from the sensor count");
            double sum = 0;
            double refsum = 0;
            for (int id : enabled)
                if (!(*mask)[i][id]) {
                    sum += Eval(id, x[i], y[i], z[i]);
                    refsum += Eval(id, corr.xref, corr.yref, corr.zref);
                }
            cf[i] = refsum/sum;
        }
        return cf;
    }
}

std::vector <double> LRModel::GetCorrFactor(std::vector <double> &x, std::vector <double> &y, std::vector<std::vector <bool> > *mask)
{
    if (x.size() != y.size())
        throw std::length_error("GetCorrFactor: Input vectors must be of the same length");
    std::vector <double> z(x.size(), 0);
    return GetCorrFactor(x, y, z, mask);
}

double LRModel::GetCorrFactorList(std::vector <int> &list, double x, double y, double z)
{
    return GetLRFSumList(list, corr.xref, corr.yref, corr.zref)/GetLRFSumList(list, x, y, z);
}

double LRModel::GetCorrectedArea(std::vector <double> &area, std::vector <bool> &mask, double x, double y, double z)
{
    if (area.size() != mask.size())
        throw std::length_error("GetCorrectedArea: area and mask vectors must be of the same length");

    double sum(0), refsum(0), refsum_0(0);
    //std::vector <double> list;
    for (int id : GetEnabledList()) {
        if(mask[id]) 
            continue;
        refsum += Eval(id, x, y, z);
        refsum_0 += Eval(id, corr.xref, corr.yref, corr.zref);
        sum += area[id];
    }
    return sum*refsum_0/refsum;
}

std::vector <double> LRModel::GetCorrectedArea(std::vector <std::vector <double> > &area, std::vector <std::vector <bool> > &mask, std::vector <double> &x, std::vector <double> &y, std::vector <double> &z)
{
    size_t len = area.size();
    if (len != mask.size() || len != x.size() || len != y.size() || len != z.size())
        throw std::length_error("GetCorrectedArea: Input vectors must be of the same length");

    std::vector <double> corrected_areas;
    for(int evt=0; evt<len; evt++)
        corrected_areas.push_back(GetCorrectedArea(area[evt], mask[evt], x[evt], y[evt], z[evt]));

    return corrected_areas;
}

// Access to Profile Histograms
// To make it safe, we first verify that there is an LRF associated with the sensor
// if there is not, return 0 for scalar or an empty vector for vector  

std::vector <int> LRModel::GetHistBins(int id)
{  
    LRF *f = GetLRF(id);
    if (f) {
        return f->GetHist()->GetBins();
    } else {
        std::vector <int> dim;
        return dim;
    }
}

int LRModel::GetHistNdim(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetNdim() : 0;
}

int LRModel::GetHistBinsX(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetBinsX() : 0;
}

int LRModel::GetHistBinsY(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetBinsY() : 0;
}

int LRModel::GetHistBinsZ(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetBinsZ() : 0;
}

std::vector <double> LRModel::GetHistMeans(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetMeans() : empty_vector;
//    return GetLRF(id)->GetHist()->GetMeans)();
}

std::vector <double> LRModel::GetHistSigmas(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetSigmas() : empty_vector;
}

std::vector <double> LRModel::GetHistWeights(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetWeights() : empty_vector;
}

std::vector <double> LRModel::GetHistXCenters(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetXCenters() : empty_vector;
}

std::vector <double> LRModel::GetHistYCenters(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetYCenters() : empty_vector;
}

std::vector <double> LRModel::GetHistZCenters(int id)
{
    LRF *f = GetLRF(id);
    return f ? f->GetHist()->GetZCenters() : empty_vector;
}

std::vector <double> LRModel::GetHistFit(int id)
{
    LRF *f = GetLRF(id);
    if (!f)
        return empty_vector;

    std::vector <double> x = GetHistXCenters(id);
    std::vector <double> y = GetHistYCenters(id);
    std::vector <double> z = GetHistZCenters(id);

    size_t len = x.size();
    std::vector <double> lrf(len);

    for (size_t i = 0; i<len; i++)
        lrf[i] = f->evalraw(x[i], y[i], z[i]);

    return lrf;
}

// Input-output

Json::object LRModel::SensorGetJsonObject(int id) const
{
    Json::object json;
    LRSensor s = Sensor.at(id);
    json["id"] = s.id;
    json["group_id"] = s.group_id;
    json["x"] = s.x;
    json["y"] = s.y;
    json["gain"] = s.gain;
    json["disabled"] = s.disabled;
    if (s.tr) json["transform"] = s.tr->GetJsonObject();
    if (s.group_id == -1 && s.lrf) json["LRF"] = s.lrf->GetJsonObject();
    return json;
}

void LRModel::ReadSensor(const Json &json)
{
    int id = json["id"].int_value();
    LRSensor &s = Sensor.at(id);
    s.id = id;
    s.x = json["x"].number_value();
    s.y = json["y"].number_value();
    s.group_id = json["group_id"].int_value();
    s.gain = json["gain"].number_value();
    s.disabled = json["disabled"].bool_value();
    if (json["transform"].is_object())
        s.tr = Transform::Factory(json["transform"]);
    if (json["LRF"].is_object())
        s.lrf = LRF::mkFromJson(json["LRF"]);
//        s.lrf = new LRFaxial(json["LRF"]);
}

Json::object LRModel::GroupGetJsonObject(int gid) const
{
    Json::object json;
    LRGroup g = Group.at(gid);
    json["id"] = g.id;
    json["x0"] = g.x;
    json["y0"] = g.y;
    json["members"] = g.members;
    if (g.glrf) json["LRF"] = g.glrf->GetJsonObject();
    return json;
}

void LRModel::ReadGroup(const Json &json)
{
    int id = json["id"].int_value();
    LRGroup &g = Group.at(id);
    g.id = id;
    g.x = json["x0"].number_value();
    g.y = json["y0"].number_value();
    if (json["members"].is_array()) {
        Json::array members = json["members"].array_items();
        for (unsigned int i=0; i<members.size(); i++)
            g.members.insert(members[i].int_value());
    }
    if (json["LRF"].is_object())
        g.glrf = LRF::mkFromJson(json["LRF"]);
//        g.glrf = new LRFaxial(json["LRF"]);
}

void LRModel::ToJsonObject(Json_object &json) const
{
    std::vector <Json_object> sensors;
    std::vector <Json_object> groups;
    int n_sensors = (int)Sensor.size();
    int n_groups = (int)Group.size();
    json["n_sensors"] = n_sensors;
    json["n_groups"] = n_groups;

    for (int i=0; i<n_sensors; i++)
        sensors.push_back(SensorGetJsonObject(i));
    json["sensors"] = sensors;
    for (int i=0; i<n_groups; i++)
        groups.push_back(GroupGetJsonObject(i));
    json["groups"] = groups;
}

Json::object LRModel::GetJsonObject() const
{
    Json::object json;
    ToJsonObject(json);
    return json;
}

std::string LRModel::GetJsonString() const
{
    Json::object json;
    ToJsonObject(json);
    return Json(json).dump();
}

std::string LRModel::SensorGetJsonString(int id) const
{
    return Json(SensorGetJsonObject(id)).dump();
}
std::string LRModel::GroupGetJsonString(int gid) const
{
    return Json(GroupGetJsonObject(gid)).dump();    
}

LRModel::LRModel(const Json &json)
{
    int n_sensors = json["n_sensors"].int_value();
    int n_groups = json["n_groups"].int_value();
    Sensor.resize(n_sensors);
    Group.resize(n_groups);
    if (json["sensors"].is_array()) {
        Json::array sensors = json["sensors"].array_items();
        for (unsigned int i=0; i<sensors.size(); i++)
            ReadSensor(sensors[i]);
    }
    if (json["groups"].is_array()) {
        Json::array groups = json["groups"].array_items();
        for (unsigned int i=0; i<groups.size(); i++)
            ReadGroup(groups[i]);
    }
}

LRModel::LRModel(std::string json_str) : LRModel(Json::parse(json_str, json_err)) {}

// Utility
double LRModel::GetMaxR(int id, const std::vector <LRFdata> &data) const
{
    double x0 = GetX(id);
    double y0 = GetY(id);
    double maxr2 = 0.;

    for (LRFdata d : data) {
        double dx = d.x-x0;
        double dy = d.y-y0;
        maxr2 = std::max(maxr2, dx*dx + dy*dy);
    }
    return sqrt(maxr2);
}

double LRModel::GetGroupMaxR(int gid, const std::vector <LRFdata> &data) const
{
    double maxr = 0.;
    for (int id : Group.at(gid).members)
        maxr = std::max(maxr, GetMaxR(id, data));
    return maxr;
}

// ============ Gain estimator ===============

// We'll be using gain estimator to equalize the sensors in the same group 
// Need to pass an LRM to the constructor with groups already defined and LRFs assigned to them
// The constructor rebuilds similar LRM, but with individual sensors (however, keeping
// transform of the original) so the data in their profile histograms could be compared
GainEstimator::GainEstimator(std::string lrmtxt)
{
    LRModel lrm(lrmtxt);
    Init(&lrm);
}

GainEstimator::~GainEstimator()
{
    delete M;
}

void GainEstimator::Init(LRModel *lrm)
{
    size = lrm->GetSensorCount();
    M = new LRModel(size);
    for (int i=0; i<size; i++) {        
        M->AddSensor(i, lrm->GetX(i), lrm->GetX(i));
        Transform *tr = lrm->GetTransform(i);
        if (tr)
            M->SetTransform(i, tr->clone());
        LRF *lrf = lrm->GetLRF(i);
        if (lrf) M->SetLRF(i, lrf->clone());
    }
}

bool GainEstimator::AddData(int id, const std::vector <Vec4data> &data)
{
    if (M->GetLRF(id) == nullptr)
        return false;
    return M->AddFitData(id, data);
}

// ToDo: swap id and refid!
// returns: 0 if either of LRFs does not exist, -1 if they are incompatible
double GainEstimator::GetRelativeGain(int id, int refid)
{
    LRF *lrf = M->GetLRF(id);
    LRF *ref = M->GetLRF(refid);
    if (lrf == nullptr || ref == nullptr)
        return 0;
    return lrf->GetRatio(ref);
}

std::vector <double> GainEstimator::GetRelativeGainsList(std::vector <int> ids, int refid)
{
    std::vector <double> gains;
    for (int id : ids)
        gains.push_back(GetRelativeGain(id, refid));
    return gains;
}
/*
std::vector <int> GainEstimator::GetAllSensors()
{
    std::vector <int> list;
    for (int id = 0; id < size; id++)
        if (M->GetLRF(id))
            list.push_back(id);
    return list;
}
*/
