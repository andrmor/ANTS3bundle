#ifndef ALIGHTRESPONSEHUB_H
#define ALIGHTRESPONSEHUB_H

#include <QObject>

class LRModel;
class ALrfPlotter;;

class ALightResponseHub : public QObject
{
    Q_OBJECT

public:
    static ALightResponseHub & getInstance();
    static const ALightResponseHub & getConstInstance();

private:
    ALightResponseHub();
    ~ALightResponseHub(){}

    ALightResponseHub(const ALightResponseHub&)            = delete;
    ALightResponseHub(ALightResponseHub&&)                 = delete;
    ALightResponseHub& operator=(const ALightResponseHub&) = delete;
    ALightResponseHub& operator=(ALightResponseHub&&)      = delete;

public:
    LRModel     * Model = nullptr;
    ALrfPlotter * LrfPlotter = nullptr;
};

#endif // ALIGHTRESPONSEHUB_H
