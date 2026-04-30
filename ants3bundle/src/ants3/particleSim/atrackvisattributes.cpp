#include "atrackvisattributes.h"
#include "ajsontools.h"
#include "a3global.h"

#include "TVirtualGeoTrack.h"

void ATrackAttributes::writeToJson(QJsonObject &json) const
{
    json["color"] = Color;
    json["width"] = Width;
    json["style"] = Style;
}

const QJsonObject ATrackAttributes::writeToJson() const
{
    QJsonObject json;
    writeToJson(json);
    return json;
}

void ATrackAttributes::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "color", Color);
    jstools::parseJson(json, "width", Width);
    jstools::parseJson(json, "style", Style);
}

void ATrackAttributes::setTrackAttributes(TVirtualGeoTrack *track) const
{
    track->SetLineColor(Color);
    track->SetLineWidth(Width);
    track->SetLineStyle(Style);
}

void ATrackAttributes::reset()
{
    Color = 7;
    Width = 1;
    Style = 1;
}

// ---

ATrackVisAttributes & ATrackVisAttributes::getInstance()
{
    static ATrackVisAttributes instance;
    return instance;
}

ATrackVisAttributes::ATrackVisAttributes()
{
    clearParticleProps();
    clearPhotonProps();

    const A3Global & GlobSet = A3Global::getConstInstance();
    if (GlobSet.TrackVisAttributes.isEmpty())
    {
        DefinedAttributes["proton"]  = ATrackAttributes(2,1,1);
        DefinedAttributes["e-"]      = ATrackAttributes(9,1,1);
        DefinedAttributes["e+"]      = ATrackAttributes(6,1,1);
        DefinedAttributes["gamma"]   = ATrackAttributes(1,1,1);
        DefinedAttributes["neutron"] = ATrackAttributes(3,1,1);
        writeToJson(A3Global::getInstance().TrackVisAttributes);
    }
    else readFromJson(GlobSet.TrackVisAttributes);
}

ATrackAttributes * ATrackVisAttributes::getAttributesForParticle(const QString & name)
{
    auto it = DefinedAttributes.find(name);
    if (it == DefinedAttributes.end()) return nullptr;

    else return &(it->second);
}

const QStringList ATrackVisAttributes::getDefinedParticles() const
{
    QStringList sl;
    for (auto const & it : DefinedAttributes) sl << it.first;
    return sl;
}

void ATrackVisAttributes::defineAttributesForParticle(const QString & name, const ATrackAttributes & att)
{
    DefinedAttributes[name] = att;
}

void ATrackVisAttributes::writeToJson(QJsonObject &json) const
{
    json["DefaultAttributes"] = DefaultAttributes.writeToJson();

    QJsonArray ar;
    for (auto const & it : DefinedAttributes)
    {
        QJsonArray el;
            el.push_back(it.first);
                QJsonObject js;
                it.second.writeToJson(js);
            el.push_back(js);
        ar.push_back(el);
    }
    json["CustomAttribtes"] = ar;

    json["PrimaryPhotonTracks"] = PrimaryPhotonTracks.writeToJson();
    json["SecondaryPhotonTracks"] = SecondaryPhotonTracks.writeToJson();
    json["HitSensorPhotonTracks"] = HitSensorPhotonTracks.writeToJson();
    json["UseHitSensorAttributes"] = UseHitSensorAttributes;
}

void ATrackVisAttributes::readFromJson(const QJsonObject & json)
{
    clearParticleProps();
    clearPhotonProps();
    if (json.isEmpty()) return;

    {
        QJsonObject js;
        jstools::parseJson(json, "DefaultAttributes", js);
        DefaultAttributes.readFromJson(js);
    }

    DefinedAttributes.clear();
    QJsonArray ar;
    jstools::parseJson(json, "CustomAttribtes", ar);
    for (int i = 0; i < ar.size(); i++)
    {
        QJsonArray el = ar[i].toArray();
        if (el.size() < 2) continue;

        QString pn = el[0].toString();

        QJsonObject js = el[1].toObject();
        ATrackAttributes ta;
        ta.readFromJson(js);

        DefinedAttributes[pn]= ta;
    }

    bool ok = jstools::parseJson(json, "UseHitSensorAttributes", UseHitSensorAttributes);
    if (ok)
    {
        {
            QJsonObject js;
            jstools::parseJson(json, "PrimaryPhotonTracks", js);
            PrimaryPhotonTracks.readFromJson(js);
        }

        {
            QJsonObject js;
            jstools::parseJson(json, "SecondaryPhotonTracks", js);
            SecondaryPhotonTracks.readFromJson(js);
        }

        {
            QJsonObject js;
            jstools::parseJson(json, "HitSensorPhotonTracks", js);
            HitSensorPhotonTracks.readFromJson(js);
        }
    }
}

void ATrackVisAttributes::removeCustom(const QString & name)
{
    DefinedAttributes.erase(name);
}

void ATrackVisAttributes::applyToParticleTrack(TVirtualGeoTrack *track, const QString & Particle) const
{
    auto search = DefinedAttributes.find(Particle);

    if (search == DefinedAttributes.end())
        DefaultAttributes.setTrackAttributes(track);
    else
        search->second.setTrackAttributes(track);
}

void ATrackVisAttributes::clearParticleProps()
{
    DefaultAttributes.Color = 15;
    DefaultAttributes.Width = 2;
    DefaultAttributes.Style = 1;
    DefinedAttributes.clear();
}

void ATrackVisAttributes::clearPhotonProps()
{
    PrimaryPhotonTracks   = {7, 1, 1};
    SecondaryPhotonTracks = {6, 1, 1};
    HitSensorPhotonTracks = {2, 1, 1};
    UseHitSensorAttributes = true;
}
