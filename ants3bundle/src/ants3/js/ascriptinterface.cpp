#include "ascriptinterface.h"

const QString &AScriptInterface::getMethodHelp(const QString & method) const
{
    auto it = Help.find(method);
    if (it == Help.end()) return NoHelp;
    return it->second;
}
