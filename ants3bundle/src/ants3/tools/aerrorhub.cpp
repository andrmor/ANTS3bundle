#include "aerrorhub.h"

AErrorHub &AErrorHub::getInstance()
{
    static AErrorHub instance;
    return instance;
}

void AErrorHub::clear()
{
    getInstance().Error.clear();
}

void AErrorHub::addError(const std::string & ErrorLine)
{
    if (ErrorLine.empty()) return;

    std::string & err = getInstance().Error;
    if (!err.empty()) err += '\n';

    err += ErrorLine;
}

bool AErrorHub::isError()
{
    return !getInstance().Error.empty();
}

const std::string &AErrorHub::getError()
{
    return getInstance().Error;
}
