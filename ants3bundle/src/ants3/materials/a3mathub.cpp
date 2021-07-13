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

A3MatHub::A3MatHub()
{
    tmpMaterial.MatParticle.resize(1);
}

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
    clearMaterialCollection();
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

QString A3MatHub::CheckOverrides()
{
    for (const AMaterial * mat : Materials)
        for (AOpticalOverride * ov : qAsConst(mat->OpticalOverrides))
            if (ov)
            {
                QString err = ov->checkOverrideData();
                if ( !err.isEmpty())
                    return QString("Error in optical override from %1 to %2:\n").arg(mat->name, getMaterialName(ov->getMaterialTo())) + err;
            }
    return QString();
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

void A3MatHub::getFirstOverridenMaterial(int &ifrom, int &ito)
{
    for (ifrom=0; ifrom<Materials.size(); ifrom++)
        for (ito=0; ito<Materials.size(); ito++)
            if (Materials[ifrom]->OpticalOverrides[ito]) return;
    ifrom = 0;
    ito = 0;
}

QString A3MatHub::getMaterialName(int matIndex)
{
    if (matIndex<0 || matIndex >= Materials.size()) return "";
    return Materials.at(matIndex)->name;
}

const QStringList A3MatHub::getListOfMaterialNames() const
{
    QStringList l;
    for (AMaterial* m : Materials)
        l << m->name;
    return l;
}

void A3MatHub::clearMaterialCollection()
{
    for (int i=0; i<Materials.size(); i++)
    {
        if (Materials[i]->PrimarySpectrumHist)
        {
            delete Materials[i]->PrimarySpectrumHist;
            Materials[i]->PrimarySpectrumHist = 0;
        }
        if (Materials[i]->SecondarySpectrumHist)
        {
            delete Materials[i]->SecondarySpectrumHist;
            Materials[i]->SecondarySpectrumHist = 0;
        }

        for (int im=0; im<Materials[i]->OpticalOverrides.size(); im++)
            if (Materials[i]->OpticalOverrides[im])
                delete Materials[i]->OpticalOverrides[im];
        Materials[i]->OpticalOverrides.clear();

        delete Materials[i];
    }
    Materials.clear();

    A3MatHub::ClearTmpMaterial();
}

void A3MatHub::AddNewMaterial(bool fSuppressChangedSignal)
{
    AMaterial *m = new AMaterial;

    int thisMat = Materials.size(); //index of this material (after it is added)
    int numMats = thisMat+1; //collection size after it is added

    //initialize empty optical overrides for all materials
    m->OpticalOverrides.resize(numMats);
    for (int i=0; i<numMats; i++) m->OpticalOverrides[i] = 0;

    //all other materials have to be resized and set to 0 overrides on this material border:
    for (int i=0; i<numMats-1; i++) //over old materials
    {
        Materials[i]->OpticalOverrides.resize(numMats);
        Materials[i]->OpticalOverrides[thisMat] = nullptr;
    }

    //inicialize empty MatParticle vector for all defined particles
//    int numParticles = ParticleCollection.size();
//    m->MatParticle.resize(numParticles);

    //appending to the material collection
    Materials.push_back(m);

    //tmpMaterial object has to be updated too!
    //tmpMaterial "knew" one less material, have to set overrides to itself to nothing
    tmpMaterial.OpticalOverrides.resize(numMats);
    for (int i=0; i<numMats; i++) tmpMaterial.OpticalOverrides[thisMat] = 0;

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void A3MatHub::AddNewMaterial(QString name, bool fSuppressChangedSignal)
{
    AddNewMaterial(true);
    Materials.back()->name = name;
    ensureMatNameIsUnique(Materials.back());

    if (!fSuppressChangedSignal) emit materialsChanged();
}

void A3MatHub::ClearTmpMaterial()
{
    tmpMaterial.name = "";
    tmpMaterial.density = 0;
    tmpMaterial.temperature = 298.0;
    tmpMaterial.n = 1;
    tmpMaterial.abs = 0;
    tmpMaterial.reemissionProb = 0;
    tmpMaterial.rayleighMFP = 0;
    tmpMaterial.e_driftVelocity = 0;
    tmpMaterial.e_diffusion_L = 0;
    tmpMaterial.e_diffusion_T = 0;
    tmpMaterial.W = 0;
    tmpMaterial.SecYield = 0;
    tmpMaterial.SecScintDecayTime = 0;
    tmpMaterial.Comments = "";

    tmpMaterial.PriScint_Decay.clear();
    tmpMaterial.PriScint_Decay << APair_ValueAndWeight(0, 1.0);
    tmpMaterial.PriScint_Raise.clear();
    tmpMaterial.PriScint_Raise << APair_ValueAndWeight(0, 1.0);

    tmpMaterial.PhotonYieldDefault = 0;
    tmpMaterial.IntrEnResDefault = 0;

/*
    int particles = ParticleCollection.size();
    tmpMaterial.MatParticle.resize(particles);
    for (int i=0; i<particles; i++)
    {
        tmpMaterial.MatParticle[i].InteractionDataF.resize(0);
        tmpMaterial.MatParticle[i].InteractionDataX.resize(0);
        tmpMaterial.MatParticle[i].Terminators.resize(0);
        tmpMaterial.MatParticle[i].PhYield = 0;
        tmpMaterial.MatParticle[i].IntrEnergyRes = 0;
        tmpMaterial.MatParticle[i].TrackingAllowed = false;
    }
*/

    int materials = Materials.size();

    tmpMaterial.OpticalOverrides.resize(materials);
    for (int i=0; i<materials; i++)
    {
        if (tmpMaterial.OpticalOverrides[i])
        {
            delete tmpMaterial.OpticalOverrides[i];
            tmpMaterial.OpticalOverrides[i] = 0;
        }
    }

    tmpMaterial.nWave_lambda.clear();
    tmpMaterial.nWave.clear();
    tmpMaterial.nWaveBinned.clear();
    tmpMaterial.absWave_lambda.clear();
    tmpMaterial.absWave.clear();
    tmpMaterial.absWaveBinned.clear();
    tmpMaterial.reemisProbWave.clear();
    tmpMaterial.reemisProbWave_lambda.clear();
    tmpMaterial.reemissionProbBinned.clear();

    tmpMaterial.PrimarySpectrum_lambda.clear();
    tmpMaterial.PrimarySpectrum.clear();
    tmpMaterial.SecondarySpectrum_lambda.clear();
    tmpMaterial.SecondarySpectrum.clear();

    //---POINTERS---
    if (tmpMaterial.PrimarySpectrumHist)
    {
        delete tmpMaterial.PrimarySpectrumHist;
        tmpMaterial.PrimarySpectrumHist = 0;
    }
    if (tmpMaterial.SecondarySpectrumHist)
    {
        delete tmpMaterial.SecondarySpectrumHist;
        tmpMaterial.SecondarySpectrumHist = 0;
    }
    if (tmpMaterial.GeoMat)
    {
        delete tmpMaterial.GeoMat;
        tmpMaterial.GeoMat = 0;
    }
    if (tmpMaterial.GeoMed)
    {
        delete tmpMaterial.GeoMed;
        tmpMaterial.GeoMed = 0;
    }
}

void A3MatHub::CopyTmpToMaterialCollection()
{
    QString name = tmpMaterial.name;
    int index = A3MatHub::FindMaterial(name);

    if (index == -1)
    {
        //      qDebug()<<"MaterialCollection--> New material: "<<name;
        A3MatHub::AddNewMaterial();
        index = Materials.size()-1;
    }
    else
    {
        //      qDebug()<<"MaterialCollection--> Material "+name+" already defined; index = "<<index;
    }
    //*MaterialCollectionData[index] = tmpMaterial; //updating material properties
    QJsonObject js;
    tmpMaterial.writeToJson(js);
    Materials[index]->readFromJson(js);

    //now update pointers!
    A3MatHub::UpdateWaveResolvedProperties(index); //updating effective properties (hists, Binned), remaking hist objects (Pointers are safe - they objects are recreated on each copy)

    emit materialsChanged();
    return;
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

        for (int ior=0; ior<Materials[imat]->OpticalOverrides.size(); ior++)
            if (Materials[imat]->OpticalOverrides[ior])
                Materials[imat]->OpticalOverrides[ior]->initializeWaveResolved();
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

        for (int ior=0; ior<Materials[imat]->OpticalOverrides.size(); ior++)
            if (Materials[imat]->OpticalOverrides[ior])
                Materials[imat]->OpticalOverrides[ior]->initializeWaveResolved();
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
    return mat->CheckMaterial();
}

const QString A3MatHub::CheckMaterial(int iMat) const
{
    if (iMat<0 || iMat>=Materials.size()) return "Wrong material index: " + QString::number(iMat);
    return CheckMaterial(Materials[iMat]);
}

const QString A3MatHub::CheckTmpMaterial() const
{
    return tmpMaterial.CheckMaterial();
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
    if (Materials[imat]->PrimarySpectrumHist)
        delete Materials[imat]->PrimarySpectrumHist;
    if (Materials[imat]->SecondarySpectrumHist)
        delete Materials[imat]->SecondarySpectrumHist;
    for (int iOther=0; iOther<size; iOther++)  //overrides from this materials to other materials
    {
        if ( Materials[imat]->OpticalOverrides[iOther] )
        {
            delete Materials[imat]->OpticalOverrides[iOther];
            Materials[imat]->OpticalOverrides[iOther] = 0;
        }
    }

    //clear overrides from other materials to this one
    for (int iOther=0; iOther<size; iOther++)
    {
        if ( Materials[iOther]->OpticalOverrides[imat] ) delete Materials[iOther]->OpticalOverrides[imat];
        Materials[iOther]->OpticalOverrides.remove(imat);
    }

    //delete this material
    delete Materials[imat];
    //Materials.remove(imat);
    Materials.erase( std::next(Materials.begin(), imat) );

    //update indices of override materials
    for (int i=0; i<size-1; i++)
        for (int j=0; j<size-1; j++)
            if (Materials[i]->OpticalOverrides[j])
                Materials[i]->OpticalOverrides[j]->updateMatIndices(i, j);

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

    QJsonArray oar;
    for (size_t iFrom=0; iFrom<Materials.size(); iFrom++)
        for (size_t iTo=0; iTo<Materials.size(); iTo++)
        {
            if ( !Materials.at(iFrom)->OpticalOverrides.at(iTo) ) continue;
            QJsonObject js;
            Materials.at(iFrom)->OpticalOverrides[iTo]->writeToJson(js);
            oar.append(js);
        }
    if (!oar.isEmpty()) js["Overrides"] = oar;

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
        exit(-2);
    }
    QJsonObject js = json["MaterialCollection"].toObject();

    //reading materials
    QJsonArray ar = js["Materials"].toArray();
    if (ar.isEmpty())
    {
        qCritical() << "No materials in json";
        exit(-2);
    }
    A3MatHub::clearMaterialCollection();
    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject jj = ar[i].toObject();
        A3MatHub::AddNewMaterial(true); //also initialize overrides
        Materials.back()->readFromJson(jj);
    }
    int numMats = countMaterials();
    //qDebug() << "--> Loaded material collection with"<<numMats<<"materials";

    //reading overrides if present
    QJsonArray oar = js["Overrides"].toArray();
    for (int i=0; i<oar.size(); i++)
    {
        QJsonObject jj = oar[i].toObject();
        if (jj.contains("Model"))
        {
            //new format
            QString model = jj["Model"].toString();
            int MatFrom = jj["MatFrom"].toInt();
            int MatTo = jj["MatTo"].toInt();
            if (MatFrom>numMats-1 || MatTo>numMats-1)
            {
                qWarning()<<"Attempt to override for non-existent material skipped";
                continue;
            }
            AOpticalOverride* ov = OpticalOverrideFactory(model, this, MatFrom, MatTo);
            if (!ov || !ov->readFromJson(jj))
                qWarning() << Materials[MatFrom]->name  << ": optical override load failed!";
            else Materials[MatFrom]->OpticalOverrides[MatTo] = ov;
        }
        //compatibility with old format:
        else if (jj.contains("MatFrom") && jj.contains("ScatModel"))
        {
            int MatFrom, MatTo;
            jstools::parseJson(jj, "MatFrom", MatFrom);
            jstools::parseJson(jj, "MatTo", MatTo);
            if (MatFrom>numMats-1 || MatTo>numMats-1)
            {
                qWarning()<<"Attempt to override for non-existent material skipped";
                continue;
            }
            double Abs, Scat, Spec;
            Abs = Scat = Spec = 0;
            int ScatMode = 1;
            jstools::parseJson(jj, "Loss", Abs);
            jstools::parseJson(jj, "Ref", Spec);
            jstools::parseJson(jj, "Scat", Scat);
            jstools::parseJson(jj, "ScatModel", ScatMode);
            ABasicOpticalOverride* ov = new ABasicOpticalOverride(this, MatFrom, MatTo);
            if (!ov)
                qWarning() << Materials[MatFrom]->name << ": optical override load failed!";
            else
            {
                Materials[MatFrom]->OpticalOverrides[MatTo] = ov;
                ov->probLoss = Abs;
                ov->probRef = Spec;
                ov->probDiff = Scat;
                ov->scatterModel = ScatMode;
            }
        }
    }
    //qDebug() << "--> Loaded material border overrides from array with"<<oar.size()<<"entries";

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
