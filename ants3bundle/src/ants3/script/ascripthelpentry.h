#ifndef ASCRIPTHELPENTRY_H
#define ASCRIPTHELPENTRY_H

#include <vector>

#include <QString>

class AScriptHelpEntry
{
public:
    AScriptHelpEntry(const QString & defaultHelpText);
    AScriptHelpEntry(const char * defaultHelpText);
    AScriptHelpEntry(const std::vector<std::pair<int, QString>> & records);
    AScriptHelpEntry(std::initializer_list<std::pair<int, const char *>> records);
    AScriptHelpEntry & operator=(AScriptHelpEntry && other);
    AScriptHelpEntry() {}

    void operator=(const AScriptHelpEntry & other) {Records = other.Records;}

    bool addRecord(int arguments, const QString & helpText);

    const QString & getHelpText(int arguments = -1) const;

private:
    std::vector<std::pair<int, QString>> Records;

    const QString NotProvided = "Help text is missing";
};

#endif // ASCRIPTHELPENTRY_H
