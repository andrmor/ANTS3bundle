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

#ifdef QT
void AErrorHub::addQError(const QString & ErrorLine)
{
    AErrorHub::addError(std::string(ErrorLine.toLatin1().data()));
}
#endif

bool AErrorHub::isError()
{
    return !getInstance().Error.empty();
}

const std::string &AErrorHub::getError()
{
    return getInstance().Error;
}

#ifdef QT
QString AErrorHub::getQError()
{
    return QString(getInstance().Error.data());
}
#endif
