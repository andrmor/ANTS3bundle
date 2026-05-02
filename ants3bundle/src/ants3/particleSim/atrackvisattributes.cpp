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

ATrackVisAttributes::ATrackVisAttributes()
{
    clearParticleProps();
    clearPhotonProps();
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

void ATrackVisAttributes::writeToJson(QJsonObject & json) const
{
    writeToJson_particles(json);
    writeToJson_photons(json);
}

void ATrackVisAttributes::writeToJson_particles(QJsonObject & json) const
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
}

void ATrackVisAttributes::writeToJson_photons(QJsonObject & json) const
{
    json["PrimaryPhotonTracks"] = PrimaryPhotonTracks.writeToJson();
    json["SecondaryPhotonTracks"] = SecondaryPhotonTracks.writeToJson();
    json["HitSensorPhotonTracks"] = HitSensorPhotonTracks.writeToJson();
    json["UseHitSensorAttributes"] = UseHitSensorAttributes;
}

void ATrackVisAttributes::readFromJson(const QJsonObject & json)
{
    readFromJson_particles(json);
    readFromJson_photons(json);
}

void ATrackVisAttributes::readFromJson_particles(const QJsonObject & json)
{
    clearParticleProps();
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
}

void ATrackVisAttributes::readFromJson_photons(const QJsonObject & json)
{
    clearPhotonProps();
    if (json.isEmpty()) return;

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

void ATrackVisAttributes::applyToPhotonTrack(TVirtualGeoTrack * track, bool secondary, bool hit) const
{
    if (hit && UseHitSensorAttributes) HitSensorPhotonTracks.setTrackAttributes(track);
    else
    {
        secondary ? SecondaryPhotonTracks.setTrackAttributes(track) : PrimaryPhotonTracks.setTrackAttributes(track);
    }
}

void ATrackVisAttributes::clearParticleProps()
{
    DefaultAttributes.Color = 15;
    DefaultAttributes.Width = 2;
    DefaultAttributes.Style = 1;

    DefinedAttributes.clear();
    DefinedAttributes["proton"]  = ATrackAttributes(2,1,1);
    DefinedAttributes["e-"]      = ATrackAttributes(9,1,1);
    DefinedAttributes["e+"]      = ATrackAttributes(6,1,1);
    DefinedAttributes["gamma"]   = ATrackAttributes(1,1,1);
    DefinedAttributes["neutron"] = ATrackAttributes(3,1,1);
}

void ATrackVisAttributes::clearPhotonProps()
{
    PrimaryPhotonTracks   = {7, 1, 1};
    SecondaryPhotonTracks = {6, 1, 1};
    HitSensorPhotonTracks = {2, 1, 1};
    UseHitSensorAttributes = true;
}

void ATrackVisAttributes::importParticleAttributes(ATrackVisAttributes & fromOther)
{
    DefaultAttributes = fromOther.DefaultAttributes;
    DefinedAttributes = fromOther.DefinedAttributes;
}

void ATrackVisAttributes::importAndMergeParticleAttributes(ATrackVisAttributes & fromOther)
{
    DefaultAttributes = fromOther.DefaultAttributes;

    for (auto const & pair : fromOther.DefinedAttributes)
        DefinedAttributes[pair.first] = pair.second;
}

void ATrackVisAttributes::importPhotonAttributes(ATrackVisAttributes & fromOther)
{
    PrimaryPhotonTracks = fromOther.PrimaryPhotonTracks;
    SecondaryPhotonTracks = fromOther.SecondaryPhotonTracks;
    HitSensorPhotonTracks = fromOther.HitSensorPhotonTracks;
    UseHitSensorAttributes = fromOther.UseHitSensorAttributes;
}
