#include "amaterialhub.h"
#include "ageoobject.h"
#include "ageometryhub.h"
#include "ainterfacerulehub.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "TROOT.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TH1D.h"
#include "TString.h"

#include <cmath>

AMaterialHub::AMaterialHub() {}

AMaterialHub &AMaterialHub::getInstance()
{
    static AMaterialHub instance;
    return instance;
}

const AMaterialHub & AMaterialHub::getConstInstance()
{
    return getInstance();
}

AMaterialHub::~AMaterialHub()
{
    clearMaterials();
}

double AMaterialHub::getDriftSpeed(int iMat) const
{
    return 0.01 * Materials[iMat]->ElDriftVelocity; //given in cm/us - returns in mm/ns
}

double AMaterialHub::getDiffusionSigmaTime(int iMat, double length_mm) const
{
    //sqrt(2Dl/v^3)
    //https://doi.org/10.1016/j.nima.2016.01.094
    const AMaterial * m = Materials[iMat];
    if (m->ElDriftVelocity == 0 || m->ElDiffusionL == 0) return 0;

    const double v = 0.01 * m->ElDriftVelocity; // in mm/ns <- from cm/us
    const double d = m->ElDiffusionL; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v) / v; // in ns
}

double AMaterialHub::getDiffusionSigmaTransverse(int iMat, double length_mm) const
{
    //sqrt(2Dl/v)
    const AMaterial * m = Materials[iMat];
    if (m->ElDriftVelocity == 0 || m->ElDiffusionL == 0) return 0;

    const double v = 0.01 * m->ElDriftVelocity; // in mm/ns <- from cm/us
    const double d = m->ElDiffusionT; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v); // in mm
}

void AMaterialHub::updateRuntimeProperties()
{
    for (AMaterial * mat : Materials) mat->updateRuntimeOpticalProperties();
}

QString AMaterialHub::getMaterialName(int matIndex) const
{
    if (matIndex < 0 || matIndex >= (int)Materials.size()) return "";
    return Materials[matIndex]->Name;
}

QStringList AMaterialHub::getListOfMaterialNames() const
{
    QStringList l;
    for (AMaterial * m : Materials) l << m->Name;
    return l;
}

void AMaterialHub::convertPressureToDensity()
{
    for (AMaterial * mat : Materials)
    {
        QString err = mat->convertPressureToDensity();
        if (!err.isEmpty()) AErrorHub::addQError("Gas to pressure convertion problem for " + mat->Name + ": " + err);
    }
}

void AMaterialHub::generateGeoMedia(bool useGdmlExportNames)
{
    for (size_t iMat = 0; iMat < Materials.size(); iMat++)
    {
        AMaterial * mat = Materials[iMat];
        mat->generateTGeoMat(useGdmlExportNames ? getGdmlExportName(iMat) : mat->Name);
        mat->_GeoMed = new TGeoMedium(mat->Name.toLocal8Bit().data(), iMat, mat->_GeoMat);
    }
}

QString AMaterialHub::makeGdmlExportName(int iMat, const QString & name)
{
    // the prefix makes the name unique and ensures it does not start with a digit
    return QString("m%1_%2").arg(iMat).arg(AGeometryHub::sanitizeForGdml(name));
}

QString AMaterialHub::getGdmlExportName(int iMat) const
{
    if (iMat < 0 || iMat >= (int)Materials.size()) return "";
    return makeGdmlExportName(iMat, Materials[iMat]->Name);
}

#include <QXmlStreamReader>
#include <map>
QString AMaterialHub::checkGdmlExportNames(const QString & gdmlFileName) const
{
    QFile file(gdmlFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) return "Cannot open exported GDML file for verification: " + gdmlFileName;

    std::map<QString, int> found; // material name in GDML -> number of occurrences
    bool inMaterials = false;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.isStartElement())
        {
            if      (xml.name() == QLatin1String("materials")) inMaterials = true;
            else if (inMaterials && xml.name() == QLatin1String("material"))
                found[xml.attributes().value("name").toString()]++;
        }
        else if (xml.isEndElement() && xml.name() == QLatin1String("materials")) inMaterials = false;
    }
    if (xml.hasError()) return "Error parsing exported GDML file " + gdmlFileName + ": " + xml.errorString();

    QString err;
    for (size_t iMat = 0; iMat < Materials.size(); iMat++)
    {
        const QString expected = getGdmlExportName(iMat);
        const auto it = found.find(expected);
        const int num = (it == found.end() ? 0 : it->second);
        if (num != 1)
            err += QString("\n  material #%1 (%2): expected GDML name %3, found %4 time(s)").arg(iMat).arg(Materials[iMat]->Name).arg(expected).arg(num);
    }
    if (!err.isEmpty())
        return "Material names in the exported GDML file do not match the expected names (has ROOT changed its GDML naming rules?):" + err;
    return "";
}

void AMaterialHub::clearMaterials()
{
    for (AMaterial * mat : Materials)
    {
        mat->clearDynamicProperties();
        delete mat;
    }
    Materials.clear();
}

void AMaterialHub::addNewMaterial()
{
    AMaterial * m = new AMaterial(); // vacuum by default

    AInterfaceRuleHub::getInstance().onMaterialAdded();

    Materials.push_back(m);
}

void AMaterialHub::addNewMaterial(QString name)
{
    addNewMaterial();
    Materials.back()->Name = name;
    ensureMatNameIsUnique(Materials.back());
}

void AMaterialHub::copyMaterialToTmp(int imat, AMaterial & tmpMaterial)
{
    if (imat < 0 || imat >= (int)Materials.size())
    {
        qWarning()<<"Error: attempting to copy non-existent material #"<<imat<< " to tmpMaterial!";
        return;
    }

    //do not want to copy dynamic objects!
    QJsonObject js;
        Materials[imat]->writeToJson(js);
    tmpMaterial.readFromJson(js);
}

double AMaterialHub::getS1PhotonYield(int iMat, const QString & particle) const
{
    if (iMat < 0 || iMat >= (int)Materials.size()) return 0;
    return Materials[iMat]->PhotonYield;
    //if (particle.isEmpty()) return Materials[iMat]->PhotonYieldDefault;
    //else return ***;
}

double AMaterialHub::getS1FanoFactor(int iMat) const
{
    if (iMat < 0 || iMat >= (int)Materials.size()) return 0;
    return Materials[iMat]->FanoS1;
}

void AMaterialHub::copyToMaterials(const AMaterial & tmpMaterial)
{
    const QString name = tmpMaterial.Name;
    int index = findMaterial(name);
    if (index == -1)
    {
        //qDebug() << "MatHub-> New material: " << name;
        addNewMaterial();
        index = Materials.size() - 1;
    }
    //else qDebug() << "MatHub-> Material " + name + " already defined; index = " << index;

    //do not copy dynamic properties!
    QJsonObject js;
    tmpMaterial.writeToJson(js);
    Materials[index]->readFromJson(js);

    AGeometryHub::getInstance().populateGeoManager();
}

int AMaterialHub::findMaterial(const QString & name) const
{
    const int size = Materials.size();

    for (int index = 0; index < size; index++)
        if (name == Materials[index]->Name) return index;

    return -1;
}

QString AMaterialHub::tryRemoveMaterial(int iMat)
{
    int size = Materials.size();
    if (iMat < 0 || iMat >= size) return QString("Cannot remove material with invalid index %0").arg(iMat);

    QString volName;
    bool bUsed = AGeometryHub::getInstance().isMaterialInUse(iMat, volName);
    if (bUsed) return QString("Material is in use by at least one geometry object.\n"
                              "The first object found: %0").arg(volName);

    removeMaterial(iMat);
    return "";
}

void AMaterialHub::removeMaterial(int iMat)
{
    Materials[iMat]->clearDynamicProperties();
    delete Materials[iMat];
    Materials.erase(Materials.begin() + iMat);

    AInterfaceRuleHub::getInstance().onMaterialRemoved(iMat);
    AGeometryHub::getInstance().onMaterialRemoved(iMat);
}

QString AMaterialHub::CheckMaterial(const AMaterial* mat) const
{
    if (!mat) return "nullptr material";
    return mat->checkMaterial();
}

QString AMaterialHub::CheckMaterial(int iMat) const
{
    if (iMat < 0 || iMat >= (int)Materials.size()) return "Wrong material index: " + QString::number(iMat);
    return CheckMaterial(Materials[iMat]);
}

void AMaterialHub::importMaterials(TList * matList)
{
    clearMaterials();

    const int size = matList->GetEntries();
    for (int i = 0; i < size; i++)
    {
        TGeoMaterial * tmat = (TGeoMaterial*)matList->At(i);

        addNewMaterial();
        AMaterial & amat = *Materials.back();

        amat.Name = tmat->GetName();
        amat.importComposition(tmat);
    }

    if (countMaterials() == 0) addNewMaterial("Vacuum"); // !!!*** error?
}

void AMaterialHub::writeToJson(QJsonObject & json) const
{
    QJsonArray ar;
    for (const AMaterial * m : Materials)
    {
        QJsonObject js;
        m->writeToJson(js);
        ar.append(js);
    }

    json["Materials"] = ar;
}

QString AMaterialHub::readFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "Materials", ar);
    if (!ok) return "Json does not contain material hub settings";

    clearMaterials();

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        addNewMaterial();
        Materials.back()->readFromJson(js);
    }

    if (Materials.empty())
    {
        addNewMaterial("Dummy"); //emits the signal!
        return "Materials are empty!";
    }

    return "";
}

void AMaterialHub::clear()
{
    clearMaterials();
    addNewMaterial("Vacuum");
}

void AMaterialHub::addNewMaterial(QJsonObject & json) //have to be sure json is indeed material properties!
{
    addNewMaterial();
    AMaterial * mat = Materials.back();
    mat->readFromJson(json);

    ensureMatNameIsUnique(mat);
}

bool AMaterialHub::renameMaterial(int iMat, const QString & newName)
{
    for (const AMaterial * m : Materials)
        if (newName == m->Name) return false;

    Materials[iMat]->Name = newName;
    return true;
}

void AMaterialHub::ensureMatNameIsUnique(AMaterial * mat)
{
    QString name = mat->Name;
    bool fFound;
    do
    {
        fFound = false;
        for (const AMaterial * m : Materials)
        {
            if (m == mat) continue;
            if (m->Name == name)
            {
                fFound = true;
                name += "*";
                break;
            }
        }
    }
    while (fFound);
    mat->Name = name;
}

void AMaterialHub::checkReadyForGeant4Sim(QString & Errors) const
{
    const AGeoObject * World = AGeometryHub::getInstance().World;

    for (int iM = 0; iM < (int)Materials.size(); iM++)
    {
        if (!World->isMaterialInActiveUse(iM)) continue;

        const AMaterial * mat = Materials[iM];

        if (mat->CompositionType == AMaterial::Geant4Material && mat->G4MaterialName.isEmpty())
            Errors += QString("\nGeant4 material use activated for %1, but G4 name not selected\n").arg(mat->Name);

        // !!!*** check NCrystal?

        // !!!*** check material here, which should include check of composition
        //if (!mat->Composition.isDefined() && !mat->UseNistMaterial)
        //    Errors += QString("\nComposition not defined for material %1\n").arg(mat->Name);
    }
}
