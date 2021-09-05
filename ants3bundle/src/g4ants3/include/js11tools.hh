#ifndef jstools_h
#define jstools_h

#include "json11.hh"

#include <string>

namespace jstools
{
    bool contains (const json11::Json & json, const std::string & key);

    bool parseJson(const json11::Json & json, const std::string & key, bool                 & var);
    bool parseJson(const json11::Json & json, const std::string & key, int                  & var);
    bool parseJson(const json11::Json & json, const std::string & key, double               & var);
    bool parseJson(const json11::Json & json, const std::string & key, std::string          & var);
    bool parseJson(const json11::Json & json, const std::string & key, json11::Json::array  & var);
    bool parseJson(const json11::Json & json, const std::string & key, json11::Json::object & var);
}

#endif // jstools_h
