#include "lrfio.h"

#ifdef BSIO
#include "json11.hpp"
Json::object LRF_IO::GetJsonObject() const
{
    Json::object json;
    ToJsonObject(json);
    return json;
}

std::string LRF_IO::GetJsonString() const
{
    Json::object json;
    ToJsonObject(json);
    return Json(json).dump();
}
#endif

LRF_IO::LRF_IO()
{

}
