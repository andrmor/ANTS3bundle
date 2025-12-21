#include "alightresponsehub.h"
#include "alrfplotter.h"
#include "lrmodel.h"

ALightResponseHub::ALightResponseHub() :
    LrfPlotter(new ALrfPlotter()) {}

ALightResponseHub & ALightResponseHub::getInstance()
{
    static ALightResponseHub instance;
    return instance;
}

const ALightResponseHub &ALightResponseHub::getConstInstance()
{
    return ALightResponseHub::getInstance();
}

void ALightResponseHub::clearModel()
{
    delete Model; Model = nullptr;
}

QString ALightResponseHub::makeModel(const QString & text)
{
    delete Model; Model = nullptr;
    Model = new LRModel(text.toLatin1().data());
    if (!Model) return "Failed to generate the model";
    //if (Model->isValid()) return "Failed to generate model from the provided text";
    return "";
}

