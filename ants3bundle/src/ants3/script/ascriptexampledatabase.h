#ifndef ASCRIPTEXAMPLEDATABASE_H
#define ASCRIPTEXAMPLEDATABASE_H

#include "ascriptexample.h"

#include <QString>
#include <QStringList>

#include <vector>

class AScriptExampleDatabase
{
public:
    AScriptExampleDatabase(QString configText);

    void select(const QStringList & tags);
    void unselectAll();
    int  size() const;
    int  find(const QString & fileName) const;

    std::vector<AScriptExample> Examples;
    QStringList Tags;
};

#endif // ASCRIPTEXAMPLEDATABASE_H
