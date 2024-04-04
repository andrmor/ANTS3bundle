#ifndef ASCRIPTEXAMPLEDATABASE_H
#define ASCRIPTEXAMPLEDATABASE_H

#include "ascriptexample.h"

#include <QString>
#include <QVector>
#include <QStringList>

class AScriptExampleDatabase
{
public:
    AScriptExampleDatabase(QString configText);

    void select(const QStringList & tags);
    void unselectAll();
    int  size() const;
    int  find(const QString & fileName) const;

    QVector<AScriptExample> Examples;  // to std::vector
    QStringList Tags;
};

#endif // ASCRIPTEXAMPLEDATABASE_H
