#include "amercury_si.h"
#include "reconstructor.h"
#include "reconstructor_mp.h"
#include "lrmodel.h"
#include "ajsontools.h"

#include <QVariant>

AMercury_si::AMercury_si() {}

/*
void AMercury_si::createReconstructor_CoG()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecCoG(Model);
}

void AMercury_si::createReconstructor_LS()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecLS(Model);
}

void AMercury_si::createReconstructor_ML()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecML(Model);
}
*/

void AMercury_si::createReconstructor_COG_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new ReconstructorMP(Model, numThreads);
}

void AMercury_si::createReconstructor_LS_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new RecLS_MP(Model, numThreads);
}

void AMercury_si::createReconstructor_ML_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new RecML_MP(Model, numThreads);
}

/*
void AMercury_si::reconstructEvent(QVariantList sensSignals)
{
    const size_t numEl = sensSignals.size();
    std::vector<double> a(numEl);
    std::vector<bool>   sat(numEl, false);

    for (size_t i = 0; i < numEl; i++)
        a[i] = sensSignals[i].toDouble();

    if (Rec) Rec->ProcessEvent(a, sat);
    else     abort("Reconstructor was not yet created");
}
*/

void AMercury_si::reconstructEvents(QVariantList sensorSignalsOverAllEvents)
{
    const size_t numEvents = sensorSignalsOverAllEvents.size();

    std::vector<std::vector<double>> A(numEvents);

    size_t numEl = 0;

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList sensSignals = sensorSignalsOverAllEvents[iEv].toList();
        numEl = sensSignals.size();

        A[iEv].resize(numEl);
        for (size_t i = 0; i < numEl; i++)
            A[iEv][i] = sensSignals[i].toDouble();
    }

    if (RecMP) RecMP->ProcessEvents(A);
    else     abort("Reconstructor was not yet created");
}

/*
double AMercury_si::getPositionX()
{
    if (Rec) return Rec->getRecX();

    abort("Reconstructor was not yet created");
    return 0;
}

double AMercury_si::getPositionY()
{
    if (Rec) return Rec->getRecY();

    abort("Reconstructor was not yet created");
    return 0;
}
*/

QVariantList AMercury_si::getRecXYZ()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    const std::vector<double> & z = RecMP->rec_z;

    const size_t size = x.size();
    if (size != y.size() || size != z.size())
    {
        abort("Mismatch in xyz array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{x[i], y[i], z[i]});
    return res;
}

QVariantList AMercury_si::getRecXYZE()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    const std::vector<double> & z = RecMP->rec_z;
    const std::vector<double> & e = RecMP->rec_e;

    const size_t size = x.size();
    if (size != y.size() || size != z.size() || size != e.size())
    {
        abort("Mismatch in xyze array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{x[i], y[i], z[i], e[i]});
    return res;
}

QVariantList AMercury_si::getRecStats()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<double> & cov_xx = RecMP->cov_xx;
    const std::vector<double> & cov_yy = RecMP->cov_yy;
    const std::vector<double> & cov_xy = RecMP->cov_xy;

    const size_t size = status.size();
    if (size != chi2.size() || size != cov_xx.size() || size != cov_yy.size() || size != cov_xy.size())
    {
        abort("Mismatch in status array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{status[i], chi2[i], cov_xx[i], cov_yy[i], cov_xy[i]});
    return res;
}

void AMercury_si::setCOG_AbsCutoff(double val)
{
    if (Rec) Rec->setCogAbsCutoff(val);
    else abort("Reconstructor was not yet created");
}

void AMercury_si::setCOG_RelCutoff(double val)
{
    if (Rec) Rec->setCogRelCutoff(val);
    else abort("Reconstructor was not yet created");
}

void AMercury_si::setCutoffRadius(double val)
{
    if (Rec) Rec->setRecCutoffRadius(val);
    else abort("Reconstructor was not yet created");
}

// --- LRFs ---

// void AMercury_si::newLightResponseModel(int numSensors)
// {
//     delete Model;
//     Model = new LRModel(numSensors);
// }

void AMercury_si::newLightResponseModel(QVariantList sensorPositions)
{
    size_t numSens = sensorPositions.size();
    if (numSens == 0)
    {
        abort("Cannot create light response model: sensorPositions array is empty");
        return;
    }

    std::vector<std::pair<double,double>> arSensPos(numSens);
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        QVariantList pos = sensorPositions[iSens].toList();
        if (pos.size() < 2)
        {
            abort("Array sensorPositions should contains arrays of at least size of 2: X and Y positions of sensor centers");
            return;
        }

        bool ok0, ok1;
        double x = pos[0].toDouble(&ok0);
        double y = pos[1].toDouble(&ok1);
        if (!ok0 || !ok1)
        {
            abort("Array sensorPositions should contains arrays of at least size of 2: X and Y positions of sensor centers,\nconvertable to double numbers");
            return;
        }
        arSensPos[iSens] = {x, y};
    }

    delete Model;
    Model = new LRModel(numSens);

    for (size_t iSens = 0; iSens < numSens; iSens++)
        Model->AddSensor(iSens, arSensPos[iSens].first, arSensPos[iSens].second);
}

void AMercury_si::addSensor(int iSensor, double x, double y)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if (Model->SensorExists(iSensor)) Model->AddSensor(iSensor, x, y);
    else abort("Sensor index is not valid");
}

#include "asensorhub.h"
void AMercury_si::setLRF(int iSensor, QString jsonString)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if (iSensor < 0 || iSensor >= Model->GetSensorCount())
    {
        abort("set LRF: invalid sensor index");
        return;
    }

    QJsonObject json = jstools::strToJson(jsonString);
    if (json["type"] == "Axial")
    {
        const ASensorHub & SensHub = ASensorHub::getConstInstance();
        int num = SensHub.countSensors();
        if (iSensor <= num)
        {
            if (!json.contains("x0")) json["x0"] = SensHub.getSensorData(iSensor)->Position[0];
            if (!json.contains("y0")) json["y0"] = SensHub.getSensorData(iSensor)->Position[1];
            jsonString = jstools::jsonToString(json);
        }
    }

    Model->SetJsonLRF(iSensor, jsonString.toLatin1().data());
}

void AMercury_si::setLRF(QString jsonString)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    LRF * lrfToClone = LRF::mkFromJson(jsonString.toLatin1().data());
    if (!lrfToClone)
    {
        abort("Failed to make LRF from jsonString");
        return;
    }

    const size_t numGroups = Model->GetGroupCount();
    for (size_t iGr = 0; iGr < numGroups; iGr++)
    {
        LRF * lrf = lrfToClone->clone();
        ifAxialUpdateLrfCenter(lrf, Model->GetGroupX(iGr), Model->GetGroupY(iGr));
        Model->SetGroupLRF(iGr, lrf);
    }

    // find sensors not belonging to any group
    const ASensorHub & SensHub = ASensorHub::getConstInstance();
    const size_t numSens = Model->GetSensorCount();
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        int iGr = Model->GetGroup(iSens);
        if (iGr != -1) continue;

        LRF * lrf = lrfToClone->clone();
        ifAxialUpdateLrfCenter(lrf, SensHub.getSensorData(iSens)->Position[0], SensHub.getSensorData(iSens)->Position[1]);
        Model->SetLRF(iSens, lrf);
    }
}

QString AMercury_si::newLRF_axial(int intervals, double rmin, double rmax)
{
    QJsonObject json;
    json["type"] = "Axial";
    json["nint"] = intervals;
    json["rmin"] = rmin;
    json["rmax"] = rmax;

    return jstools::jsonToString(json);
}

QString AMercury_si::configureLRF_AxialCompression(QString LRF, double k, double lambda, double r0)
{
    QJsonObject json = jstools::strToJson(LRF);

    if (json.isEmpty())
    {
        abort("LRF should be a json object string");
        return "";
    }
    if (json["type"] != "Axial")
    {
        abort("Compression can be applied only to Axial type LRFs");
        return "";
    }

    QJsonObject js;
        js["method"] = "dualslope";
        js["k"]      = k;
        js["lam"]    = lambda;
        js["r0"]     = r0;
    json["compression"] = js;

    return jstools::jsonToString(json);
}

QString AMercury_si::configureLRF_Constrains(QString LRF, bool nonNegative, bool nonIncreasing, bool flattop)
{
    QJsonObject json = jstools::strToJson(LRF);

    if (json.isEmpty())
    {
        abort("LRF should be a json object string");
        return "";
    }

    QJsonArray ar;
        if (nonNegative) ar.push_back("non-negative");
        if (nonIncreasing) ar.push_back("non-increasing");
        if (flattop) ar.push_back("flattop");
    json["constraints"] = ar;

    return jstools::jsonToString(json);
}

void AMercury_si::clearGroups()
{
    if (Model) Model->ResetGroups();
}

int AMercury_si::countGroups()
{
    if (Model) return Model->GetGroupCount();
    else return 0;
}

void AMercury_si::makeGroups_OneForAllSensors()
{
    if (Model) Model->MakeGroupsCommon();
}

void AMercury_si::makeGroups_ByRadius()
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    Model->MakeGroupsByRadius();
    assignGroupLrfsFromSensors();
}

void AMercury_si::MakeGroups_RectanglePattern()
{
    if (Model) Model->MakeGroupsRectangle();
}

void AMercury_si::MakeGroups_SquarePattern()
{
    if (Model) Model->MakeGroupsSquare();
}

void AMercury_si::MakeGroups_HexagonPattern()
{
    if (Model) Model->MakeGroupsHexagon();
}

void AMercury_si::MakeGroups_NgonPattern(int n)
{
    if (Model) Model->MakeGroupsNgon(n);
}

void AMercury_si::setGroupLRF(int iGroup, QString jsonString)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    Model->SetGroupJsonLRF(iGroup, jsonString.toLatin1().data());
}

QString AMercury_si::exportLightResponseModel()
{
    QString res;
    if (Model) res = Model->GetJsonString().data();
    return res;
}

void AMercury_si::importLightResponseModel(QString jsonStr)
{
    delete Model; Model = nullptr;
    Model = new LRModel(jsonStr.toLatin1().data());
}

void AMercury_si::clearAllFitData()
{
    if (Model) Model->ClearAllFitData();
}

void AMercury_si::addFitData(int iSensor, QVariantList xyza)
{
    if (!Model) return;

    const size_t size = xyza.size();
    std::vector<std::array <double, 4>> data(size);
    for (size_t iEv = 0; iEv < size; iEv++)
    {
        QVariantList event = xyza[iEv].toList();
        for (size_t i = 0; i < 4; i++)
            data[iEv][i] = event[i].toDouble();
    }

    Model->AddFitData(iSensor, data);
}

void AMercury_si::fitSensor(int iSensor)
{
    if (Model) Model->FitSensor(iSensor);
}

void AMercury_si::fitGroup(int iGroup)
{
    if (Model) Model->FitGroup(iGroup);
}

void AMercury_si::fitResponse(QVariantList floodSignals, QVariantList floodPositions)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    const size_t numEv = floodSignals.size();
    if (numEv != floodPositions.size())
    {
        abort("Mismatch in the seizes of the flood signal and position arrays");
        return;
    }

    Model->ClearAllFitData();

    // preparing gitting data
    const size_t numSens = Model->GetSensorCount();
    std::vector<std::array<double, 4>> data(numEv);
    std::vector<std::vector<double>> arSig(numEv);
    for (size_t iEv = 0; iEv < numEv; iEv++)
    {
        QVariantList eventPos = floodPositions[iEv].toList();
        if (eventPos.size() < 3)
        {
            abort("Bad format of floodPositions array");
            return;
        }
        for (size_t i = 0; i < 3; i++)
            data[iEv][i] = eventPos[i].toDouble();

        arSig[iEv].resize(numSens);
        QVariantList eventSignals = floodSignals[iEv].toList();
        if (eventSignals.size() != numSens)
        {
            abort("Bad format of floodSignals array");
            return;
        }
        for (size_t iSens = 0; iSens < numSens; iSens++)
        {
            arSig[iEv][iSens] = eventSignals[iSens].toDouble();
        }
    }

    const size_t numGroups = Model->GetGroupCount();
    bool estimateGains = (Model->GetGroupCount() > 0);
    GainEstimator * gainEstimator = nullptr;
    if (estimateGains) gainEstimator = new GainEstimator(Model);

    // adding fitting data
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        std::vector<std::array <double, 4>> thisData(data);
        for (size_t iEv = 0; iEv < numEv; iEv++)
        {
            thisData[iEv][3] = arSig[iEv][iSens];
        }
        Model->AddFitData(iSens, thisData);
        if (estimateGains) gainEstimator->AddData(iSens, thisData);
    }



    //fitting response for sensors not belonging to any groups
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        if (Model->GetGroup(iSens) == -1)
            Model->FitSensor(iSens);
    }

    // fitting response for groups
    for (size_t iGr = 0; iGr < numGroups; iGr++)
        Model->FitGroup(iGr);


    if (estimateGains)
    {
        for (size_t iGr = 0; iGr < numGroups; iGr++)
        {
            qDebug() << "-----\n" << iGr;
            std::set<int> & groupSet = Model->GroupMembers(iGr);
            const int size = groupSet.size();
            std::vector<int> groupVec;
            groupVec.reserve(size);
            for (auto itr = groupSet.begin(); itr != groupSet.end(); itr++)
                groupVec.push_back(*itr);
            qDebug() << "Sensors:" << groupVec;

            std::vector<double> relGains = gainEstimator->GetRelativeGainsList(groupVec, groupVec.front());
            //qDebug() << "Inverse (apparently) gains:" << "-->" << relGains;
            double sumGain = 0;
            for (size_t i = 0; i < size; i++)
                sumGain += 1.0/relGains[i];
            sumGain /= size;

            for (size_t i = 0; i < size; i++)
            {
                Model->SetGain(groupVec[i], 1.0/relGains[i]/sumGain);
                qDebug() << groupVec[i] << "-->" << 1.0/relGains[i]/sumGain;
            }
        }
    }



    // dests
    delete gainEstimator;
}

void AMercury_si::enableSensor(int iSensor, bool enableFlag)
{
    if (!Model)
    {
        abort("Light response model is not defined");
        return;
    }
    if (iSensor < 0 || iSensor >= Model->GetSensorCount())
    {
        abort("Bad sensor index: " + QString::number(iSensor));
        return;
    }

    if (enableFlag) Model->SetEnabled(iSensor);
    else            Model->SetDisabled(iSensor);
}

void AMercury_si::setModelGains(QVariantList gains)
{
    if (!Model)
    {
        abort("Light response model is not defined");
        return;
    }
    size_t size = gains.size();
    if (size != Model->GetSensorCount())
    {
        abort("gains array length is not equal to the number of sensors");
        return;
    }

    for (int i = 0; i < size; i++)
        Model->SetGain(i, gains[i].toDouble());
}

QVariantList AMercury_si::getModelGains()
{
    QVariantList vl;
    if (!Model)
    {
        abort("Light response model is not defined");
        return vl;
    }

    int size = Model->GetSensorCount();
    for (int i = 0; i < size; i++)
        vl << Model->GetGain(i);
    return vl;
}

#include "alrfdrawer.h"
void AMercury_si::plotLRF_radial(int iSensor, bool showNodes)
{
    ALrfDrawer dr(Model);
    QString err = dr.drawRadial(iSensor, showNodes);
    if (!err.isEmpty()) abort(err);
}

#include "ascripthub.h"
#include "alrfmouseexplorer.h" // tmp
void AMercury_si::showLightResponseExplorer()
{
    if (!Model) abort("Light response model is not defined");
    else emit AScriptHub::getInstance().requestShowLightResponseExplorer(Model);
}

double AMercury_si::eval(int iSensor, double x, double y, double z)
{
    if (Model) return Model->Eval(iSensor, x, y, z);
    else return 0;
}

double AMercury_si::eval(int iSensor, QVariantList xyz)
{
    if (xyz.length() != 3) return 0;

    double pos[3];
    for (size_t i = 0; i < 3; i++) pos[i] = xyz[i].toDouble();

    if (Model) return Model->Eval(iSensor, pos);
    else return 0;
}

void AMercury_si::setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls)
{
    RecMinuitMP * rec = dynamic_cast<RecMinuitMP*>(RecMP);
    if (!rec)
    {
        abort("Reconstructor not created or it is not of Minuit type");
        return;
    }
    rec->setRMtolerance(RMtolerance);
    rec->setRMmaxIterations(RMmaxIterations);
    rec->setRMmaxFuncCalls(RMmaxFuncCalls);
}

void AMercury_si::resetReconstructors()
{
    delete Rec;   Rec   = nullptr;
    delete RecMP; RecMP = nullptr;
}

void AMercury_si::assignGroupLrfsFromSensors()
{
    const size_t numGroups = Model->GetGroupCount();
    for (size_t iGr = 0; iGr < numGroups; iGr++)
    {
        std::set <int> & members = Model->GroupMembers(iGr);
        if (members.empty()) continue;
        int iFirst = *(members.begin());
        LRF * lrf = Model->GetLRF(iFirst);  // already nullptr! !!!***
        if (lrf)
        {
            ifAxialUpdateLrfCenter(lrf, Model->GetGroupX(iGr), Model->GetGroupY(iGr));
            Model->SetGroupLRF(iGr, lrf);
        }
    }
}

#include "lrfaxial.h"
void AMercury_si::ifAxialUpdateLrfCenter(LRF * lrf, double x, double y)
{
    LRFaxial * axlrf = dynamic_cast<LRFaxial*>(lrf);
    if (axlrf) axlrf->SetOrigin(x, y);
}

