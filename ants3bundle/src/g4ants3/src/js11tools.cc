#include "js11tools.hh"

bool jstools::contains(const json11::Json & json, const std::string & key)
{
    if (!json.is_object()) return false;
    return (json.object_items().count(key) > 0);
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, bool & var)
{
    if (!contains(json, key)) return false;
    if (!json[key].is_bool()) return false;
    var = json[key].bool_value();
    return true;
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, int & var)
{
    if (!contains(json, key))   return false;
    if (!json[key].is_number()) return false;
    var = json[key].int_value();
    return true;
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, double & var)
{
    if (!contains(json, key))   return false;
    if (!json[key].is_number()) return false;
    var = json[key].number_value();
    return true;
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, std::string & var)
{
    if (!contains(json, key))   return false;
    if (!json[key].is_string()) return false;
    var = json[key].string_value();
    return true;
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, json11::Json::array & var)
{
    if (!contains(json, key))  return false;
    if (!json[key].is_array()) return false;
    var = json[key].array_items();
    return true;
}

bool jstools::parseJson(const json11::Json & json, const std::string & key, json11::Json::object & var)
{
    if (!contains(json, key))   return false;
    if (!json[key].is_object()) return false;
    var = json[key].object_items();
    return true;
}
