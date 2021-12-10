#ifndef AJSONTOOLS_H
#define AJSONTOOLS_H

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include <vector>
#include <string>

class TH1D;
class TH2D;

namespace jstools
{
bool        saveJsonToFile(const QJsonObject &json, const QString& fileName);
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

QJsonObject regularTh1dToJson(TH1D * hist);
QJsonObject regularTh2dToJson(TH2D * hist);

bool        parseJson(const QJsonObject & json, const QString & name, TH1D* & distr);
bool        parseJson(const QJsonObject & json, const QString & name, TH2D* & distr);

TH1D      * jsonToRegularTh1D(const QJsonObject & json);
TH2D      * jsonToRegularTh2D(const QJsonObject & json);

void        writeDPairVectorToArray(const std::vector<std::pair<double, double>> & vec, QJsonArray & ar);
bool        readDPairVectorFromArray(const QJsonArray & ar, std::vector<std::pair<double, double>> & vec);

void        writeDVectorOfVectorsToArray(const std::vector<std::vector<double>> & vec, QJsonArray & ar);
bool        readDVectorOfVectorsFromArray(const QJsonArray & ar, std::vector<std::vector<double>> & vec);

}

#endif // AJSONTOOLS_H
