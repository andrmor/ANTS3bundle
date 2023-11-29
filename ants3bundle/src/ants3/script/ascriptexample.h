#ifndef ASCRIPTEXAMPLE_H
#define ASCRIPTEXAMPLE_H

#include <QString>
#include <QVector>

class AScriptExample
{
public:
    bool readFromRecord(QString text);

    QString FileName;
    QVector<QString> Tags;  // !!!*** to std::vector
    QString Description;

    bool Selected = true;

    QString ErrorString;
};

#endif // ASCRIPTEXAMPLE_H
