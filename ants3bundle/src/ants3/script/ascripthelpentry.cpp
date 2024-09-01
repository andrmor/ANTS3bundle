#include "ascripthelpentry.h"

#include <QDebug>

AScriptHelpEntry::AScriptHelpEntry(const QString & defaultHelpText)
{
    Records.push_back({-1, defaultHelpText});
}

AScriptHelpEntry::AScriptHelpEntry(const char * defaultHelpText)
{
    Records.push_back({-1, defaultHelpText});
}

AScriptHelpEntry::AScriptHelpEntry(const std::vector<std::pair<int, QString>> &records)
{
    for (const auto & pair : records)
        Records.push_back({pair.first, pair.second});
}

AScriptHelpEntry::AScriptHelpEntry(std::initializer_list<std::pair<int, const char *>> records)
{
    for (const auto & pair : records)
        Records.push_back({pair.first, pair.second});
}

AScriptHelpEntry & AScriptHelpEntry::AScriptHelpEntry::operator=(AScriptHelpEntry && other)
{
    Records = other.Records;
    return *this;
}

bool AScriptHelpEntry::addRecord(int arguments, const QString & helpText)
{
    for (auto & pair : Records)
        if (pair.first == arguments) return false;

    Records.push_back({arguments, helpText});
    return true;
}

const QString & AScriptHelpEntry::getHelpText(int arguments) const
{
    if (Records.empty()) return NotProvided;

    int indexOfDefault = -1;
    for (size_t i = 0; i < Records.size(); i++)
    {
        if (arguments == Records[i].first) return Records[i].second;
        if (-1        == Records[i].first) indexOfDefault = i;
    }

    if (indexOfDefault == -1) return NotProvided;
    return Records[indexOfDefault].second;
}
