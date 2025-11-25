#include "alightresponse_si.h"
#include "alightresponsehub.h"
#include "asensorhub.h"
#include "ascripthub.h"
#include "ajsontools.h"
#include "alrfdrawer.h"

#include "lrmodel.h"
#include "lrfaxial.h"

ALightResponse_SI::ALightResponse_SI() :
    LRHub(ALightResponseHub::getInstance()) {}

// --- High level ---

void ALightResponse_SI::newResponseModel(QVariantList sensorPositions)
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

    clearModel();
    LRHub.Model = new LRModel(numSens);

    for (size_t iSens = 0; iSens < numSens; iSens++)
        LRHub.Model->AddSensor(iSens, arSensPos[iSens].first, arSensPos[iSens].second);
}

#include "afiletools.h"
void ALightResponse_SI::saveResponseModel(QString fileName)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return;
    }

    QString jsonStr = LRHub.Model->GetJsonString().data();
    bool ok = ftools::saveTextToFile(jsonStr, fileName);
    if (!ok) abort("Failed to save response to file: " + fileName);
}

void ALightResponse_SI::loadResponseModel(QString fileName)
{
    clearModel();
    QString jsonStr;
    bool ok = ftools::loadTextFromFile(jsonStr, fileName);
    if (!ok) abort("Failed to load response from file: " + fileName);
    else LRHub.Model = new LRModel(jsonStr.toLatin1().data());
}

void ALightResponse_SI::makeSensorGroups(QString type, int numNodes)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if      (type == "Common")    LRHub.Model->MakeGroupsCommon();
    if      (type == "ByRadius")  LRHub.Model->MakeGroupsByRadius();
    else if (type == "Rectangle") LRHub.Model->MakeGroupsRectangle();
    else if (type == "Square")    LRHub.Model->MakeGroupsSquare();
    else if (type == "Hexagon")   LRHub.Model->MakeGroupsHexagon();
    else if (type == "Polygon")   LRHub.Model->MakeGroupsNgon(numNodes);
    else
    {
        abort("Unknow groupping type! Available options are:\nCommon, ByRadius, Rectangle, Square, Hexagon and Polygon");
        return;
    }

    if (!CommonJsonString.isEmpty()) setLRF(CommonJsonString);
}

QString ALightResponse_SI::newLRF_axial(int intervals, double minR, double maxR)
{
    LRFaxial lrf(maxR, intervals);
    lrf.SetRmin(minR);
    return QString(lrf.GetJsonString().data());
}

QString ALightResponse_SI::configureLRF_AxialCompression(QString LRF, double k, double lambda, double r0)
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

QString ALightResponse_SI::configureLRF_Constrains(QString LRF, bool nonNegative, bool nonIncreasing, bool flattop)
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

#include "lrfxy.h"
QString ALightResponse_SI::newLRF_xy(int intervalsX, double minX, double maxX, int intervalsY, double minY, double maxY)
{
    LRFxy lrf(minX, maxX, intervalsX, minY, maxY, intervalsY);
    return QString(lrf.GetJsonString().data());
}

void ALightResponse_SI::setLRF(QString jsonString)
{
    if (!LRHub.Model)
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

    const size_t numGroups = LRHub.Model->GetGroupCount();
    for (size_t iGr = 0; iGr < numGroups; iGr++)
    {
        LRF * lrf = lrfToClone->clone();
        ifAxialUpdateLrfCenter(lrf, LRHub.Model->GetGroupX(iGr), LRHub.Model->GetGroupY(iGr));
        LRHub.Model->SetGroupLRF(iGr, lrf);
    }

    // find sensors not belonging to any group
    const ASensorHub & SensHub = ASensorHub::getConstInstance();
    const size_t numSens = LRHub.Model->GetSensorCount();
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        int iGr = LRHub.Model->GetGroup(iSens);
        if (iGr != -1) continue;

        LRF * lrf = lrfToClone->clone();
        ifAxialUpdateLrfCenter(lrf, SensHub.getSensorData(iSens)->Position[0], SensHub.getSensorData(iSens)->Position[1]);
        LRHub.Model->SetLRF(iSens, lrf);
    }

    CommonJsonString = jsonString;
}


void ALightResponse_SI::clearFitData()
{
    if (LRHub.Model) LRHub.Model->ClearAllFitData();
}

void ALightResponse_SI::addFitData(int iSensor, QVariantList xyza)
{
    if (!LRHub.Model) return;

    const size_t size = xyza.size();
    std::vector<std::array <double, 4>> data(size);
    for (size_t iEv = 0; iEv < size; iEv++)
    {
        QVariantList event = xyza[iEv].toList();
        for (size_t i = 0; i < 4; i++)
            data[iEv][i] = event[i].toDouble();
    }

    LRHub.Model->AddFitData(iSensor, data);
}

void ALightResponse_SI::fitSensor(int iSensor)
{
    if (LRHub.Model) LRHub.Model->FitSensor(iSensor);
}

void ALightResponse_SI::fitGroup(int iGroup)
{
    if (LRHub.Model) LRHub.Model->FitGroup(iGroup);
}

void ALightResponse_SI::plotLRF_radial(int iSensor, bool showNodes)
{
    ALrfDrawer dr(LRHub.Model);
    QString err = dr.drawRadial(iSensor, showNodes);
    if (!err.isEmpty()) abort(err);
}

void ALightResponse_SI::showResponseExplorer()
{
    if (!LRHub.Model) abort("Light response model is not defined");
    else emit AScriptHub::getInstance().requestShowLightResponseExplorer(LRHub.Model);
}





void ALightResponse_SI::fitResponse(QVariantList floodSignals, QVariantList floodPositions)
{
    if (!LRHub.Model)
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

    LRHub.Model->ClearAllFitData();

    // preparing gitting data
    const size_t numSens = LRHub.Model->GetSensorCount();
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

    const size_t numGroups = LRHub.Model->GetGroupCount();
    bool estimateGains = (LRHub.Model->GetGroupCount() > 0);
    GainEstimator * gainEstimator = nullptr;
    if (estimateGains) gainEstimator = new GainEstimator(LRHub.Model);

    // adding fitting data
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        std::vector<std::array <double, 4>> thisData(data);
        for (size_t iEv = 0; iEv < numEv; iEv++)
        {
            thisData[iEv][3] = arSig[iEv][iSens];
        }
        LRHub.Model->AddFitData(iSens, thisData);
        if (estimateGains) gainEstimator->AddData(iSens, thisData);
    }

    //fitting response for sensors not belonging to any groups
    for (size_t iSens = 0; iSens < numSens; iSens++)
    {
        if (LRHub.Model->GetGroup(iSens) == -1)
            LRHub.Model->FitSensor(iSens);
    }

    // fitting response for groups
    for (size_t iGr = 0; iGr < numGroups; iGr++)
        LRHub.Model->FitGroup(iGr);

    if (estimateGains)
    {
        for (size_t iGr = 0; iGr < numGroups; iGr++)
        {
            //qDebug() << "-----\n" << iGr;
            std::set<int> & groupSet = LRHub.Model->GroupMembers(iGr);
            const int size = groupSet.size();
            std::vector<int> groupVec;
            groupVec.reserve(size);
            for (auto itr = groupSet.begin(); itr != groupSet.end(); itr++)
                groupVec.push_back(*itr);
            //qDebug() << "Sensors:" << groupVec;

            std::vector<double> relGains = gainEstimator->GetRelativeGainsList(groupVec, groupVec.front());
            //qDebug() << "Inverse (apparently) gains:" << "-->" << relGains;
            double sumGain = 0;
            for (size_t i = 0; i < size; i++)
                sumGain += 1.0/relGains[i];
            sumGain /= size;

            for (size_t i = 0; i < size; i++)
            {
                LRHub.Model->SetGain(groupVec[i], 1.0/relGains[i]/sumGain);
                //qDebug() << groupVec[i] << "-->" << 1.0/relGains[i]/sumGain;
            }
        }
    }

    // dests
    delete gainEstimator;
}



// --- Low level ---

void ALightResponse_SI::enableSensor(int iSensor, bool enableFlag)
{
    if (!LRHub.Model)
    {
        abort("Light response model is not defined");
        return;
    }
    if (iSensor < 0 || iSensor >= LRHub.Model->GetSensorCount())
    {
        abort("Bad sensor index: " + QString::number(iSensor));
        return;
    }

    if (enableFlag) LRHub.Model->SetEnabled(iSensor);
    else            LRHub.Model->SetDisabled(iSensor);
}

void ALightResponse_SI::clearGroups()
{
    if (LRHub.Model) LRHub.Model->ResetGroups();
}

int ALightResponse_SI::countGroups()
{
    if (LRHub.Model) return LRHub.Model->GetGroupCount();
    else return 0;
}

void ALightResponse_SI::setLRF_Sensor(int iSensor, QString jsonString)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if (iSensor < 0 || iSensor >= LRHub.Model->GetSensorCount())
    {
        abort("SetLRF_Sensor: invalid sensor index");
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

    LRHub.Model->SetJsonLRF(iSensor, jsonString.toLatin1().data());
}

void ALightResponse_SI::setLRF_Group(int iGroup, QString jsonString)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if (iGroup < 0 || iGroup >= LRHub.Model->GetGroupCount())
    {
        abort("SetLRF_Group: invalid group index");
        return;
    }

    QJsonObject json = jstools::strToJson(jsonString);
    if (json["type"] == "Axial")
    {
        if (!json.contains("x0")) json["x0"] = LRHub.Model->GetGroupX(iGroup);
        if (!json.contains("y0")) json["y0"] = LRHub.Model->GetGroupY(iGroup);
        jsonString = jstools::jsonToString(json);
    }

    LRHub.Model->SetGroupJsonLRF(iGroup, jsonString.toLatin1().data());
}

void ALightResponse_SI::setModelGains(QVariantList gains)
{
    if (!LRHub.Model)
    {
        abort("Light response model is not defined");
        return;
    }
    size_t size = gains.size();
    if (size != LRHub.Model->GetSensorCount())
    {
        abort("gains array length is not equal to the number of sensors");
        return;
    }

    for (int i = 0; i < size; i++)
        LRHub.Model->SetGain(i, gains[i].toDouble());
}

QVariantList ALightResponse_SI::getModelGains()
{
    QVariantList vl;
    if (!LRHub.Model)
    {
        abort("Light response model is not defined");
        return vl;
    }

    int size = LRHub.Model->GetSensorCount();
    for (int i = 0; i < size; i++)
        vl << LRHub.Model->GetGain(i);
    return vl;
}

double ALightResponse_SI::eval(int iSensor, double x, double y, double z)
{
    if (LRHub.Model) return LRHub.Model->Eval(iSensor, x, y, z);
    else return 0;
}

double ALightResponse_SI::eval(int iSensor, QVariantList xyz)
{
    if (xyz.length() != 3) return 0;

    double pos[3];
    for (size_t i = 0; i < 3; i++) pos[i] = xyz[i].toDouble();

    if (LRHub.Model) return LRHub.Model->Eval(iSensor, pos);
    else return 0;
}

// --- private methods ---

void ALightResponse_SI::clearModel()
{
    delete LRHub.Model; LRHub.Model = nullptr;

    CommonJsonString.clear();
}

void ALightResponse_SI::ifAxialUpdateLrfCenter(LRF *lrf, double x, double y)
{
    LRFaxial * axlrf = dynamic_cast<LRFaxial*>(lrf);
    if (axlrf) axlrf->SetOrigin(x, y);
}
