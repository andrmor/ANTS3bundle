#ifndef ACALSETTINGS_H
#define ACALSETTINGS_H

#include <string>
#include <vector>
#include <array>

#ifdef JSON11
    #include "js11tools.hh"
#else
    #include <QString>
    class QJsonObject;
#endif

class ACalorimeterProperties
{
public:
    ACalorimeterProperties(){}
    ACalorimeterProperties(const std::array<double, 3> & origin, const std::array<double, 3> & step, const std::array<int, 3> & bins) :
        Origin(origin), Step(step), Bins(bins) {}

    bool operator==(const ACalorimeterProperties & other) const;
    bool operator!=(const ACalorimeterProperties & other) const;

    int getNumDimensions() const;

    bool isAxisOff(int index) const;

#ifdef JSON11
    void writeToJson(json11::Json::object & json) const;
    void readFromJson(const json11::Json::object & json); // !!!*** Error control!
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void toStrings(QString & originRet, QString & stepRet, QString & binsRet, bool useStringValues) const;
#endif

    void copyDepoDoseProperties(const ACalorimeterProperties & other);
    void copyEventDepoProperties(const ACalorimeterProperties & other);

    bool isSameDepoDoseProperties(const ACalorimeterProperties & other) const;
    bool isSameEventDepoProperties(const ACalorimeterProperties & other) const;

    enum EDataType {Energy, Dose};

    EDataType DataType = Energy;
    bool RandomizeBin = false;
    std::array<double, 3> Origin = {-5, -5, -5};
    std::array<double, 3> Step   = { 1,  1,  1};
    std::array<int,    3> Bins   = {10, 10,  10};

    bool CollectDepoOverEvent = false;
    int EventDepoBins = 100;
    double EventDepoFrom = 0.1;
    double EventDepoTo = 2.0;

#ifndef JSON11
    //ants3 side: text fields to be used with Geo Constants
    std::array<QString, 3> strOrigin = {"", "", ""};
    std::array<QString, 3> strStep   = {"", "", ""};
    std::array<QString, 3> strBins   = {"", "", ""};
    QString strEventDepoBins, strEventDepoFrom, strEventDepoTo;
#endif
};

class ACalSetRecord
{
public:
    std::string Name;
    //std::string Particle;
    int Index;
    ACalorimeterProperties Properties;

#ifdef JSON11
    void readFromJson(const json11::Json::object & json); // !!!*** add error control
#else
    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
#endif
};

class ACalSettings
{
public:
    bool Enabled         = false;
    std::string FileName = "Calorimeters.json";

#ifdef JSON11
    void readFromJson(const json11::Json::object & json);
#else
    void writeToJson(QJsonObject & json, bool includeG4ants3Set) const;
    void readFromJson(const QJsonObject & json);
    void initFromHub();
#endif

    std::vector<ACalSetRecord> Calorimeters;

    void clear();
};

#endif // ACALSETTINGS_H
