#ifndef ALUTINTERFACERULE_H
#define ALUTINTERFACERULE_H

#include "ainterfacerule.h"

#include <vector>
#include <array>

class AHistogram1D;
class AGeoMeshHandler;
class TVector3;

class ALutInterfaceRule : public AInterfaceRule
{
public:
    ALutInterfaceRule(int MatFrom, int MatTo);

    EInterfaceRuleResult calculate(APhoton * Photon, const double * NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "LUT";}
    QString getAbbreviation() const override {return "LUT";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;
    QString getDescription() const override;

    bool    canHaveRoughSurface() const override {return false;}

    // at least DataReflection or DataTransmission should be non-empty
    // pair: [incidentTheta, ProbabilityMeshData]
    // if only one element in top array, all incident angles have the same behavior
    // non-empty top vectors should have the same size
    // inner vectors can be empty (there is zero reflection or transmission probability for a given incidence angle)
    std::vector<std::pair<double,std::vector<double>>> DataReflection;      // can be empty
    std::vector<std::pair<double,std::vector<double>>> DataTransmission;    // can be empty
    std::vector<std::pair<double,double>> DataAbsorption;                   // can be empty

    QString loadLUT(const QJsonObject & json);
    QString check();
    AGeoMeshHandler * getTransMesh() {return _MeshTransmission;}

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override; // !!!*** error reporting

    QString doCheckOverrideData() override; // !!!*** check increasing order in angle

    // runtime
    // vs incident angle bin
    std::vector<std::array<double,3>> _AbsRefTransVsTheta; // relative probabilities
    std::vector<AHistogram1D*> _HistReflections;
    std::vector<AHistogram1D*> _HistTransmissions;

    AGeoMeshHandler * _MeshReflections = nullptr;
    AGeoMeshHandler * _MeshTransmission = nullptr;

    bool _GloballyNoReflection  = false;
    bool _GloballyNoTransmission = false;
    bool _GloballyNoAbsorption  = false;
    size_t _NumberIncidentAngleBins = 0;
    std::vector<double> _DefinedIncidentThetaValues;

    void generateRandomPointInTriangle(const std::array<double, 3> & A, const std::array<double, 3> & B, const std::array<double, 3> & C, std::array<double, 3> & result);
    void reflectedLocalToGlobal(const TVector3 & nHat, const TVector3 & aHat, const TVector3 & B_local, TVector3 & photOutGlobal);
    size_t getClosestInboundThetaIndex(double theta_deg);
};

#endif // ALUTINTERFACERULE_H
