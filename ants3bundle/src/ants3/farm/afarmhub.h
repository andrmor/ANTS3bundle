#ifndef AFARMHUB_H
#define AFARMHUB_H

#include <vector>

class AFarmNodeRecord;
class QString;
class QJsonObject;

class AFarmHub
{
public:
    static AFarmHub & getInstance();
    static const AFarmHub & getConstInstance();

private:
    AFarmHub(){}
    ~AFarmHub(){}

    AFarmHub(const AFarmHub&)            = delete;
    AFarmHub(AFarmHub&&)                 = delete;
    AFarmHub& operator=(const AFarmHub&) = delete;
    AFarmHub& operator=(AFarmHub&&)      = delete;

public:
    bool UseLocal       = true;
    int  LocalProcesses = 4;

    bool UseFarm        = false;
    const std::vector<AFarmNodeRecord*> & getNodes() const {return FarmNodes;}

    double TimeoutMs    = 10000;

    void addNewNode();
    bool addNode(const QString & name, const QString & ip, int port, int maxProcesses, double speedFactor);

    bool removeNode(int index);
    void clearNodes();

    void checkFarmStatus();

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

private:
    std::vector<AFarmNodeRecord*> FarmNodes;

    bool isIPandPortAlreadyExist(const QString & address, int port) const;

};

#endif // AFARMHUB_H
