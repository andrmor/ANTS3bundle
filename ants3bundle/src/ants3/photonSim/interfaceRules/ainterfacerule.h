#ifndef AINTERFACERULE_H
#define AINTERFACERULE_H

#include <QString>

class AInterfaceRule;
class APhoton;
class QJsonObject;
class ATracerStateful;

#ifdef GUI
class GraphWindowClass;
class QWidget;
#endif

//  ----  !!!  ----
// modify these two functions if you want to register a new override type
AInterfaceRule * interfaceRuleFactory(const QString & model, int MatFrom, int MatTo);
QStringList      getAllInterfaceRuleTypes();


class AInterfaceRule
{
public:
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

    virtual OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) = 0; //unitary vectors! iWave = -1 if not wavelength-resolved

    virtual QString getType() const = 0;
    virtual QString getAbbreviation() const = 0; //for GUI: used to identify - must be short (<= 4 chars) - try to make unique
    virtual QString getReportLine() const = 0; // for GUI: used to reports override status (try to make as short as possible)
    virtual QString getLongReportLine() const; //for GUI: used in overlap map

    //called automatically before sim start
    virtual void initializeWaveResolved() {}  //override if override has wavelength-resolved data

    // save/load config
    virtual void writeToJson(QJsonObject & json) const;
    virtual bool readFromJson(const QJsonObject & json);

    //used by MatCollection when a material is removed
    void updateMatIndices(int iMatFrom, int iMatTo) {MatFrom = iMatFrom; MatTo = iMatTo;}

#ifdef GUI
    // returns a GUI widget to show / edit parameters
    virtual QWidget* getEditWidget(QWidget* caller, GraphWindowClass* GraphWindow);
#endif

    //called on editing end (widget above) and before sim start to avoid miss-configurations
    virtual QString checkOverrideData() = 0; //cannot be const - w.resolved needs rebin

    // read-out variables for standalone checker only (not multithreaded)
    ScatterStatusEnum Status;               // type of interaction which happened - use in 1 thread only!

    //misc
    int getMaterialTo() const {return MatTo;}

protected:
    int MatFrom, MatTo;   // material index of material before(from) and after(to) the optical interface
};

#endif // AINTERFACERULE_H
