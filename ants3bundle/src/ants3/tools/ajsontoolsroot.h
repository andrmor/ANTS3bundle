#ifndef AJSONTOOLSROOT_H
#define AJSONTOOLSROOT_H

#include <QString>
#include <QJsonObject>

class TH1D;
class TH2D;

namespace jstools
{
    bool        parseJson(const QJsonObject & json, const QString & name, TH1D* & distr);
    bool        parseJson(const QJsonObject & json, const QString & name, TH2D* & distr);

    QJsonObject regularTh1dToJson(TH1D * hist);
    QJsonObject regularTh2dToJson(TH2D * hist);

    TH1D      * jsonToRegularTh1D(const QJsonObject & json);
    TH2D      * jsonToRegularTh2D(const QJsonObject & json);
}

#endif // AJSONTOOLSROOT_H
