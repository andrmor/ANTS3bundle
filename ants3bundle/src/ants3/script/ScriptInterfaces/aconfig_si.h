#ifndef ACONFIG_SI_H
#define ACONFIG_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>

class AConfig;
class QJsonObject;
class QJsonValue;

// !!!*** implement use of ErrorHub

class AConfig_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AConfig_SI();

    AScriptInterface * cloneBase() const {return new AConfig_SI();}

public slots:
    bool        load(QString FileName);
    bool        save(QString FileName);

    QVariantMap getConfig() const;
    bool        setConfig(QVariantMap ConfigObject);

    void        exportToGDML(QString FileName);  // !!!*** to geometry?
    void        exportToROOT(QString FileName);  // !!!*** to geometry?

    bool        replace(QString Key, QVariant Value);
    QVariant    getKeyValue(QString Key);

    void        rebuildDetector();
    void        updateGui();

private:
    AConfig & Config;
    QString   LastError;

    bool modifyJsonValue(QJsonObject & obj, const QString & path, const QJsonValue & newValue);
    void find(const QJsonObject & obj, QStringList Keys, QStringList & Found, QString Path = "");
    bool keyToNameAndIndex(QString Key, QString & Name, std::vector<int> & Indexes);
    bool expandKey(QString & Key);

};

#endif // ACONFIG_SI_H
