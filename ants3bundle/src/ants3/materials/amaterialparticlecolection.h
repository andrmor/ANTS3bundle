#ifndef AMATERIALPARTICLECOLECTION_H
#define AMATERIALPARTICLECOLECTION_H

#include "amaterial.h"

#include <QVector>
#include <QString>
#include <QObject>
#include <QStringList>

#include <vector>

class AGeneralSimSettings;
class ATracerStateful;
class AGeoObject;

class A3MatHub : public QObject
{
    Q_OBJECT

    A3MatHub();
    ~A3MatHub();

    A3MatHub(const A3MatHub&)            = delete;
    A3MatHub(A3MatHub&&)                 = delete;
    A3MatHub& operator=(const A3MatHub&) = delete;
    A3MatHub& operator=(A3MatHub&&)      = delete;

public:
    static       A3MatHub & getInstance();
    static const A3MatHub & getConstInstance();

    AMaterial tmpMaterial; //all pointers are 0 on start -see default constructor

private:
    std::vector<AMaterial*> Materials;
    double WaveFrom  = 200.0;
    double WaveTo    = 800.0;
    double WaveStep  = 5.0;
    int    WaveNodes = 121;
    bool   WavelengthResolved = false;

public:
    //configuration
    void SetWave(bool wavelengthResolved, double waveFrom, double waveTo, double waveStep, int waveNodes);

    //hopefully we will get rid of the RandGen after update in NCrystal
    void UpdateRuntimePropertiesAndWavelengthBinning(AGeneralSimSettings *SimSet);  // !!!***
    QString CheckOverrides();

    //for script-based optical override initialization
    bool isScriptOpticalOverrideDefined() const;  // !!!***

    //info requests
    //materials
    AMaterial* operator[](int i) {return Materials[i]; } //get pointer to material with index i
    const AMaterial* operator[](int i) const {return Materials[i]; } //get pointer to material with index i
    int countMaterials() const {return Materials.size();}
    void getFirstOverridenMaterial(int &ifrom, int &ito);
    double convertWaveIndexToWavelength(int index) {return WaveFrom + WaveStep * index;}
    QString getMaterialName(int matIndex);
    const QStringList getListOfMaterialNames() const;

    //Material handling
    void AddNewMaterial(bool fSuppressChangedSignal = false);
    void AddNewMaterial(QString name, bool fSuppressChangedSignal = false);
    int  FindMaterial(const QString &name) const; //if not found, returns -1; if found, returns material index
    bool DeleteMaterial(int imat); //takes care of overrides of materials with index larger than imat!
    void UpdateWaveResolvedProperties(int imat); //updates wavelength-resolved material properties

    //tmpMaterial - related
    void ClearTmpMaterial(); //deletes all objects pointed by the class pointers!!!
    void CopyTmpToMaterialCollection(); //creates a copy of all pointers // true is new material was added to material collection
    void CopyMaterialToTmp(int imat);

    //json write/read handling
    void writeToJson(QJsonObject &json);
    void writeMaterialToJson(int imat, QJsonObject &json);
    bool readFromJson(QJsonObject &json);
    void AddNewMaterial(QJsonObject &json);

    //general purpose requests
    void   GetWave(bool& wavelengthResolved, double& waveFrom, double& waveTo, double& waveStep, int& waveNodes) const;
    bool   IsWaveResolved() const {return WavelengthResolved;}
    double getDriftSpeed(int iMat) const; //returns in mm / ns
    double getDiffusionSigmaTime(int iMat, int length_mm) const;
    double getDiffusionSigmaTransverse(int iMat, int length_mm) const;
    void   CheckReadyForGeant4Sim(QString & Errors, QString & Warnings, const AGeoObject * World) const;

public:
    void ConvertToStandardWavelengthes(QVector<double> *sp_x, QVector<double> *sp_y, QVector<double> *y);

    QString CheckMaterial(const AMaterial *mat) const; //"" - check passed, otherwise error
    const QString CheckMaterial(int iMat) const;       //"" - check passed, otherwise error
    const QString CheckTmpMaterial() const;                       //"" - check passed, otherwise error

    int WaveToIndex(double wavelength) const;

private:
    int ConflictingMaterialIndex; //used by CheckMaterial function

    //internal kitchen
    void clearMaterialCollection();
    void generateMaterialsChangedSignal();
    void ensureMatNameIsUnique(AMaterial *mat);

signals:
    void MaterialsChanged(const QStringList);

};

#endif // AMATERIALPARTICLECOLECTION_H
