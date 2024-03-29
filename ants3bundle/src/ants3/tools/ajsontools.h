#ifndef AJSONTOOLS_H
#define AJSONTOOLS_H

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include <vector>
#include <string>

namespace jstools
{
bool        saveJsonToFile(const QJsonObject &json, const QString& fileName);
bool        saveJsonArrayToFile(const QJsonArray & jsar, const QString& fileName);
bool        loadJsonFromFile(QJsonObject &json, const QString& fileName);
bool        loadJsonArrayFromFile(QJsonArray &ar, const QString& fileName);

QJsonObject strToJson(const QString& s);
QString     jsonToString(const QJsonObject& json);
QString     jsonArrayToString(const QJsonArray & ar);

bool        parseJson(const QJsonObject &json, const QString &key, bool &var);
bool        parseJson(const QJsonObject &json, const QString &key, int  &var);      //can convert double content of the key to int - uses std::round
bool        parseJson(const QJsonObject &json, const QString &key, qint64 &var);   //can convert double content of the key to int - uses std::round
bool        parseJson(const QJsonObject &json, const QString &key, double &var);
bool        parseJson(const QJsonObject &json, const QString &key, long   &var);
bool        parseJson(const QJsonObject &json, const QString &key, float &var);
bool        parseJson(const QJsonObject &json, const QString &key, QString &var);
bool        parseJson(const QJsonObject &json, const QString &key, std::string &var);
bool        parseJson(const QJsonObject &json, const QString &key, QJsonArray &var);
bool        parseJson(const QJsonObject &json, const QString &key, QJsonObject &obj);

QJsonArray  vectorQStringsToJsonArray(const std::vector<QString> & vec);
bool        parseJson(const QJsonObject & json, const QString & key, std::vector<QString> & vec);

void        arrayElementToObject(const QJsonArray & array, size_t index, QJsonObject & object);

void        writeDPairVectorToArray(const std::vector<std::pair<double, double>> & vec, QJsonArray & ar);
bool        readDPairVectorFromArray(const QJsonArray & ar, std::vector<std::pair<double, double>> & vec);

void        writeDVectorOfVectorsToArray(const std::vector<std::vector<double>> & vec, QJsonArray & ar);
bool        readDVectorOfVectorsFromArray(const QJsonArray & ar, std::vector<std::vector<double>> & vec);

}

#endif // AJSONTOOLS_H
