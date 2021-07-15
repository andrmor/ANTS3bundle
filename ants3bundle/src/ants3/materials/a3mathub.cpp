#include "a3mathub.h"
//#include "ageneralsimsettings.h"
#include "aopticaloverride.h"
#include "ajsontools.h"
#include "acommonfunctions.h"
#include "atracerstateful.h"
#include "ascriptopticaloverride.h"

#include <QtDebug>
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

A3MatHub::A3MatHub() {}

A3MatHub &A3MatHub::getInstance()
{
    static A3MatHub instance;
    return instance;
}

const A3MatHub & A3MatHub::getConstInstance()
{
    return getInstance();
}

A3MatHub::~A3MatHub()
{
    clearMaterials();
}

void A3MatHub::SetWave(bool wavelengthResolved, double waveFrom, double waveTo, double waveStep, int waveNodes)
{
    WavelengthResolved = wavelengthResolved;
    WaveFrom = waveFrom;
    WaveTo = waveTo;
    WaveStep = waveStep;
    WaveNodes = waveNodes;
}

void A3MatHub::GetWave(bool &wavelengthResolved, double &waveFrom, double &waveTo, double &waveStep, int &waveNodes) const
{
    wavelengthResolved = WavelengthResolved;
    waveFrom = WaveFrom;
    waveTo = WaveTo;
    waveStep = WaveStep;
    waveNodes = WaveNodes;
}

double A3MatHub::getDriftSpeed(int iMat) const
{
    return 0.01 * Materials.at(iMat)->e_driftVelocity; //given in cm/us - returns in mm/ns
}

double A3MatHub::getDiffusionSigmaTime(int iMat, int length_mm) const
{
    //sqrt(2Dl/v^3)
    //https://doi.org/10.1016/j.nima.2016.01.094
    const AMaterial * m = Materials.at(iMat);
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_L; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v) / v; // in ns
}

double A3MatHub::getDiffusionSigmaTransverse(int iMat, int length_mm) const
{
    //sqrt(2Dl/v)
    const AMaterial * m = Materials.at(iMat);
    if (m->e_driftVelocity == 0 || m->e_diffusion_L == 0) return 0;

    const double v = 0.01 * m->e_driftVelocity; // in mm/ns <- from cm/us
    const double d = m->e_diffusion_T; //now in mm^2/ns

    return sqrt(2.0 * d * length_mm / v); // in mm
}

void A3MatHub::UpdateRuntimePropertiesAndWavelengthBinning(AGeneralSimSettings * SimSet)
{
 //   SetWave(SimSet->fWaveResolved, SimSet->WaveFrom, SimSet->WaveTo, SimSet->WaveStep, SimSet->WaveNodes);
    for (int imat = 0; imat < Materials.size(); imat++)
    {
        UpdateWaveResolvedProperties(imat);
        Materials[imat]->updateRuntimeProperties();
    }
}

bool A3MatHub::isScriptOpticalOverrideDefined() const
{
/*
    for (AMaterial* mat : MaterialCollectionData)
        for (AOpticalOverride* ov : mat->OpticalOverrides)
            if (ov)
            {
                AScriptOpticalOverride* sov = dynamic_cast<AScriptOpticalOverride*>(ov);
                if (sov) return true;
            }
*/
    return false;
}

QString A3MatHub::getMaterialName(int matIndex) const
{
    if (matIndex<0 || matIndex >= Materials.size()) return "";
    return Materials.at(matIndex)->name;
}

const QStringList A3MatHub::getListOfMaterialNames() const
{
    QStringList l;
    for (AMaterial * m : Materials)
        l << m->name;
    return l;
}

void A3MatHub::clearMaterials()
{
    for (AMaterial * mat : Materials)
    {
        delete mat->PrimarySpectrumHist;
        delete mat->SecondarySpectrumHist;

        delete mat;
    }
    Materials.clear();

    tmpMaterial.clear();
}

void A3MatHub::AddNewMaterial(bool fSuppressChangedSignal)
{
    AMaterial *m = new AMaterial;

    int thisMat = Materials.size(); //index of this material (after it is added)
    int numMats = thisMat+1; //collection size after it is added

    // !!!*** override handling?

    //appending to the material collection
    Materials.push_back(m);

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void A3MatHub::AddNewMaterial(QString name, bool fSuppressChangedSignal)
{
    AddNewMaterial(true);
    Materials.back()->name = name;
    ensureMatNameIsUnique(Materials.back());

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void A3MatHub::CopyTmpToMaterialCollection()
{
    const QString name = tmpMaterial.name;
    int index = FindMaterial(name);
    if (index == -1)
    {
        //      qDebug()<<"MaterialCollection--> New material: "<<name;
        AddNewMaterial(true);
        index = Materials.size() - 1;
    }
    else
    {
        //      qDebug()<<"MaterialCollection--> Material "+name+" already defined; index = "<<index;
    }

    QJsonObject js;
    tmpMaterial.writeToJson(js);
    Materials[index]->readFromJson(js);

    //now update pointers!   !!!*** need it here?
    UpdateWaveResolvedProperties(index); //updating effective properties (hists, Binned), remaking hist objects (Pointers are safe - they objects are recreated on each copy)

    emit materialsChanged();
}

int A3MatHub::FindMaterial(const QString & name) const
{
    const int size = Materials.size();

    for (int index = 0; index < size; index++)
        if (name == Materials[index]->name) return index;

    return -1;
}

void A3MatHub::UpdateWaveResolvedProperties(int imat)
{
    //qDebug()<<"Wavelength-resolved?"<<WavelengthResolved;
    //qDebug()<<"--updating wavelength-resolved properties for material index"<<imat;
    if (WavelengthResolved)
    {
        //calculating histograms and "-Binned" for effective data
        Materials[imat]->nWaveBinned.clear();
        if (Materials[imat]->nWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->nWave_lambda, &Materials[imat]->nWave, &Materials[imat]->nWaveBinned);

        Materials[imat]->absWaveBinned.clear();
        if (Materials[imat]->absWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->absWave_lambda, &Materials[imat]->absWave, &Materials[imat]->absWaveBinned);

        Materials[imat]->reemissionProbBinned.clear();
        if (Materials[imat]->reemisProbWave_lambda.size() > 0)
            ConvertToStandardWavelengthes(&Materials[imat]->reemisProbWave_lambda, &Materials[imat]->reemisProbWave, &Materials[imat]->reemissionProbBinned);

        if (Materials[imat]->rayleighMFP != 0)
        {
            Materials[imat]->rayleighBinned.clear();
            double baseWave4 = Materials[imat]->rayleighWave * Materials[imat]->rayleighWave * Materials[imat]->rayleighWave * Materials[imat]->rayleighWave;
            double base = Materials[imat]->rayleighMFP / baseWave4;
            for (int i=0; i<WaveNodes; i++)
            {
                double wave = WaveFrom + WaveStep*i;
                double wave4 = wave*wave*wave*wave;
                Materials[imat]->rayleighBinned.append(base * wave4);
            }
        }

        if (Materials[imat]->PrimarySpectrumHist)
        {
            delete Materials[imat]->PrimarySpectrumHist;
            Materials[imat]->PrimarySpectrumHist = 0;
        }
        if (Materials[imat]->PrimarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            ConvertToStandardWavelengthes(&Materials[imat]->PrimarySpectrum_lambda, &Materials[imat]->PrimarySpectrum, &y);
            TString name = "PrimScSp";
            name += imat;
            Materials[imat]->PrimarySpectrumHist = new TH1D(name,"Primary scintillation", WaveNodes, WaveFrom, WaveTo);
            for (int j = 1; j<WaveNodes+1; j++)  Materials[imat]->PrimarySpectrumHist->SetBinContent(j, y[j-1]);
            Materials[imat]->PrimarySpectrumHist->GetIntegral(); //to make thread safe
        }

        if (Materials[imat]->SecondarySpectrumHist)
        {
            delete Materials[imat]->SecondarySpectrumHist;
            Materials[imat]->SecondarySpectrumHist = 0;
        }
        if (Materials[imat]->SecondarySpectrum_lambda.size() > 0)
        {
            QVector<double> y;
            ConvertToStandardWavelengthes(&Materials[imat]->SecondarySpectrum_lambda, &Materials[imat]->SecondarySpectrum, &y);
            TString name = "SecScSp";
            name += imat;
            Materials[imat]->SecondarySpectrumHist = new TH1D(name,"Secondary scintillation", WaveNodes, WaveFrom, WaveTo);
            for (int j = 1; j<WaveNodes+1; j++)  Materials[imat]->SecondarySpectrumHist->SetBinContent(j, y[j-1]);
            Materials[imat]->SecondarySpectrumHist->GetIntegral(); //to make thread safe
        }
    }
    else
    {
        //making empty histograms and "-Binned" for effective data
        Materials[imat]->nWaveBinned.clear();
        Materials[imat]->absWaveBinned.clear();
        Materials[imat]->reemissionProbBinned.clear();
        Materials[imat]->rayleighBinned.clear();
        if (Materials[imat]->PrimarySpectrumHist)
        {
            delete Materials[imat]->PrimarySpectrumHist;
            Materials[imat]->PrimarySpectrumHist = 0;
        }
        if (Materials[imat]->SecondarySpectrumHist)
        {
            delete Materials[imat]->SecondarySpectrumHist;
            Materials[imat]->SecondarySpectrumHist = 0;
        }
    }
}

void A3MatHub::CopyMaterialToTmp(int imat)
{
    if (imat<0 || imat>Materials.size()-1)
    {
        qWarning()<<"Error: attempting to copy non-existent material #"<<imat<< " to tmpMaterial!";
        return;
    }
    QJsonObject js;
    Materials[imat]->writeToJson(js);
    tmpMaterial.readFromJson(js);
}

void A3MatHub::ConvertToStandardWavelengthes(QVector<double>* sp_x, QVector<double>* sp_y, QVector<double>* y)
{
    y->resize(0);

    //qDebug()<<"Data range:"<<sp_x->at(0)<<sp_x->at(sp_x->size()-1);
    double xx, yy;
    for (int i=0; i<WaveNodes; i++)
    {
        xx = WaveFrom + WaveStep*i;
        if (xx <= sp_x->at(0)) yy = sp_y->at(0);
        else
        {
            if (xx >= sp_x->at(sp_x->size()-1)) yy = sp_y->at(sp_x->size()-1);
            else
            {
                //general case
                yy = GetInterpolatedValue(xx, sp_x, sp_y); //reusing interpolation function from functions.h
                if (yy<0) yy = 0; //!!! protection against negative
            }
        }
        //      qDebug()<<xx<<yy;
        y->append(yy);
    }
}

QString A3MatHub::CheckMaterial(const AMaterial* mat) const
{
    if (!mat) return "nullptr material";
    return mat->checkMaterial();
}

QString A3MatHub::CheckMaterial(int iMat) const
{
    if (iMat<0 || iMat>=Materials.size()) return "Wrong material index: " + QString::number(iMat);
    return CheckMaterial(Materials[iMat]);
}

QString A3MatHub::CheckTmpMaterial() const
{
    return tmpMaterial.checkMaterial();
}

bool A3MatHub::DeleteMaterial(int imat)
{
    int size = Materials.size();
    if (imat<0 || imat >= size)
    {
        qWarning()<<"Attempt to remove material with invalid index!";
        return false;
    }

    //clear dynamic properties of this material
    delete Materials[imat]->PrimarySpectrumHist;
    delete Materials[imat]->SecondarySpectrumHist;

    emit materialsChanged();

    return true;
}

void A3MatHub::writeToJson(QJsonObject &json)
{
    QJsonObject js;

    QJsonArray ar;
    for (const AMaterial * m : Materials)
    {
        QJsonObject jj;
        m->writeToJson(jj);
        ar.append(jj);
    }
    js["Materials"] = ar;

    json["MaterialCollection"] = js;
}

void A3MatHub::writeMaterialToJson(int imat, QJsonObject &json)
{
    if (imat < 0 || imat >= (int)Materials.size())
    {
        qWarning() << "Attempt to save non-existent material!";
        return;
    }
    Materials[imat]->writeToJson(json);
}

#include "abasicopticaloverride.h"
bool A3MatHub::readFromJson(QJsonObject &json)
{
    if (!json.contains("MaterialCollection"))
    {
        qCritical() << "Material collection not found in json";
        exit(2);
    }
    QJsonObject js = json["MaterialCollection"].toObject();

    //reading materials
    QJsonArray ar = js["Materials"].toArray();
    if (ar.isEmpty())
    {
        qCritical() << "No materials in json";
        exit(-2);
    }
    clearMaterials();
    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject jj = ar[i].toObject();
            AddNewMaterial(true); //also initialize overrides
        Materials.back()->readFromJson(jj);
    }

    emit materialsChanged();

    return true;
}

void A3MatHub::AddNewMaterial(QJsonObject &json) //have to be sure json is indeed material properties!
{
    AddNewMaterial();
    AMaterial * mat = Materials.back();
    mat->readFromJson(json);

    ensureMatNameIsUnique(mat);

    emit materialsChanged();
}

void A3MatHub::ensureMatNameIsUnique(AMaterial * mat)
{
    QString name = mat->name;
    bool fFound;
    do
    {
        fFound = false;
        for (const AMaterial * m : Materials)
        {
            if (m == mat) continue;
            if (m->name == name)
            {
                fFound = true;
                name += "*";
                break;
            }
        }
    }
    while (fFound);
    mat->name = name;
}

int A3MatHub::WaveToIndex(double wavelength) const
{
    if (!WavelengthResolved) return -1;

    int iwave = round( (wavelength - WaveFrom) / WaveStep );
    if (iwave >= WaveNodes) iwave = WaveNodes-1;
    if (iwave < 0) iwave = 0;
    return iwave;
}

#include "ageoobject.h"
void A3MatHub::CheckReadyForGeant4Sim(QString & Errors, QString & Warnings, const AGeoObject * World) const
{
    for (int iM = 0; iM<Materials.size(); iM++)
    {
        if (!World->isMaterialInActiveUse(iM)) continue;

        const AMaterial * mat = Materials.at(iM);
        if (!mat->ChemicalComposition.isDefined())
            Errors += QString("\nComposition not defined for %1, while needed for tracking!\n").arg(mat->name);
    }
}
