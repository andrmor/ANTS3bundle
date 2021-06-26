#include "a3config.h"
#include "ajsontools.h"

A3Config & A3Config::getInstance()
{
    static A3Config instance;
    return instance;
}

A3Config::A3Config()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

bool A3Config::saveConfig()
{
    return jstools::saveJsonToFile(JSON, ConfigFileName);
}

bool A3Config::loadConfig()
{
    return jstools::loadJsonFromFile(JSON, ConfigFileName);
}
