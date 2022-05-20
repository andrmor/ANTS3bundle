#include "ajsontools.h"

#include <QJsonDocument>
#include <QDebug>
#include <QFile>

#include <cmath>

bool jstools::saveJsonToFile(const QJsonObject &json, const QString &fileName)
{
    QJsonDocument saveDoc(json);
    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) return false;

    saveFile.write(saveDoc.toJson());
    saveFile.close();
    return true;
}

bool jstools::saveJsonArrayToFile(const QJsonArray & jsar, const QString & fileName)
{
    QJsonDocument saveDoc(jsar);
    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) return false;

    saveFile.write(saveDoc.toJson());
    saveFile.close();
    return true;
}

bool jstools::loadJsonFromFile(QJsonObject &json, const QString &fileName)
{
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) return false;

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    json = loadDoc.object();
    loadFile.close();
    return true;
}

bool jstools::loadJsonArrayFromFile(QJsonArray &ar, const QString &fileName)
{
    QFile loadFile(fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) return false;

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    ar = loadDoc.array();
    loadFile.close();
    return true;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, bool &var)
{
    if (json.contains(key))
    {
        var = json[key].toBool();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, int &var)
{
    if (json.contains(key))
    {
        //var = json[key].toInt();
        double val = json[key].toDouble();
        var = std::round(val);
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, qint64 &var)
{
    if (json.contains(key))
    {
        double val = json[key].toDouble();
        var = val;
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, double &var)
{
    if (json.contains(key))
    {
        var = json[key].toDouble();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, long &var)
{
    if (json.contains(key))
    {
        var = json[key].toDouble();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, float &var)
{
    if (json.contains(key))
    {
        var = json[key].toDouble();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, QString &var)
{
    if (json.contains(key))
    {
        var = json[key].toString();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, std::string &var)
{
    QString tmp;
    bool ok = parseJson(json, key, tmp);
    if (!ok) return false;
    var = tmp.toLatin1().data();
    return true;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, QJsonArray &var)
{
    if (json.contains(key))
    {
        var = json[key].toArray();
        return true;
    }
    else return false;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, QJsonObject &obj)
{
    if (json.contains(key))
    {
        obj = json[key].toObject();
        return true;
    }
    else return false;
}

void jstools::writeDPairVectorToArray(const std::vector<std::pair<double,double> > & vec, QJsonArray & ar)
{
    for (auto const & pair : vec)
    {
        QJsonArray el;
            el.append(pair.first);
            el.append(pair.second);
        ar.push_back(el);
    }
}

bool jstools::readDPairVectorFromArray(const QJsonArray & ar, std::vector<std::pair<double, double>> & vec)
{
    vec.clear();

    const int size = ar.size();
    for (int i = 0; i < size; i++)
    {
        if ( !ar[i].isArray() ) return false;

        const QJsonArray jar = ar.at(i).toArray();
        if (jar.size() < 2) return false;

        vec.emplace_back(jar[0].toDouble(), jar[1].toDouble());
    }
    return true;
}

void jstools::writeDVectorOfVectorsToArray(const std::vector<std::vector<double> > & vec, QJsonArray & ar)
{
    for (auto const & v1 : vec)
    {
        QJsonArray el;
            for (auto const & v : v1) el.append(v);
        ar.push_back(el);
    }
}

bool jstools::readDVectorOfVectorsFromArray(const QJsonArray & ar, std::vector<std::vector<double> > & vec)
{
    vec.clear();

    const int size1 = ar.size();
    vec.reserve(size1);
    for (int i1 = 0; i1 < size1; i1++)
    {
        if ( !ar[i1].isArray() ) return false;
        const QJsonArray jar = ar[i1].toArray();

        std::vector<double> el(jar.size());
            for (int i2 = 0; i2 < jar.size(); i2++) el[i2] = jar[i2].toDouble();
        vec.push_back(el);
    }
    return true;
}

QJsonObject jstools::strToJson(const QString &s)
{
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
    return doc.object();
}

QString jstools::jsonToString(const QJsonObject &json)
{
    QJsonDocument doc(json);
    QString s( doc.toJson(QJsonDocument::Compact) );
    return s;
}

QString jstools::jsonArrayToString(const QJsonArray & ar)
{
    QJsonDocument doc;
    doc.setArray(ar);
    return doc.toJson().simplified();
}

QJsonArray jstools::vectorQStringsToJsonArray(const std::vector<QString> & vec)
{
    QJsonArray ar;
    for (const QString & s : vec) ar.append(s);
    return ar;
}

bool jstools::parseJson(const QJsonObject &json, const QString &key, std::vector<QString> &vec)
{
    if (json.contains(key))
    {
        QJsonArray ar = json[key].toArray();
        vec.clear();
        vec.reserve(ar.size());
        for (int i=0; i<ar.size(); i++)
            vec.push_back(ar[i].toString());
        return true;
    }
    else return false;
}
