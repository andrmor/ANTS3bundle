#include "alightresponsehub.h"
#include "alrfplotter.h"
#include "lrmodel.h"
#include "ajsontools.h"

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
    if (!Model) return "Failed to generate light response model from the provided text";
    //if (Model->isValid()) return "Failed to generate light response model from the provided text";
    return "";
}

void ALightResponseHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;
        QString txt;
        if (Model) txt = Model->GetJsonString().data();
        js["Model"] = txt;
    json["LightResponse"] = js;
}

QString ALightResponseHub::readFromJson(const QJsonObject & json)
{
    QJsonObject js;
    if (jstools::parseJson(json, "LightResponse", js))
    {
        clearModel();
        QString txt;
        jstools::parseJson(js, "Model", txt);
        if (!txt.isEmpty())
        {
            QString err = makeModel(txt);
            if (!err.isEmpty()) return err;
        }
    }
    return "";
}

