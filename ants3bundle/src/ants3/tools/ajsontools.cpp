#include "ajsontools.h"

#include <QJsonDocument>
#include <QDebug>
#include <QFile>

#include <cmath>

//#include "TH1D.h"

bool jstools::saveJsonToFile(const QJsonObject &json, const QString &fileName)
{
    QJsonDocument saveDoc(json);
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

/*
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
*/

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


/*
bool jstools::writeTH1DtoJsonArr(TH1D * hist, QJsonArray &ja)
{
    for (int i=1; i<hist->GetSize(); i++)
    {
        QJsonArray el;
        el.append(hist->GetBinLowEdge(i));
        el.append(hist->GetBinContent(i));
        ja.append(el);
    }
    return true;
}
*/

/*
bool jstools::writeTwoQVectorsToJArray(const QVector<double> &x, const QVector<double> &y, QJsonArray &ar)
{
    if (x.size() != y.size())
    {
        qWarning() << "Vectors mismatch!";
        return false;
    }

    for (int i=0; i<x.size(); i++)
    {
        QJsonArray el;
        el.append(x[i]);
        el.append(y[i]);
        ar.append(el);
    }
    return true;
}

bool jstools::readTwoQVectorsFromJArray(QJsonArray &ar, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    for (int i=0; i<ar.size(); i++)
    {
        if ( !ar.at(i).isArray() ) return false;

        QJsonArray jar = ar.at(i).toArray();
        if (jar.size() < 2) return false;

        double X = jar.at(0).toDouble();
        x.append(X);
        double Y = jar.at(1).toDouble();
        y.append(Y);
    }
    return true;
}

bool jstools::write2DQVectorToJArray(const QVector<QVector<double> > &xy, QJsonArray &ar)
{
    for (int i1=0; i1<xy.size(); i1++)
    {
        QJsonArray el;
        for (int i2=0; i2<xy[i1].size(); i2++) el.append(xy[i1][i2]);
        ar.append(el);
    }
    return true;
}

void jstools::read2DQVectorFromJArray(QJsonArray &ar, QVector<QVector<double> > &xy)
{
    xy.resize(ar.size());
    for (int i1=0; i1<xy.size(); i1++)
    {
        QJsonArray el = ar[i1].toArray();
        for (int i2=0; i2<el.size(); i2++)
            xy[i1].append( el[i2].toDouble());
    }
}
*/

/*
bool jstools::isContainAllKeys(QJsonObject json, QStringList keys)
{
    for (QString key : keys)
        if (!json.contains(key)) return false;
    return true;
}
*/

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
