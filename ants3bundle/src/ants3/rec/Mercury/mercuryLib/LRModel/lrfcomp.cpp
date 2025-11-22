#include "lrfcomp.h"
#include "json11.hpp"
#include "profileHist.h"

#include <stdexcept>

LRFcomp* LRFcomp::clone() const 
{ 
    LRFcomp *copy = new LRFcomp(*this);
    for (size_t i=0; i<GetCount(); i++)
        copy->AddLayer(this->GetLayer(i));
    return copy;
}

LRFcomp::LRFcomp(const Json &json)
{
    if (json["layers"].is_array()) {
        Json::array lyrs = json["layers"].array_items();
        for (size_t i=0; i<lyrs.size(); i++) {
            LRF* tmp = mkFromJson(lyrs[i]);
            if (tmp)
                AddLayer(tmp);
            else
                throw std::runtime_error(std::string("LRFComp: Bad JSON"));
            // can be changed to a single line:    
            // AddLayer(mkFromJson(lyrs[i]));
            // after throws added to mkFromJson() in lrf.cpp
        }
    }

    valid = true;
}

LRFcomp::LRFcomp(std::string &json_str) : LRFcomp(Json::parse(json_str, json_err)) {}

LRFcomp::~LRFcomp()
{
    for (size_t i=0; i<GetCount(); i++)
        delete GetLayer(i);
}

bool LRFcomp::CheckNotEmpty()
{
    return stack.size() > 0;
}

bool LRFcomp::SetActiveLayer(int id)
{
    if ((id > 0) & (id < stack.size())) {
        active_layer = id;
        return true;
    } else {
        return false;
    }
}

double LRFcomp::evalActive(double x, double y, double z) const
{
    if (active_layer < 0)
        return 0;
    return GetLayer(active_layer)->eval(x, y, z);
}

LRF* LRFcomp::GetLayer(int id) const
{
    if ((id >= 0) & (id < stack.size())) {
        return stack[id];
    } else {
        throw std::length_error("LRFComp: Requested layer" + std::to_string(id) + "is out of range");
    }
}

void LRFcomp::AddLayer(LRF* lrf)
// add a new layer and make it active
{
    stack.push_back(lrf->clone());
    active_layer = stack.size() - 1;
}

bool LRFcomp::isValid() const
{
    if (!valid)
        return false;

    for (size_t i=0; i<GetCount(); i++)
        if (!GetLayer(i)->isValid())
            return false;
    
    return true;
}

/* alternative version -- slightly shorter but maybe less clear
bool LRFcomp::isValid() const
{
    bool v = valid;
    for (size_t i=0; i<GetCount(); i++)
        v = v && GetLayer(i)->isValid(); 
    
    return v;
}
*/

bool LRFcomp::inDomain(double x, double y, double z) const
// for now true is returned if any of the components responds true
{
    for (size_t i=0; i<GetCount(); i++)
        if (!GetLayer(i)->inDomain(x, y, z))
            return true;
    return false;
}

double LRFcomp::getRmax() const
{
    double r = 0.;
    for (size_t i=0; i<GetCount(); i++)
        r = std::max(r, GetLayer(i)->getRmax());
    return r;
}

double LRFcomp::eval(double x, double y, double z) const
{
    double val = 0.;
    for (size_t i=0; i<GetCount(); i++)
        val += GetLayer(i)->eval(x, y, z);
    return val;
}

double LRFcomp::evalDrvX(double x, double y, double z) const
{
    double val = 0.;
    for (size_t i=0; i<GetCount(); i++)
        val += GetLayer(i)->evalDrvX(x, y, z);
    return val;
}

double LRFcomp::evalDrvY(double x, double y, double z) const
{
    double val = 0.;
    for (size_t i=0; i<GetCount(); i++)
        val += GetLayer(i)->evalDrvY(x, y, z);
    return val;
}

bool LRFcomp::fitData(const std::vector <LRFdata> &data)
{
    return GetLayer(active_layer)->fitData(data);
}

bool LRFcomp::addData(const std::vector <LRFdata> &data)
{
    return GetLayer(active_layer)->addData(data);
}

bool LRFcomp::doFit()
{
    return GetLayer(active_layer)->doFit();
}

void LRFcomp::clearData()
{
    if (active_layer >= 0)
        GetLayer(active_layer)->clearData();
}

void LRFcomp::ToJsonObject(Json_object &json) const
{
    json["type"] = std::string(type());

    std::vector <Json_object> lyrs;
    for (size_t i=0; i<GetCount(); i++)
        lyrs.push_back(GetLayer(i)->GetJsonObject());   
    if (lyrs.size() > 0)
        json["layers"] = lyrs;
}

ProfileHist *LRFcomp::GetHist() 
{
    if (active_layer < 0)
        return nullptr;
    return GetLayer(active_layer)->GetHist();
}

// ToDo: think if it makes sense to have this function for composite LRFs
// If not, revoke pure virtual status
double LRFcomp::GetRatio(LRF* other_base) const
{
    return -1; // for now just report failure
}

