#include "lrf.h"
#include "json11.hpp"

#include "lrfaxial.h"
#include "lrfaxial3d.h"
#include "lrfxy.h"
#include "lrfxyz.h"
#include "lrfcomp.h"

std::string LRF::gjson_err("no error");

LRF* LRF::mkFromJson(const Json &json){
    if (!json["type"].is_string()) {
        gjson_err = std::string("type missing or isn't a string");
        return nullptr;
    }
    
    LRF* lrf = nullptr;
    std::string type = json["type"].string_value();

    if (type == "Axial")
        lrf = new LRFaxial(json);
    else if (type == "Axial3D")
        lrf = new LRFaxial3d(json);
    else if (type == "XY")
        lrf = new LRFxy(json);
    else if (type == "XYZ")
        lrf = new LRFxyz(json);
    else if (type == "Composite")
        lrf = new LRFcomp(json);    
    else {
        gjson_err = std::string("unknown type");
        return nullptr; // unknown type
    }

    if (lrf->isValid())
        return lrf;
    else {
        gjson_err = lrf->json_err;
        delete lrf;
        return nullptr;
    }    
}

// from reference to a string
LRF* LRF::mkFromJson(std::string &json_str) {
    return mkFromJson(Json::parse(json_str, gjson_err));
}

// from rvalue reference to a string
LRF* LRF::mkFromJson(std::string &&json_str) {
    return mkFromJson(Json::parse(json_str, gjson_err));
}
