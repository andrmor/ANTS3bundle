#include "alightresponse_si.h"
#include "alightresponsehub.h"
#include "asensorhub.h"
#include "ascripthub.h"
#include "ajsontools.h"
#include "alrfplotter.h"
#include "afiletools.h"

#include "lrmodel.h"
#include "lrfaxial.h"

ALightResponse_SI::ALightResponse_SI() :
    LRHub(ALightResponseHub::getInstance())
{
    Description = "A module for position response parameterization based on 'Mercury' library of Vladimir Solovov.\n"
                  "Documentation can be found here:\n"
                  "https://mercurydocs.readthedocs.io/en/latest/index.html";

    // High level

    Help["newResponseModel"] = "Define a new response model with the sensor xyz positions defined by the sensorPositions array.\n"
                               "The sensor LRFs have to be configured by the user, as wll as, optionally, the division of sensors into groups.";

    Help["loadResponseModel"] = "Load the response model previously saved to a file";
    Help["saveResponseModel"] = "Save the currently defined response model to a file";

    Help["defineSensorGroups"] = "Define the sensor groups of the response model based on a specific symmetry of the sensor array, given by the first argument.\n"
                                 "The valid options are: 'Common', 'ByRadius', 'Rectangle', 'Square', 'Hexagon' and 'Polygon'.\n"
                                 "In the case of polygon, the second argument defines the number of polygon edges";

    Help["newLRF_axial"] = "Creates an 'Axial' LRF and returns its json string.\n"
                           "The arguments define the number of nodes, as well as the radial range. It is generally a good idea to have minR = 0";
    Help["newLRF_axial3D"] = "Creates an 'Axial3D' LRF and returns its json string.\n"
                             "The arguments define the number of nodes in radial direction, the radial range, the number of intervals in Z direction and the Z range";
    Help["newLRF_xy"] = "Creates an LRF of type 'LRFxy' and returns its json string.\n"
                        "The arguments define the number of intervals and the corresponding ranges in X and Y directions";
    Help["newLRF_xy"] = "Creates an LRF of type 'LRFxyz' and returns its json string.\n"
                        "The arguments define the number of intervals and the corresponding ranges in X, Y and Z directions";

    Help["configureLRF_AxialCompression"] = "Add compression information to an existent LRF of 'Axial' or 'Axial3D' types.\n"
                                            "Note that this method returns the modified json string of the initial LRF with added compression!";

    Help["configureLRF_Constrains"] = "Add constrains on LRFs to be used in LRF fitting process.\n"
                                      "Note that this method returns the modified json string of the initial LRF with added constrain info!";

    Help["setLRF"] = "Set default LRF for all sensors and sensor groups (defined or to be defined)";

    Help["fitResponse"] = "Fit response model using the provided flood-field irradiation data.\n"
                          "floodSignals array lists, per event, amplitude for all sensors;\n"
                          "floodPosition array lists, per event, true or guessed [XYZ] positions;\n"
                          "optional goodEventFlag array lists, per event, boolean values serving as a 'bad event' flag:\n"
                          "false value identifies those events which have to be disregarded in fitting.";

    Help["showResponseExplorer"] = "Shows a GUI widget which plots the sensor array and visualizes the sensor amplitudes (LRF values) based on the position of the source, given by the current mouse position";

    Help["showLrfPlotterWidget"] = "Shows a GUI widget which plots the LRF of a given sensor, as a function of radial or XY position.\n"
                                   "If optional sensorSignals and eventPositions arguments are supplied (by-event arrays of sensor signals and the corresponding position),\n"
                                   "the widget can be used to visually explore the difference between the LRFs and the signal data";

    // Low level

    Help["enableSensor"] = "Enable or disable a given sensor. Disabled sensors are not used in position reconstruction";

    Help["countSensors"] = "Return number of sensors in the current response model";
    Help["countGroups"] = "Return number of sensors groups defined in the current response model";
    Help["getGroupMembers"] = "Return indexes of the sensors belonging to a given sensor group";

    Help["setLrf_Sensor"] = "Set LRF for a given sensor.\n"
                            "Warning! It is not recommended to mix the 'high level' setLRF method and 'low level' setLrf_Sensor and setLrf_Group methods";
    Help["setLrf_Group"] = "Set LRF for a given sensor group.\n"
                            "Warning! It is not recommended to mix the 'high level' setLRF method and 'low level' setLrf_Sensor and setLrf_Group methods";

    Help["getSensorGain"] = "Return the sensor gain (within its group!) for a given sensor";
    Help["setSensorGain"] = "Sets the sensor gain (within its group!) for a given sensor";

    Help["clearFitData"] = "Clears all fitting data previously defined for sensors";
    Help["addFitData"] = "Add fit data for a given sensor.\n"
                         "'amplitudes' array lists, per event, amplitude for all sensors;\n"
                         "'positions' array lists, per event, true or guessed [XYZ] positions;\n"
                          "optional goodEventFlag array lists, per event, boolean values serving as a 'bad event' flag:\n"
                          "false value identifies those events which have to be disregarded in fitting.";
    Help["fitSensor"] = "Fit data (perform LRF paramerization) for a given sensor and previously provided fit data (see addFitData method).";
    Help["fitGroup"] = "Fit data (perform LRF paramerization) for a given sensor group and previously provided fit data for all the sensors of that group (see addFitData method).";

    Help["evaluateLrf"] = "Return the LRF value for the sensor, assuming that the source is at the given position";

    Help["getModel"] = "Return json string of the currently defined response model";
    Help["setModel"] = "Configures the response model based on the configuration provided as json string";
}

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

void ALightResponse_SI::saveResponseModel(QString fileName)
{
    if (!checkModel()) return;
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

void ALightResponse_SI::defineSensorGroups(QString type, int numNodes)
{
    if (!checkModel()) return;

    if      (type == "Common")    LRHub.Model->MakeGroupsCommon();
    else if (type == "ByRadius")  LRHub.Model->MakeGroupsByRadius();
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
    if (json["type"] != "Axial" || json["type"] != "Axial3D")
    {
        abort("Compression can be applied only to Axial and Axial3D LRF types");
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

#include "lrfaxial3d.h"
QString ALightResponse_SI::newLRF_axial3D(int intervalsR, double minR, double maxR, int intervalsZ, double minZ, double maxZ)
{
    LRFaxial3d lrf(maxR, intervalsR, minZ, maxZ, intervalsZ);
    lrf.SetRmin(minR);
    return QString(lrf.GetJsonString().data());
}

#include "lrfxy.h"
QString ALightResponse_SI::newLRF_xy(int intervalsX, double minX, double maxX, int intervalsY, double minY, double maxY)
{
    LRFxy lrf(minX, maxX, intervalsX, minY, maxY, intervalsY);
    return QString(lrf.GetJsonString().data());
}

#include "lrfxyz.h"
QString ALightResponse_SI::newLRF_xyz(int intervalsX, double minX, double maxX,
                                      int intervalsY, double minY, double maxY,
                                      int intervalsZ, double minZ, double maxZ)
{
    LRFxyz lrf(minX, maxX, intervalsX,
               minY, maxY, intervalsY,
               minZ, maxZ, intervalsZ);
    return QString(lrf.GetJsonString().data());
}

void ALightResponse_SI::setLRF(QString jsonString)
{
    if (!checkModel()) return;

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

void ALightResponse_SI::addFitData(int iSensor, QVariantList amplitudes, QVariantList positions, QVariantList goodEventFlag)
{
    if (!checkModelAndSensor(iSensor)) return;

    bool haveGood = !goodEventFlag.empty();

    const qsizetype numEvents = amplitudes.size();
    if (positions.size() != numEvents ||
        (haveGood && goodEventFlag.size() != numEvents))
    {
        abort("addFitData: mismtach in array sizes");
        return;
    }

    std::vector<std::array<double, 3>> xyz(numEvents);
    std::vector<double>                a(numEvents);
    std::vector<bool>                  good(numEvents, true);

    for (qsizetype iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList event = positions[iEv].toList();
        if (event.size() < 3)
        {
            abort("addFitData: 'positions' argument should be array of [x, y, z] arrays");
            return;
        }

        for (size_t i = 0; i < 3; i++)
            xyz[iEv][i] = event[i].toDouble();

        a[iEv] = amplitudes[iEv].toDouble();

        if (haveGood)
            good[iEv] = goodEventFlag[iEv].toDouble();
    }

    bool ok = LRHub.Model->AddFitRawData(iSensor, xyz, a, good);
    if (!ok)
    {
        abort("addFitData: failed to add data");
        return;
    }
}

void ALightResponse_SI::fitSensor(int iSensor)
{
    if (!checkModelAndSensor(iSensor)) return;
    LRHub.Model->FitSensor(iSensor);
    LRF * lrf = LRHub.Model->GetLRF(iSensor);
    if (!lrf || !lrf->isValid()) abort("Failed to fit LRF for sensor # " + QString::number(iSensor));
}

void ALightResponse_SI::fitGroup(int iGroup)
{
    if (!checkModelAndGroup(iGroup)) return;
    LRHub.Model->FitGroup(iGroup);
    LRF * lrf = LRHub.Model->GetGroupLRF(iGroup);
    if (!lrf || !lrf->isValid()) abort("Failed to fit LRF for sensor group # " + QString::number(iGroup));
}

void ALightResponse_SI::showResponseExplorer()
{
    if (!checkModel()) return;
    emit AScriptHub::getInstance().requestShowLightResponseExplorer(LRHub.Model);
}

#include "alrfplotterdialog.h"
void ALightResponse_SI::showLrfPlotterWidget(QVariantList sensorSignals, QVariantList eventPositions)
{
    if (!checkModel()) return;

    if (sensorSignals.empty() && eventPositions.empty()) return; // using already defined data
    const size_t numEvents = sensorSignals.size();
    if (numEvents != eventPositions.size())
    {
        abort("Size mismatch of arrays in configure_plotLRF");
        return;
    }

    std::vector<std::vector<double>>  DataSignals(numEvents);
    std::vector<std::array<double,4>> DataPositions(numEvents);
    int numSensors = -1;
    int numPos = 3; // 3 for XYZ and 4 for XYZE
    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList elSignals = sensorSignals[iEv].toList();
        if (iEv == 0)
        {
            numSensors = elSignals.size();
            if (numSensors <= 0)
            {
                abort("configure_plotLRF: the sensorSignals array should contain data for at least one sensor");
                return;
            }
        }
        else if (elSignals.size() < numSensors)
        {
            abort("configure_plotLRF: the sensorSignals array contains invalid number of sensors at event number " + QString::number(iEv));
            return;
        }
        std::vector<double> subvecSignals(numSensors);
        for (size_t iSens = 0; iSens < numSensors; iSens++)
            subvecSignals[iSens] = elSignals[iSens].toDouble();

        QVariantList elPositions = eventPositions[iEv].toList();
        if (iEv == 0)
        {
            numPos = elPositions.size();
            if (numPos < 3 || numPos > 4)
            {
                abort("configure_plotLRF: eventPositions array should contain sub-arrays with XYZ or XYZEnergy data!");
                return;
            }
        }
        else if (elPositions.size() != numPos)
        {
            abort("configure_plotLRF: eventPositions array should contain sub-arrays with XYZ or XYZEnergy data");
            return;
        }
        std::array<double,4> subarPositions;
        for (size_t i = 0; i < numPos; i++)
            subarPositions[i] = elPositions[i].toDouble();

        DataSignals[iEv]   = subvecSignals;
        DataPositions[iEv] = subarPositions;
    }

    if (numPos < 4)
        for (size_t iEv = 0; iEv < numEvents; iEv++)
            DataPositions[iEv][3] = 1.0;

    // optional checks if needed

    LRHub.LrfPlotter->DataSignals = DataSignals;
    LRHub.LrfPlotter->DataPositions = DataPositions;

    emit AScriptHub::getInstance().requestShowPlotterDialog(LRHub.LrfPlotter);
}

void ALightResponse_SI::showLrfPlotterWidget()
{
    if (!checkModel()) return;

    LRHub.LrfPlotter->DataSignals.clear();
    LRHub.LrfPlotter->DataPositions.clear();

    emit AScriptHub::getInstance().requestShowPlotterDialog(LRHub.LrfPlotter);
}

void ALightResponse_SI::fitResponse(QVariantList floodSignals, QVariantList floodPositions, QVariantList goodEventFlag)
{
    if (!checkModel()) return;

    const qsizetype numEv = floodSignals.size();
    if (numEv != floodPositions.size())
    {
        abort("Mismatch in the sizes of the flood signal and position arrays");
        return;
    }

    bool haveFilter = !goodEventFlag.empty();
    if (haveFilter)
        if (numEv != goodEventFlag.size())
        {
            abort("Mismatch in the sizes of the flood data and goodEventFlag arrays");
            return;
        }

    LRHub.Model->ClearAllFitData();

    // preparing fitting data
    const qsizetype numSens = LRHub.Model->GetSensorCount();
    std::vector<std::array<double, 3>> vecPositions(numEv);
    std::vector<std::vector<double>>   vecSignals(numEv);
    std::vector<bool>                  vecGood(numEv, true);
    for (qsizetype iEv = 0; iEv < numEv; iEv++)
    {
        QVariantList eventPos = floodPositions[iEv].toList();
        if (eventPos.size() < 3)
        {
            abort("Bad format of floodPositions array");
            return;
        }
        for (size_t i = 0; i < 3; i++)
            vecPositions[iEv][i] = eventPos[i].toDouble();

        vecSignals[iEv].resize(numSens);
        QVariantList eventSignals = floodSignals[iEv].toList();
        if (eventSignals.size() != numSens)
        {
            abort("Bad format of floodSignals array");
            return;
        }
        for (qsizetype iSens = 0; iSens < numSens; iSens++)
            vecSignals[iEv][iSens] = eventSignals[iSens].toDouble();

        if (haveFilter) vecGood[iEv] = goodEventFlag[iEv].toBool();
    }

    const size_t numGroups = LRHub.Model->GetGroupCount();
    bool estimateGains = (LRHub.Model->GetGroupCount() > 0);
    GainEstimator * gainEstimator = nullptr;
    if (estimateGains) gainEstimator = new GainEstimator(LRHub.Model);

    // adding fitting data
    std::vector<double> vecOneSensorSignal(numEv);
    for (qsizetype iSens = 0; iSens < numSens; iSens++)
    {
        for (qsizetype iEv = 0; iEv < numEv; iEv++)
            vecOneSensorSignal[iEv] = vecSignals[iEv][iSens];
        LRHub.Model->AddFitRawData(iSens, vecPositions, vecOneSensorSignal, vecGood);
        if (estimateGains) gainEstimator->AddRawData(iSens, vecPositions, vecOneSensorSignal, vecGood);
    }

    //fitting response for sensors not belonging to any groups
    for (qsizetype iSens = 0; iSens < numSens; iSens++)
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
    if (!checkModelAndSensor(iSensor)) return;

    if (enableFlag) LRHub.Model->SetEnabled(iSensor);
    else            LRHub.Model->SetDisabled(iSensor);
}

int ALightResponse_SI::countSensors()
{
    if (LRHub.Model) return LRHub.Model->GetSensorCount();
    else return 0;
}

int ALightResponse_SI::countGroups()
{
    if (LRHub.Model) return LRHub.Model->GetGroupCount();
    else return 0;
}

QVariantList ALightResponse_SI::getGroupMembers(int iGroup)
{
    QVariantList vl;
    if (!checkModelAndGroup(iGroup)) return vl;

    const std::set<int> & memSet = LRHub.Model->GroupMembers(iGroup);
    std::for_each(memSet.begin(), memSet.end(), [&vl](const int & n){vl.push_back(n);});
    return vl;
}

void ALightResponse_SI::setLrf_Sensor(int iSensor, QString jsonString)
{
    if (!checkModelAndSensor(iSensor)) return;

    QJsonObject json = jstools::strToJson(jsonString);
    if (json["type"] == "Axial" || json["type"] == "Axial3D")
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

void ALightResponse_SI::setLrf_Group(int iGroup, QString jsonString)
{
    if (!checkModelAndGroup(iGroup)) return;

    QJsonObject json = jstools::strToJson(jsonString);
    if (json["type"] == "Axial" || json["type"] == "Axial3D")
    {
        if (!json.contains("x0")) json["x0"] = LRHub.Model->GetGroupX(iGroup);
        if (!json.contains("y0")) json["y0"] = LRHub.Model->GetGroupY(iGroup);
        jsonString = jstools::jsonToString(json);
    }

    LRHub.Model->SetGroupJsonLRF(iGroup, jsonString.toLatin1().data());
}

void ALightResponse_SI::setSensorGain(int iSensor, double gain)
{
    if (!checkModelAndSensor(iSensor)) return;
    LRHub.Model->SetGain(iSensor, gain);
}

double ALightResponse_SI::getSensorGain(int iSensor)
{
    if (!checkModelAndSensor(iSensor)) return 0;
    return LRHub.Model->GetGain(iSensor);
}

double ALightResponse_SI::evaluateLrf(int iSensor, double x, double y, double z)
{
    if (LRHub.Model) return LRHub.Model->Eval(iSensor, x, y, z);
    else return 0;
}

double ALightResponse_SI::evaluateLrf(int iSensor, QVariantList xyz)
{
    if (xyz.length() != 3) return 0;

    double pos[3];
    for (size_t i = 0; i < 3; i++) pos[i] = xyz[i].toDouble();

    if (LRHub.Model) return LRHub.Model->Eval(iSensor, pos);
    else return 0;
}

QString ALightResponse_SI::getModel()
{
    if (!checkModel()) return "";
    return LRHub.Model->GetJsonString().data();
}

void ALightResponse_SI::setModel(QString jsonString)
{
    clearModel();
    LRHub.Model = new LRModel(jsonString.toLatin1().data());
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

bool ALightResponse_SI::checkModel()
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return false;
    }
    return true;
}

bool ALightResponse_SI::checkModelAndSensor(int iSensor)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return false;
    }
    if (iSensor < 0 || iSensor >= LRHub.Model->GetSensorCount())
    {
        abort("Invalid sensor index: " + QString::number(iSensor));
        return false;
    }
    return true;
}

bool ALightResponse_SI::checkModelAndGroup(int iGroup)
{
    if (!LRHub.Model)
    {
        abort("Model was not created yet!");
        return false;
    }
    if (iGroup < 0 || iGroup >= LRHub.Model->GetGroupCount())
    {
        abort("Invalid sensor group index: " + QString::number(iGroup));
        return false;
    }

    return true;
}
