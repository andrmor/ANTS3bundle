#ifndef ATRACKDRAWOPTIONS_H
#define ATRACKDRAWOPTIONS_H

#include <map>
#include <vector>

#include <QString>

class QJsonObject;
class TVirtualGeoTrack;

class ATrackAttributes
{
public:
    int Color = 7;
    int Width = 1;
    int Style = 1;

    ATrackAttributes(int color = 7, int width = 1, int style = 1) : Color(color), Width(width), Style(style) {}

    void setTrackAttributes(TVirtualGeoTrack * track) const;

    void writeToJson(QJsonObject& json) const;
    const QJsonObject writeToJson() const;
    void readFromJson(const QJsonObject& json);

    void reset();
};

class AParticleTrackVisuals
{
public:
    static AParticleTrackVisuals & getInstance();

private:
    AParticleTrackVisuals();

    AParticleTrackVisuals(const AParticleTrackVisuals&)            = delete;
    AParticleTrackVisuals(AParticleTrackVisuals&&)                 = delete;
    AParticleTrackVisuals& operator=(const AParticleTrackVisuals&) = delete;
    AParticleTrackVisuals& operator=(AParticleTrackVisuals&&)      = delete;

public:
    ATrackAttributes DefaultAttributes;
    std::map<QString, ATrackAttributes> DefinedAttributes;

    ATrackAttributes * getAttributesForParticle(const QString & name); // nullptr if not yet defined
    const QStringList getDefinedParticles() const;

    void defineAttributesForParticle(const QString & name, const ATrackAttributes & att);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void removeCustom(const QString & name);

    void applyToParticleTrack(TVirtualGeoTrack * track, const QString & Particle) const;

private:
    void clear(); //clear and reset to default values
};

#endif // ATRACKDRAWOPTIONS_H
