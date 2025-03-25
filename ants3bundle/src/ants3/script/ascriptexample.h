#ifndef ASCRIPTEXAMPLE_H
#define ASCRIPTEXAMPLE_H

#include <QString>
#include <QStringList>

class AScriptExample
{
public:
    bool readFromRecord(QString text);

    QString FileName;
    QStringList Tags;
    QString Description;

    bool Selected = true;

    QString ErrorString;
};

#endif // ASCRIPTEXAMPLE_H
