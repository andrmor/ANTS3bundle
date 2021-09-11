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
    json["Particle_DefaultAttributes"] = TA_DefaultParticle.writeToJson();
    QJsonArray ar;
    for (const int& c : DefaultParticle_Colors) ar, c;
    json["Particle_DefaultColors"] = ar;
    ar = QJsonArray();
    for (ATrackAttributes* ta : CustomParticle_Attributes)
    {
        QJsonObject js;
        if (ta) ta->writeToJson(js);
        ar, js;
    }
    json["Particle_CustomAttribtes"] = ar;
}

void AParticleTrackVisuals::readFromJson(const QJsonObject &json)
{
    clear();
    if (json.isEmpty()) return;

    QJsonObject js;
    jstools::parseJson(json, "Particle_DefaultAttributes", js);
    TA_DefaultParticle.readFromJson(js);

    QJsonArray ar;
    jstools::parseJson(json, "Particle_DefaultColors", ar);
    DefaultParticle_Colors.clear();
    for (int i=0; i<ar.size(); i++)
        DefaultParticle_Colors.push_back( ar.at(i).toInt(1) );

    clearCustomParticleAttributes();
    ar = QJsonArray();
    jstools::parseJson(json, "Particle_CustomAttribtes", ar);
    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar.at(i).toObject();
        ATrackAttributes* ta = 0;
        if (!js.isEmpty())
        {
            ta = new ATrackAttributes();
            ta->readFromJson(js);
        }
        CustomParticle_Attributes, ta;
    }
}

void AParticleTrackVisuals::applyToParticleTrack(TVirtualGeoTrack *track, int ParticleId) const
{
    if ( ParticleId >= 0 && ParticleId < CustomParticle_Attributes.size() && CustomParticle_Attributes.at(ParticleId) )
    {
        //custom properties defined for this particle
        CustomParticle_Attributes.at(ParticleId)->setTrackAttributes(track);
        return;
    }

    TA_DefaultParticle.setTrackAttributes(track);

    if ( ParticleId >=0 && ParticleId < DefaultParticle_Colors.size())
        track->SetLineColor( DefaultParticle_Colors.at(ParticleId) );
}

int AParticleTrackVisuals::getParticleColor(int ParticleId) const
{
    if ( ParticleId >=0 && ParticleId < CustomParticle_Attributes.size() && CustomParticle_Attributes.at(ParticleId) )
        return CustomParticle_Attributes.at(ParticleId)->color; //custom properties defined for this particle

    if ( ParticleId >=0 && ParticleId < DefaultParticle_Colors.size()) return DefaultParticle_Colors.at(ParticleId);
    else return TA_DefaultParticle.color;
}

void AParticleTrackVisuals::clear()
{
    TA_DefaultParticle.color = 15;
    TA_DefaultParticle.width = 2;
    DefaultParticle_Colors.clear();
    DefaultParticle_Colors = {1, 2, 3, 4, 6, 7, 8, 9, 28, 30, 36, 38, 39, 40, 46, 49};

    clearCustomParticleAttributes();
}

void AParticleTrackVisuals::clearCustomParticleAttributes()
{
    for (ATrackAttributes* t : CustomParticle_Attributes)
        delete t;
    CustomParticle_Attributes.clear();
}
