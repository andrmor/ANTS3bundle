#include "aparticletrackvisuals.h"
#include "ajsontools.h"

#include "TVirtualGeoTrack.h"

void ATrackAttributes::writeToJson(QJsonObject &json) const
{
    json["color"] = color;
    json["width"] = width;
    json["style"] = style;
}

const QJsonObject ATrackAttributes::writeToJson() const
{
    QJsonObject json;
    writeToJson(json);
    return json;
}

void ATrackAttributes::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "color", color);
    jstools::parseJson(json, "width", width);
    jstools::parseJson(json, "style", style);
}

void ATrackAttributes::setTrackAttributes(TVirtualGeoTrack *track) const
{
    track->SetLineColor(color);
    track->SetLineWidth(width);
    track->SetLineStyle(style);
}

void ATrackAttributes::reset()
{
    color = 7;
    width = 1;
    style = 1;
}

// ---

AParticleTrackVisuals::AParticleTrackVisuals()
{
    clear();
}

void AParticleTrackVisuals::writeToJson(QJsonObject &json) const
{
    json["DefaultAttributes"] = DefaultAttributes.writeToJson();

    QJsonArray ar;
    for (auto c : DefaultColors) ar << c;
    json["DefaultColors"] = ar;

    ar = QJsonArray();
    for (auto const & x : CustomAttributes)
    {
        QJsonArray el;
            el.push_back(x.first);
                QJsonObject js;
                x.second.writeToJson(js);
            el.push_back(js);
        ar.push_back(el);
    }
    json["CustomAttribtes"] = ar;
}

void AParticleTrackVisuals::readFromJson(const QJsonObject &json)
{
    clear();
    if (json.isEmpty()) return;

    QJsonObject js;
    jstools::parseJson(json, "DefaultAttributes", js);
    DefaultAttributes.readFromJson(js);

    QJsonArray ar;
    jstools::parseJson(json, "DefaultColors", ar);
    DefaultColors.clear();
    for (int i=0; i<ar.size(); i++)
        DefaultColors.push_back( ar.at(i).toInt(1) );

    CustomAttributes.clear();
    ar = QJsonArray();
    jstools::parseJson(json, "CustomAttribtes", ar);
    for (int i=0; i<ar.size(); i++)
    {
        QJsonArray el = ar[i].toArray();
        if (el.size() < 2) continue;  // !!!*** error report?

        QString pn = el[0].toString();

        QJsonObject js = el[1].toObject();
        ATrackAttributes ta;
        ta.readFromJson(js);

        CustomAttributes[pn]= ta;
    }
}

void AParticleTrackVisuals::applyToParticleTrack(TVirtualGeoTrack *track, const QString & Particle) const
{
    auto search = CustomAttributes.find(Particle);

    if (search == CustomAttributes.end())
        DefaultAttributes.setTrackAttributes(track);
    else
        search->second.setTrackAttributes(track);

//    if ( ParticleId >=0 && ParticleId < DefaultColors.size())
//        track->SetLineColor( DefaultColors.at(ParticleId) );
}

void AParticleTrackVisuals::clear()
{
    DefaultAttributes.color = 15;
    DefaultAttributes.width = 2;
    DefaultColors = { 1, 2, 3, 4, 6, 7, 8, 9, 28, 30, 36, 38, 39, 40, 46, 49};
    CustomAttributes.clear();
}
