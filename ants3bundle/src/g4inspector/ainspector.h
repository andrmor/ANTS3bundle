#ifndef AINSPECTOR_H
#define AINSPECTOR_H

#include <QString>

class QJsonObject;

class AInspector
{
public:
    AInspector(const QString & dir, const QString & fileName);
    ~AInspector(){}

    void start();

    QString ErrorMessage;

protected:
    QString WorkingDir;
    QString RequestFileName;

private:
    void terminate(const QString & returnMessage);
    void generateResponseFile(const QJsonObject & responseJson);
    void processRequest(const QJsonObject & json);
    void fillMaterialComposition(const QString & matName, QJsonObject & json);
};

#endif // AINSPECTOR_H
