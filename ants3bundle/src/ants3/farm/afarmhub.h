#ifndef AFARMHUB_H
#define AFARMHUB_H

#include <vector>

class A3FarmNodeRecord;
class QString;

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
    const std::vector<A3FarmNodeRecord*> & getNodes() const {return FarmNodes;}

    double TimeoutMs    = 10000;

    void addNewNode();
    bool addNode(const QString & name, const QString & ip, int port, int maxProcesses, double speedFactor);

    bool removeNode(int index);
    void clearNodes();

    void checkFarmStatus();

private:
    std::vector<A3FarmNodeRecord*> FarmNodes;

    bool isIPandPortAlreadyExist(const QString & address, int port) const;

};

#endif // AFARMHUB_H
