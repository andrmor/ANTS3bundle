#include "SessionManager.hh"
#include "arandomg4hub.h"
#include "asourceparticlegenerator.h"
#include "afileparticlegenerator.h"
#include "aerrorhub.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4UImanager.hh"

#include "Randomize.hh"  // !!!*** need?

//#include <QDebug>

SessionManager &SessionManager::getInstance()
{
    static SessionManager instance; // Guaranteed to be destroyed, instantiated on first use.
    return instance;
}

SessionManager::SessionManager()
{
    std::vector<std::string> allElements = {"H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg","Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr","Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr","Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf","Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po","At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm","Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs"};

    for (size_t i = 0; i < allElements.size(); i++)
        ElementToZ.emplace( std::make_pair(allElements[i], i+1) );
}

SessionManager::~SessionManager()
{
    delete outStreamExit;
    delete outStreamDeposition;
    delete outStreamCalorimeterLog;
    delete outStreamHistory;
}

#ifdef ANTS3_NCRYSTAL
#include "G4NCrystal/G4NCrystal.hh"
#endif
void SessionManager::startSession()
{
    prepareParticleGun();

    prepareMonitors(); // populating particle pointers
    prepareAnalyzers();

    // preparing ouptut for deposition data
    if (Settings.RunSet.SaveDeposition) prepareOutputDepoStream();

    // preparing ouptut for track export
    if (Settings.RunSet.SaveTrackingHistory) prepareOutputHistoryStream();

    // preparing ouptut for exiting particle export
    if (Settings.RunSet.SaveSettings.Enabled) prepareOutputExitStream();

    // preparing ouptut for exiting particle export
    if (Settings.RunSet.CalorimeterSettings.Enabled && Settings.RunSet.CalorimeterSettings.SaveEnergyDepositionLog) prepareOutputCalorimeterLogStream();

    //set random generator
    ARandomHub::getInstance().init(Settings.RunSet.Seed);

    executeAdditionalCommands();

    if (Settings.RunSet.SaveSettings.Enabled) findExitVolume();

    if (Settings.G4Set.UseNCrystal)
#ifdef ANTS3_NCRYSTAL
        G4NCrystal::installOnDemand();
#else
        terminateSession("Cannot activate NCrystal: g4ants3 was compiled without NCrystal library enabled!");
#endif
}

void SessionManager::prepareParticleGun()
{
    switch (Settings.GenerationMode)
    {
    case AParticleSimSettings::Sources :
        for (AParticleSourceRecordBase * source : Settings.SourceGenSettings.SourceData)
        {
            AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(source);
            if (stSource)
            {
                for (AGunParticle & particle : stSource->Particles)
                    particle.particleDefinition = SessionManager::findGeant4Particle(particle.Particle); // terminate inside if not found
            }
        }
        ParticleGenerator = new ASourceParticleGenerator(Settings.SourceGenSettings);
        break;
    case AParticleSimSettings::File :
        Settings.FileGenSettings.FileName = WorkingDir + '/' + Settings.FileGenSettings.FileName;
        ParticleGenerator = new AFileParticleGenerator(Settings.FileGenSettings);
        break;
    default :
        terminateSession("Unknown or not-implemented primary generation mode");
    }

    bool ok = ParticleGenerator->init();
    if (!ok) terminateSession(AErrorHub::getError());
    ParticleGenerator->setStartEvent(Settings.RunSet.EventFrom);
}

void SessionManager::terminateSession(const std::string & ReturnMessage)
{
    std::cout << "$$>" << ReturnMessage << std::endl; // will flush

    bError = true;
    ErrorMessage = ReturnMessage;
    generateReceipt();

    exit(1);
}

void SessionManager::endSession()
{
    bError = false;
    ErrorMessage.clear();

    if (Settings.RunSet.MonitorSettings.Enabled)     storeMonitorsData();
    if (Settings.RunSet.CalorimeterSettings.Enabled) storeCalorimeterData();
    if (Settings.RunSet.AnalyzerSettings.Enabled)    storeAnalyzerData();

    generateReceipt();
}

void SessionManager::runSimulation()
{
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    CurrentEvent = Settings.RunSet.EventFrom;
    int NumEvents = Settings.RunSet.EventTo - Settings.RunSet.EventFrom;

    //SM.runManager->BeamOn(NumEvents);
    UImanager->ApplyCommand("/run/beamOn " + std::to_string(NumEvents));
}

void SessionManager::onEventFinished()
{
    if (!DirectDepositionBuffer.empty())
    {
        // when particle gun is prepared, the Event action did not start yet so th event start marker is not yet saved
        // the output is stored in the temporary buffer
        for (TmpDepositionBuffer & rec : DirectDepositionBuffer)
            saveDepoRecord(rec.Name, rec.iMat, rec.EDep, rec.Pos, rec.Time, rec.iCopyNumber);

        DirectDepositionBuffer.clear();
    }

    if (Settings.RunSet.CalorimeterSettings.Enabled && Settings.RunSet.CalorimeterSettings.SaveEnergyDepositionLog)
        saveCalorimeterLogEntry();

    //EventId = NextEventId;
    CurrentEvent++;

    int EventsDone = CurrentEvent - Settings.RunSet.EventFrom;
    std::cout << "$$>" << EventsDone << "<$$" << std::endl; // endl also flushes!
}

void SessionManager::reportUnknownMaterial(const G4Material * material)
{
    terminateSession("Found deposition in materials not listed in the config json: " + material->GetName());
}

#include "G4ProcessManager.hh"
#include "G4HadronElasticProcess.hh"
#include "G4NeutronHPElasticData.hh"
#include "G4NeutronHPThermalScatteringData.hh"
#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPThermalScattering.hh"
#include "G4ParticleHPInelasticData.hh"
#include "G4ParticleHPInelastic.hh"
#include "G4ParticleHPCaptureData.hh"
#include "G4ParticleHPCapture.hh"
#include "G4ParticleHPFissionData.hh"
#include "G4ParticleHPFission.hh"
#include "G4SystemOfUnits.hh"

#ifdef GEANT_VERSION_FROM_11
// starting from Geant4 v11
    #include "G4ParticleHPElasticData.hh"
    #include "G4ParticleHPThermalScatteringData.hh"
    #include "G4ParticleHPElastic.hh"
    #include "G4ParticleHPThermalScattering.hh"
    #include "G4HadronInelasticProcess.hh"
    #include "G4NeutronCaptureProcess.hh"
    #include "G4NeutronFissionProcess.hh"
#else
// before Geant4 v11
    #include "G4NeutronInelasticProcess.hh"
    #include "G4HadronCaptureProcess.hh"
    #include "G4HadronFissionProcess.hh"
#endif

bool SessionManager::activateNeutronThermalScatteringPhysics()
{
    if (!Settings.G4Set.UseTSphys) return false;

    // based on Hadr04 example of Geant4

    G4ParticleDefinition* neutron = G4Neutron::Neutron();
    G4ProcessManager* pManager = neutron->GetProcessManager();
    if (!pManager)
    {
        terminateSession("Process manager for neutron not found!");
        return false;
    }

#ifdef GEANT_VERSION_FROM_11
    //After v11

    // delete all neutron processes if already registered
    G4VProcess* process = 0;
    process = pManager->GetProcess("hadElastic");       if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("neutronInelastic"); if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nCapture");         if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nFission");         if (process) pManager->RemoveProcess(process);

    // (re) create process: elastic
    G4HadronElasticProcess* process1 = new G4HadronElasticProcess();
    pManager->AddDiscreteProcess(process1);
    //
    // model1a
    G4ParticleHPElastic*  model1a = new G4ParticleHPElastic();
    process1->RegisterMe(model1a);
    process1->AddDataSet(new G4ParticleHPElasticData());
    //
    // model1b
    model1a->SetMinEnergy(4*eV);
    G4ParticleHPThermalScattering* model1b = new G4ParticleHPThermalScattering();
    process1->RegisterMe(model1b);
    process1->AddDataSet(new G4ParticleHPThermalScatteringData());

    // (re) create process: inelastic
    //
    G4HadronInelasticProcess* process2 =
            new G4HadronInelasticProcess( "neutronInelastic", G4Neutron::Definition() );
    pManager->AddDiscreteProcess(process2);
    //
    // cross section data set
    G4ParticleHPInelasticData* dataSet2 = new G4ParticleHPInelasticData();
    process2->AddDataSet(dataSet2);
    //
    // models
    G4ParticleHPInelastic* model2 = new G4ParticleHPInelastic();
    process2->RegisterMe(model2);

    // (re) create process: nCapture
    //
    G4NeutronCaptureProcess* process3 = new G4NeutronCaptureProcess();
    pManager->AddDiscreteProcess(process3);
    //
    // cross section data set
    G4ParticleHPCaptureData* dataSet3 = new G4ParticleHPCaptureData();
    process3->AddDataSet(dataSet3);
    //
    // models
    G4ParticleHPCapture* model3 = new G4ParticleHPCapture();
    process3->RegisterMe(model3);

    // (re) create process: nFission
    //
    G4NeutronFissionProcess* process4 = new G4NeutronFissionProcess();
    pManager->AddDiscreteProcess(process4);
    //
    // cross section data set
    G4ParticleHPFissionData* dataSet4 = new G4ParticleHPFissionData();
    process4->AddDataSet(dataSet4);
    //
    // models
    G4ParticleHPFission* model4 = new G4ParticleHPFission();
    process4->RegisterMe(model4);
#else
// -- Geant4 before v11
    // delete all neutron processes which are already registered
    G4VProcess* process = nullptr;
    process = pManager->GetProcess("hadElastic");       if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("neutronInelastic"); if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nCapture");         if (process) pManager->RemoveProcess(process);
    process = pManager->GetProcess("nFission");         if (process) pManager->RemoveProcess(process);

    // (re) create process: elastic
    G4HadronElasticProcess* process1 = new G4HadronElasticProcess();
    pManager->AddDiscreteProcess(process1);
    G4ParticleHPElastic*  model1a = new G4ParticleHPElastic();
    model1a->SetMinEnergy(4*eV);
    process1->RegisterMe(model1a);
    process1->AddDataSet(new G4ParticleHPElasticData());
    G4ParticleHPThermalScattering* model1b = new G4ParticleHPThermalScattering();
    process1->RegisterMe(model1b);
    process1->AddDataSet(new G4ParticleHPThermalScatteringData());

    // (re) create process: inelastic
    G4NeutronInelasticProcess* process2 = new G4NeutronInelasticProcess();
    pManager->AddDiscreteProcess(process2);
    G4ParticleHPInelasticData* dataSet2 = new G4ParticleHPInelasticData();
    process2->AddDataSet(dataSet2);
    G4ParticleHPInelastic* model2 = new G4ParticleHPInelastic();
    process2->RegisterMe(model2);

    // (re) create process: nCapture
    G4HadronCaptureProcess* process3 = new G4HadronCaptureProcess();
    pManager->AddDiscreteProcess(process3);
    G4ParticleHPCaptureData* dataSet3 = new G4ParticleHPCaptureData();
    process3->AddDataSet(dataSet3);
    G4ParticleHPCapture* model3 = new G4ParticleHPCapture();
    process3->RegisterMe(model3);

    // (re) create process: nFission
    G4HadronFissionProcess* process4 = new G4HadronFissionProcess();
    pManager->AddDiscreteProcess(process4);
    G4ParticleHPFissionData* dataSet4 = new G4ParticleHPFissionData();
    process4->AddDataSet(dataSet4);
    G4ParticleHPFission* model4 = new G4ParticleHPFission();
    process4->RegisterMe(model4);
#endif

    return true;
}

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
void SessionManager::setupMaterials()
{
    // The GDML file contains every ants3 material under its export name (AMaterialTransferRecord::GdmlName, unique by construction).
    // For every ants3 material index, a dedicated Geant4 material is configured or created:
    //   Direct   : the material from GDML, renamed to the original ants3 name, optionally with custom mean excitation energy
    //   Nist     : a material derived from the G4NistManager material (own G4Material object for every ants3 material!);
    //              it keeps the NIST name, since some Geant4 physics recognizes materials by name (e.g. HP thermal scattering uses "G4_WATER")
    //   NCrystal : material built by NCrystal, renamed to the original ants3 name
    // The material of the logical volumes is then replaced by pointer.

    const std::vector<AMaterialTransferRecord> & Records = Settings.RunSet.Materials;
    G4NistManager * man = G4NistManager::Instance();

    MaterialsByIndex.assign(Records.size(), nullptr);
    std::map<G4Material*, G4Material*> Replacements; // material from GDML -> final material

    for (size_t iMat = 0; iMat < Records.size(); iMat++)
    {
        const AMaterialTransferRecord & rec = Records[iMat];

        if (rec.GdmlName.empty()) terminateSession("GDML name is not provided for material " + rec.Name);
        G4Material * gdmlMat = G4Material::GetMaterial(rec.GdmlName, false);
        if (!gdmlMat) terminateSession("Material " + rec.Name + " is not found in the GDML file under the name " + rec.GdmlName);

        G4Material * mat = nullptr;
        switch (rec.Type)
        {
        case AMaterialTransferRecord::Direct :
            mat = gdmlMat;
            mat->SetName(rec.Name);
            if (rec.UseCustomMeanExEnergy)
                mat->GetIonisation()->SetMeanExcitationEnergy(rec.MeanExEnergy * eV); // eV in the run time settings
            break;

        case AMaterialTransferRecord::Nist :
          {
            G4Material * base = man->FindOrBuildMaterial(rec.NistName);
            if (!base) terminateSession("Material " + rec.NistName + " (requested for material " + rec.Name + ") is not listed in G4NistManager");
            mat = new G4Material(rec.NistName, base->GetDensity(), base, base->GetState(), base->GetTemperature(), base->GetPressure());
            std::cout << "Material " << rec.Name << " -> derived from NIST material " << rec.NistName << '\n';
            break;
          }

        case AMaterialTransferRecord::NCrystal :
#ifdef ANTS3_NCRYSTAL
          {
            mat = G4NCrystal::createMaterial(rec.NCrystalCfg);
            if (!mat) terminateSession("Failed to build NCrystal material using string " + rec.NCrystalCfg + " for material " + rec.Name);

            // G4NCrystal caches its materials by the NCrystal Info/Scatter objects, so equivalent configuration strings
            // (also of different ants3 materials) return the same G4Material. Every ants3 material needs its own object:
            // in that case create a derived material with the same density/temperature and the same NCrystal scatter physics
            bool bAlreadyUsed = false;
            for (size_t i = 0; i < iMat; i++)
                if (MaterialsByIndex[i] == mat) bAlreadyUsed = true;
            if (bAlreadyUsed)
            {
                G4Material * ncMat = mat;
                mat = new G4Material(rec.Name, ncMat->GetDensity(), ncMat, ncMat->GetState(), ncMat->GetTemperature(), ncMat->GetPressure());
                G4NCrystal::addNCrystalScatterProperty(mat, rec.NCrystalCfg);
                std::cout << "Material " << rec.Name << " -> separate copy of the NCrystal material for string " << rec.NCrystalCfg << '\n';
            }
            else
            {
                mat->SetName(rec.Name);
                std::cout << "Material " << rec.Name << " -> NCrystal material created using string " << rec.NCrystalCfg << '\n';
            }
          }
#else
            terminateSession("NCrystal materials are not empty, but g4ants3 was compiled without NCrystal library! (material " + rec.Name + ")");
#endif
            break;
        }
        if (!mat) terminateSession("Failed to configure Geant4 material for material " + rec.Name);

        for (size_t i = 0; i < iMat; i++)
            if (MaterialsByIndex[i] == mat)
                terminateSession("Materials " + Records[i].Name + " and " + rec.Name + " resolve to the same Geant4 material object");

        MaterialsByIndex[iMat] = mat;
        if (mat != gdmlMat) Replacements[gdmlMat] = mat;
    }

    if (!Replacements.empty())
        for (G4LogicalVolume * lv : *G4LogicalVolumeStore::GetInstance())
        {
            auto it = Replacements.find(lv->GetMaterial());
            if (it != Replacements.end()) lv->SetMaterial(it->second);
        }

    G4IndexToAntsIndex.assign(G4Material::GetNumberOfMaterials(), -1);
    for (size_t iMat = 0; iMat < MaterialsByIndex.size(); iMat++)
        G4IndexToAntsIndex[MaterialsByIndex[iMat]->GetIndex()] = (int)iMat;
}

#include <unordered_map>
void SessionManager::restoreVolumeNames()
{
    // ants3 gives a GDML-safe name to every volume whose name would be changed by ROOT's GDML writer or the Geant4 GDML reader.
    // Restoring the original names here lets all the name-based configuration (sensitive detectors of monitors, calorimeters and analyzers,
    // deposition volumes, step limits, acolinearity volumes, exit volume) and the tracking history output use the ants3 names.
    if (Settings.RunSet.RenamedVolumes.empty()) return;

    std::unordered_map<std::string, std::string> OriginalNames; // GDML name -> original name
    for (const auto & pair : Settings.RunSet.RenamedVolumes) OriginalNames[pair.first] = pair.second;

    std::unordered_map<std::string, int> NumRestored;
    for (G4LogicalVolume * lv : *G4LogicalVolumeStore::GetInstance())
    {
        const auto it = OriginalNames.find(lv->GetName());
        if (it == OriginalNames.end()) continue;
        NumRestored[it->first]++;
        lv->SetName(it->second);
    }
    // SetName() only invalidates the name map of the store; G4VUserDetectorConstruction::SetSensitiveDetector(name, ...)
    // reads the map via GetMap() without updating it, so the map has to be rebuilt here
    G4LogicalVolumeStore::GetInstance()->UpdateMap();

    for (const auto & pair : Settings.RunSet.RenamedVolumes)
        if (NumRestored[pair.first] == 0)
            terminateSession("Volume " + pair.second + " is not found in the GDML file under the name " + pair.first);

    std::cout << "Restored original names of " << Settings.RunSet.RenamedVolumes.size() << " volume name(s)\n";
}

G4Material * SessionManager::findMaterialByAntsName(const std::string & antsName) const
{
    const std::vector<AMaterialTransferRecord> & Records = Settings.RunSet.Materials;
    for (size_t iMat = 0; iMat < Records.size() && iMat < MaterialsByIndex.size(); iMat++)
        if (Records[iMat].Name == antsName) return MaterialsByIndex[iMat];
    return nullptr;
}

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "afastsimphyshandler.h"
#include "AcollinearGammaModel.hh"
#include "G4AutoDelete.hh"
void SessionManager::configureAcolinearitySimulation(G4VModularPhysicsList * physicsList)
{
    AFastSimPhysHandler::createFastSimulationPhysics(physicsList);

    G4Region * reg = new G4Region("__AcolRegion");
    for (std::string volName : Settings.G4Set.AcolinearityVolumes)
    {
        bool bWildcard = (volName[volName.size()-1] == '*');
        if (bWildcard) volName.resize(volName.size() - 1);

        G4LogicalVolumeStore * lvStore = G4LogicalVolumeStore::GetInstance();
        for (auto lv : *lvStore)
        {
            const std::string thisVolName = lv->GetName();
            if (bWildcard)
            {
                if (thisVolName.size() >= volName.size() && thisVolName.compare(0, volName.size(), volName) == 0) // starts_with before c++20
                    reg->AddRootLogicalVolume(lv);
            }
            else
            {
                if (thisVolName == volName) reg->AddRootLogicalVolume(lv);
            }
        }
    }

    AcollinearGammaModel * mod;
    if (Settings.G4Set.AcolinearityModel == 0)
        mod = new AcollinearGammaModel("AcollinearGammas", reg, Settings.G4Set.AcolinearityFWHM);
    else
        mod = new AcollinearGammaModel2D("AcollinearGammas", reg, Settings.G4Set.AcolinearityFWHM);
    G4AutoDelete::Register(mod);
}

void SessionManager::writeNewEventMarker()
{
    std::string EventId = "#" + std::to_string(CurrentEvent);

    if (outStreamDeposition)
    {
        if (bBinaryOutput)
        {
            *outStreamDeposition << char(0xEE);
             outStreamDeposition->write((char*)&CurrentEvent, sizeof(int));
        }
        else
            *outStreamDeposition << EventId.data() << '\n';
    }

    if (Settings.RunSet.SaveTrackingHistory)
        if (outStreamHistory)
        {
            if (bBinaryOutput)
            {
                *outStreamHistory << char(0xEE);
                 outStreamHistory->write((char*)&CurrentEvent, sizeof(int));
            }
            else
                *outStreamHistory << EventId.data() << '\n';
        }

    if (outStreamExit)
    {
        if (bExitBinary)
        {
            *outStreamExit << char(0xEE);
             outStreamExit->write((char*)&CurrentEvent, sizeof(int));
        }
        else
            *outStreamExit << EventId.data() << '\n';
    }
}

void SessionManager::saveDepoRecord(const std::string & pName, int iMat, double edep, double *pos, double time, int copyNumber)
{
    if (!outStreamDeposition) return;

    // format:
    // partId matId DepoE X Y Z Time

    if (bBinaryOutput)
    {
        *outStreamDeposition << char(0xFF);

        *outStreamDeposition << pName << char(0x00);

        outStreamDeposition->write((char*)&iMat,        sizeof(int));
        outStreamDeposition->write((char*)&edep,        sizeof(double));
        outStreamDeposition->write((char*)pos,        3*sizeof(double));
        outStreamDeposition->write((char*)&time,        sizeof(double));
        outStreamDeposition->write((char*)&copyNumber,  sizeof(int)); // !!!***
    }
    else
    {
        std::stringstream ss;
        ss.precision(Settings.RunSet.AsciiPrecision);

        ss << pName << ' ';
        ss << iMat << ' ';
        ss << edep << ' ';
        ss << pos[0] << ' ' << pos[1] << ' ' << pos[2] << ' ';
        ss << time << ' ';
        ss << copyNumber;  // !!!***

        *outStreamDeposition << ss.rdbuf() << '\n';
    }
}

#include "SensitiveDetector.hh"
void SessionManager::saveCalorimeterLogEntry()
{
    if (!outStreamCalorimeterLog) return;

    if (bBinaryOutput)
    {
        for (CalorimeterSensitiveDetectorWrapper * sd : Calorimeters)
            outStreamCalorimeterLog->write((char*)&sd->SumDepoOverEvent, sizeof(double));
    }
    else
    {
        std::stringstream ss;
        for (CalorimeterSensitiveDetectorWrapper * sd : Calorimeters)
        {
            ss << sd->SumDepoOverEvent << ' ';
        }
        *outStreamCalorimeterLog << ss.rdbuf() << '\n';
    }
}

void SessionManager::saveTrackStart(int trackID, int parentTrackID,
                                    const G4String & particleName,
                                    const G4ThreeVector & pos, double time, double kinE,
                                    int iMat, const std::string &volName, int volIndex)
{
    if (!outStreamHistory) return;

    if (bBinaryOutput)
    {
        //format:
        //F0 trackId(int) parentTrackId(int) PartName(string) 0 X(double) Y(double) Z(double) time(double) kinEnergy(double) NextMat(int) NextVolNmae(string) 0 NextVolIndex(int)
        *outStreamHistory << char(0xF0);

        outStreamHistory->write((char*)&trackID,       sizeof(int));
        outStreamHistory->write((char*)&parentTrackID, sizeof(int));

        *outStreamHistory << particleName << char(0x00);

        double posArr[3];
        posArr[0] = pos.x();
        posArr[1] = pos.y();
        posArr[2] = pos.z();
        outStreamHistory->write((char*)posArr,  3*sizeof(double));
        outStreamHistory->write((char*)&time,     sizeof(double));
        outStreamHistory->write((char*)&kinE,     sizeof(double));

        outStreamHistory->write((char*)&iMat,     sizeof(int));
        *outStreamHistory << volName << char(0x00);
        outStreamHistory->write((char*)&volIndex, sizeof(int));
    }
    else
    {
        // format:
        // > TrackID ParentTrackID Particle X Y Z Time E iMat VolName VolIndex

        std::stringstream ss;
        ss.precision(Settings.RunSet.AsciiPrecision);

        ss << '>';
        ss << trackID << ' ';
        ss << parentTrackID << ' ';
        ss << particleName << ' ';
        ss << pos[0] << ' ' << pos[1] << ' ' << pos[2] << ' ';
        ss << time << ' ';
        ss << kinE << ' ';
        ss << iMat << ' ';
        ss << volName << ' ';
        ss << volIndex;

        *outStreamHistory << ss.rdbuf() << '\n';
    }

}

void SessionManager::saveTrackRecord(const std::string & procName,
                                     const G4ThreeVector & pos, double time,
                                     double kinE, double depoE,
                                     const std::vector<int> * secondaries,
                                     int iMatTo, const std::string & volNameTo, int volIndexTo)
{
    if (!outStreamHistory) return;

    // format for "T" processes:
    // ascii: ProcName  X Y Z Time KinE DirectDepoE iMatTo VolNameTo  VolIndexTo [secondaries] \n
    // bin:   [FF or F8] ProcName0 X Y Z Time KinE DirectDepoE iMatTo VolNameTo0 VolIndexTo numSec [secondaries]
    // for non-"T" process, iMatTo VolNameTo  VolIndexTo are absent
    // not that if energy depo is present on T step, it is in the previous volume!
    if (bBinaryOutput)
    {
        *outStreamHistory << char( iMatTo == -1 ? 0xFF    // not a transportation step
                                                : 0xF8 ); // transportation step, next volume/material is saved too

        *outStreamHistory << procName << char(0x00);

        double posArr[3];
        posArr[0] = pos.x();
        posArr[1] = pos.y();
        posArr[2] = pos.z();
        outStreamHistory->write((char*)posArr,  3*sizeof(double));
        outStreamHistory->write((char*)&time,     sizeof(double));

        outStreamHistory->write((char*)&kinE,     sizeof(double));
        outStreamHistory->write((char*)&depoE,    sizeof(double));

        if (iMatTo != -1)
        {
            outStreamHistory->write((char*)&iMatTo,     sizeof(int));
            *outStreamHistory << volNameTo << char(0x00);
            outStreamHistory->write((char*)&volIndexTo, sizeof(int));
        }

        int numSec = (secondaries ? secondaries->size() : 0);
        outStreamHistory->write((char*)&numSec, sizeof(int));
        if (secondaries)
        {
            for (const int & iSec : *secondaries)
                outStreamHistory->write((char*)&iSec, sizeof(int));
        }
    }
    else
    {
        std::stringstream ss;
        ss.precision(Settings.RunSet.AsciiPrecision);

        ss << procName << ' ';

        ss << pos[0] << ' ' << pos[1] << ' ' << pos[2] << ' ';
        ss << time << ' ';

        ss << kinE << ' ';
        ss << depoE;

        if (iMatTo != -1)
        {
            ss << ' ';
            ss << iMatTo << ' ';
            ss << volNameTo << ' ';
            ss << volIndexTo;
        }

        if (secondaries)
        {
            for (const int & isec : *secondaries)
                ss << ' ' << isec;
        }

        *outStreamHistory << ss.rdbuf() << '\n';
    }
}

#include "PrimaryGeneratorAction.hh"
void SessionManager::resetPredictedTrackID()
{
    PrimGenAction->resetPredictedTrackID();
}
void SessionManager::incrementPredictedTrackID()
{
    PrimGenAction->incrementPredictedTrackID();
}
int SessionManager::getPredictedTrackID() const
{
    return PrimGenAction->getPredictedTrackID();
}

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
void SessionManager::findExitVolume()
{
    G4LogicalVolumeStore * lvs = G4LogicalVolumeStore::GetInstance();

    std::vector<G4LogicalVolume*>::const_iterator lvciter;
    for (lvciter = lvs->begin(); lvciter != lvs->end(); ++lvciter)
    {
        std::string name = (std::string)(*lvciter)->GetName();
        if (name == Settings.RunSet.SaveSettings.VolumeName)
        {
            ExitVolume = *lvciter;
            std::cout << "Found exit volume " << ExitVolume << " --> " << ExitVolume->GetName().data() << '\n';
            return;
        }
    }

    //not found
    Settings.RunSet.SaveSettings.Enabled = false;
}

void SessionManager::saveParticle(const G4String &particle, double energy, double time, double *PosDir)
{
    if (bExitBinary)
    {
        *outStreamExit << char(0xFF);
        *outStreamExit << particle << char(0x00);
        outStreamExit->write((char*)&energy,  sizeof(double));
        outStreamExit->write((char*)PosDir, 6*sizeof(double));
        outStreamExit->write((char*)&time,    sizeof(double));
    }
    else
    {
        std::stringstream ss;
        ss.precision(Settings.RunSet.AsciiPrecision);

        ss << particle << ' ';
        ss << energy << ' ';
        ss << PosDir[0] << ' ' << PosDir[1] << ' ' << PosDir[2] << ' ';     //position
        ss << PosDir[3] << ' ' << PosDir[4] << ' ' << PosDir[5] << ' ';     //direction
        ss << time;

        *outStreamExit << ss.rdbuf() << '\n';
    }
}

#include "G4VSensitiveDetector.hh"
#include "G4MultiSensitiveDetector.hh"
bool SessionManager::isEnergyDepoLogger(G4LogicalVolume * vol)
{
    G4VSensitiveDetector * sd = vol->GetSensitiveDetector();
    G4MultiSensitiveDetector * multi = dynamic_cast<G4MultiSensitiveDetector*>(sd);
    if (!multi) return (sd && sd->GetName() == DepoLoggerSDName);

    int numElements = multi->GetSize();
    for (int i = 0; i < numElements; i++)
        if (multi->GetSD(i)->GetName() == DepoLoggerSDName) return true;

    return false;
}

#include "SensitiveDetector.hh"
void SessionManager::prepareMonitors()
{
    for (MonitorSensitiveDetectorWrapper * m : Monitors)
    {
        if (!m->ParticleName.empty())
            m->pParticleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(m->ParticleName);
    }
}

#include "amonitorsettings.h"
void SessionManager::readConfig(const std::string & workingDir, const std::string & ConfigFileName, int ID)
{
    WorkingDir = workingDir;

    std::ifstream in(WorkingDir + "/" + ConfigFileName);
    std::stringstream sstr;
    sstr << in.rdbuf();
    std::string s = sstr.str();
    std::cout << "Config file:\n" << s << '\n';

    json11::Json jo = json11::Json::parse(s, ErrorMessage);
    if (!ErrorMessage.empty()) terminateSession(ErrorMessage);

    json11::Json::object json = jo.object_items();

    AErrorHub::clear();
    Settings.readFromJson(json);
    if (AErrorHub::isError()) terminateSession(AErrorHub::getError());

    if (Settings.RunSet.Receipt.empty())
        terminateSession("File name for receipt was not provided");
    if (Settings.RunSet.GDML.empty())
        terminateSession("GDML file name is not provided");
    if (Settings.G4Set.PhysicsList.empty())
        terminateSession("Reference physics list is not provided");
    if (Settings.RunSet.SaveDeposition && Settings.RunSet.FileNameDeposition.empty())
        terminateSession("File name for deposition output not provided");
    if (Settings.RunSet.SaveTrackingHistory && Settings.RunSet.FileNameTrackingHistory.empty())
        terminateSession("File name for tracking history output not provided");

    std::cout << "GUI mode? " << Settings.RunSet.GuiMode << '\n';
    std::cout << "Random generator seed: " << Settings.RunSet.Seed << '\n';

    std::cout << "Config lists the following materials:" << '\n';
    for (size_t i = 0; i < Settings.RunSet.Materials.size(); i++)
    {
        const AMaterialTransferRecord & rec = Settings.RunSet.Materials[i];
        std::cout << i << " -> " << rec.Name << "  (GDML name: " << rec.GdmlName << ")";
        if      (rec.Type == AMaterialTransferRecord::Nist)     std::cout << "  to be constructed using G4NistManager: " << rec.NistName;
        else if (rec.Type == AMaterialTransferRecord::NCrystal) std::cout << "  to be constructed using NCrystal: " << rec.NCrystalCfg;
        std::cout << '\n';
    }

    bBinaryOutput = !Settings.RunSet.AsciiOutput;
    bExitBinary   = bBinaryOutput;
    std::cout << "Binary output? " << bBinaryOutput << '\n';

    if (Settings.RunSet.SaveSettings.Enabled)
        std::cout << "Save exit particles enabled for volume: " << Settings.RunSet.SaveSettings.VolumeName << "  Kill on exit? " << Settings.RunSet.SaveSettings.StopTrack << '\n';

    if (Settings.RunSet.MonitorSettings.Enabled)
    {
        for (const AMonSetRecord & r : Settings.RunSet.MonitorSettings.Monitors)
        {
            MonitorSensitiveDetectorWrapper * mobj = new MonitorSensitiveDetectorWrapper(r.Name, r.Particle, r.Index);
            bool ok = mobj->readFromJson(r.ConfigJson);
            if (!ok) terminateSession("Failed to read monitor config, might be units");
            Monitors.push_back(mobj);
            if (!mobj->bAcceptDirect || !mobj->bAcceptIndirect) bMonitorsRequireSteppingAction = true;
        }
        std::cout << "Monitors require stepping action: " << bMonitorsRequireSteppingAction << '\n';
    }

    if (Settings.RunSet.CalorimeterSettings.Enabled)
    {
        for (ACalSetRecord & r : Settings.RunSet.CalorimeterSettings.Calorimeters)
        {
            CalorimeterSensitiveDetectorWrapper * sd = new CalorimeterSensitiveDetectorWrapper(r.Name, r.Properties, r.Index);
            Calorimeters.push_back(sd);
        }
    }

    std::cout << "Config read completed" << std::endl;
}

void SessionManager::prepareAnalyzers()
{
    if (Settings.RunSet.AnalyzerSettings.Enabled)
    {
        const size_t num = Settings.RunSet.AnalyzerSettings.NumberOfUniqueAnalyzers;
        Analyzers.reserve(num);

        for (size_t iUnique = 0; iUnique < num; iUnique++)
        {
            const AParticleAnalyzerSettings & settings = Settings.RunSet.AnalyzerSettings;
            size_t iType = settings.UniqueToTypeLUT[iUnique]; // !!!*** error handling!

            const AParticleAnalyzerRecord & properties = settings.AnalyzerTypes[iType];

            int seenCopies = 0;
            int seenGlobal = -1;
            for (size_t iGlob = 0; iGlob < settings.GlobalToUniqueLUT.size(); iGlob++)
            {
                size_t iUn = settings.GlobalToUniqueLUT[iGlob];
                if (iUn == iUnique)
                {
                    seenCopies++;
                    if (seenCopies > 1) break;
                    seenGlobal = iGlob;
                }
            }
            if (seenCopies > 1) seenGlobal = -1;

            Analyzers.push_back( AAnalyzerUniqueInstance(properties, seenGlobal) );
        }
    }
}

void SessionManager::prepareOutputDepoStream()
{
    outStreamDeposition = new std::ofstream();

    if (bBinaryOutput)
        outStreamDeposition->open(WorkingDir + "/" + Settings.RunSet.FileNameDeposition, std::ios::out | std::ios::binary);
    else
        outStreamDeposition->open(WorkingDir + "/" + Settings.RunSet.FileNameDeposition);

    if (!outStreamDeposition->is_open())
        terminateSession("Cannot open file to store deposition data");
}

void SessionManager::prepareOutputCalorimeterLogStream()
{
    outStreamCalorimeterLog = new std::ofstream();

    if (bBinaryOutput)
        outStreamCalorimeterLog->open(WorkingDir + "/" + Settings.RunSet.CalorimeterSettings.LogFileName, std::ios::out | std::ios::binary);
    else
        outStreamCalorimeterLog->open(WorkingDir + "/" + Settings.RunSet.CalorimeterSettings.LogFileName);

    if (!outStreamCalorimeterLog->is_open())
        terminateSession("Cannot open file to store calorimeter log");
}

void SessionManager::prepareOutputHistoryStream()
{
    outStreamHistory = new std::ofstream();

    if (bBinaryOutput)
        outStreamHistory->open(WorkingDir + "/" + Settings.RunSet.FileNameTrackingHistory, std::ios::out | std::ios::binary);
    else
        outStreamHistory->open(WorkingDir + "/" + Settings.RunSet.FileNameTrackingHistory);

    if (!outStreamHistory->is_open())
        terminateSession("Cannot open file to export history/tracks data");
}

void SessionManager::prepareOutputExitStream()
{
    outStreamExit = new std::ofstream();

    if (bExitBinary)
        outStreamExit->open(WorkingDir + "/" + Settings.RunSet.SaveSettings.FileName, std::ios::out | std::ios::binary);
    else
        outStreamExit->open(WorkingDir + "/" + Settings.RunSet.SaveSettings.FileName);

    if (!outStreamExit->is_open())
        terminateSession("Cannot open file to export exiting particle data");
}

void SessionManager::executeAdditionalCommands()
{
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    for (const auto & cmd : Settings.G4Set.Commands)
        UImanager->ApplyCommand(cmd);

    UImanager->ApplyCommand("/run/initialize");
}

void SessionManager::generateReceipt()
{
    json11::Json::object receipt;

    receipt["Success"] = !bError;
    if (bError) receipt["Error"] = ErrorMessage;

    std::string json_str = json11::Json(receipt).dump();

    std::ofstream outStream;
    outStream.open(WorkingDir + "/" + Settings.RunSet.Receipt);
    if (outStream.is_open())
        outStream << json_str << '\n';
    outStream.close();
}

void SessionManager::storeMonitorsData()
{
    json11::Json::array Arr;

    for (MonitorSensitiveDetectorWrapper * mon : Monitors)
    {
        json11::Json::object json;
        mon->writeToJson(json);
        Arr.push_back(json);
    }

    std::ofstream outStream;
    outStream.open(WorkingDir + "/" + Settings.RunSet.MonitorSettings.FileName);
    if (outStream.is_open())
    {
        std::string json_str = json11::Json(Arr).dump();
        outStream << json_str << '\n';
    }
    outStream.close();
}

void SessionManager::storeCalorimeterData()
{
    json11::Json::array Arr;

    for (CalorimeterSensitiveDetectorWrapper * cal : Calorimeters)
    {
        json11::Json::object json;
        cal->writeToJson(json);
        Arr.push_back(json);
    }

    std::ofstream outStream;
    outStream.open(WorkingDir + "/" + Settings.RunSet.CalorimeterSettings.FileName);
    if (outStream.is_open())
    {
        std::string json_str = json11::Json(Arr).dump();
        outStream << json_str << '\n';
    }
    outStream.close();
}

void SessionManager::storeAnalyzerData()
{
    json11::Json::array Arr;

    for (const AAnalyzerUniqueInstance & an : Analyzers)
    {
        json11::Json::object json;
        an.writeToJson(json);
        Arr.push_back(json);
    }

    std::ofstream outStream;
    outStream.open(WorkingDir + "/" + Settings.RunSet.AnalyzerSettings.FileName);
    if (outStream.is_open())
    {
        std::string json_str = json11::Json(Arr).dump();
        outStream << json_str << '\n';
    }
    outStream.close();
}

G4ParticleDefinition * SessionManager::findGeant4Particle(const std::string & particleName)
{
    if (particleName == "-") return nullptr;
    if (!particleName.empty() && particleName[0] == '_') return nullptr;

    G4ParticleDefinition * Particle = G4ParticleTable::GetParticleTable()->FindParticle(particleName);

    if (!Particle)
    {
        // is it an ion?
        int Z, A;
        double E;
        bool ok = extractIonInfo(particleName, Z, A, E);
        if (!ok)
            terminateSession("Found an unknown particle: " + particleName);

        Particle = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(Z, A, E*keV);

        if (!Particle)
            terminateSession("Failed to generate ion: " + particleName);

        //std::cout << particleName << "   ->   " << Particle->GetParticleName() << '\n';
    }

    return Particle;
}

bool SessionManager::extractIonInfo(const std::string & text, int & Z, int & A, double & E)
{
    size_t size = text.length();
    if (size < 2) return false;

    // -- extracting Z --
    const char & c0 = text[0];
    if (c0 < 'A' || c0 > 'Z') return false;
    std::string symbol;
    symbol += c0;

    size_t index = 1;
    const char & c1 = text[1];
    if (c1 >= 'a' && c1 <= 'z')
    {
        symbol += c1;
        index++;
    }
    try
    {
        Z = ElementToZ.at(symbol);
    }
    catch (...)
    {
        return false;
    }

    // -- extracting A --
    A = 0; E = 0;
    char ci;
    while (index < size)
    {
        ci = text[index];
        if (ci < '0' || ci > '9') break;
        A = A*10 + (int)ci - (int)'0';
        index++;
    }
    if (A == 0) return false;

    if (index == size) return true;

    // -- extracting excitation energy --
    if (ci != '[') return false;
    index++;
    std::stringstream energy;
    while (index < size)
    {
        ci = text[index];
        if (ci == ']')
        {
            energy >> E;
            return !energy.fail();
        }
        energy << ci;
        index++;
    }
    return false;
}
