#ifndef AINTERFACERULE_H
#define AINTERFACERULE_H

#include <QString>

class AInterfaceRule;
class APhoton;
class QJsonObject;
class ARandomHub;
class ASurfaceSettings;

//  ----  !!!  ----
// modify two static functions in the cpp file after adding a new override type!

class AInterfaceRule
{
public:
    static AInterfaceRule * interfaceRuleFactory(const QString & Model, int MatFrom, int MatTo);
    static QStringList      getAllInterfaceRuleTypes();

    enum OpticalOverrideResultEnum {NotTriggered, Absorbed, Forward, Back, _Error_}; //return status for photon tracing:
    enum ScatterStatusEnum {
        Absorption,
        SpikeReflection, LobeReflection, LambertianReflection,
        UnclassifiedReflection,
        Transmission,
        Empty, Fresnel, Error
    }; //detailed status for statistics only - used by override tester only

    AInterfaceRule(int MatFrom, int MatTo);
    virtual ~AInterfaceRule() {}

    void configureSurface(const ASurfaceSettings & surfaceSettings) {SurfaceSettings = &surfaceSettings;}

    // !!!*** to reference
    virtual OpticalOverrideResultEnum calculate(APhoton * Photon, const double * NormalVector) = 0; //unitary vectors! iWave = -1 if not wavelength-resolved

    virtual QString getType() const = 0;
    virtual QString getAbbreviation() const = 0; //for GUI: used to identify - must be short (<= 4 chars) - try to make unique
    virtual QString getReportLine() const = 0;   // for GUI: used to reports override status (try to make as short as possible)
    virtual QString getLongReportLine() const;   //for GUI: used in overlap map

    //called automatically before sim start
    virtual void initializeWaveResolved() {}  //override if override has wavelength-resolved data

    // save/load config
    virtual void writeToJson(QJsonObject & json) const;
    virtual bool readFromJson(const QJsonObject & json);

    //used by MatCollection when a material is removed
    void updateMatIndices(int iMatFrom, int iMatTo) {MatFrom = iMatFrom; MatTo = iMatTo;}

    //called on editing end (widget above) and before sim start to avoid miss-configurations
    virtual QString checkOverrideData() = 0; //cannot be const - w.resolved needs rebin

    // read-out variables for standalone checker only (not multithreaded)
    ScatterStatusEnum Status;               // type of interaction which happened - use in 1 thread only!

    //misc
    int getMaterialFrom() const {return MatFrom;}
    int getMaterialTo()   const {return MatTo;}

protected:
    ARandomHub & RandomHub;
    int MatFrom, MatTo;   // material index of material before(from) and after(to) the optical interface

    const ASurfaceSettings * SurfaceSettings = nullptr;

};

#endif // AINTERFACERULE_H
