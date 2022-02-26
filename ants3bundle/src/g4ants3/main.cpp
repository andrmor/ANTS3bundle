#include "SessionManager.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4GDMLParser.hh"
#include "G4PhysListFactory.hh"

#include "QGSP_BIC.hh"
#include "QGSP_BIC_HP.hh"
#include "QGSP_BIC_AllHP.hh"

#include "QGSP_BERT.hh"
#include "QGSP_BERT_HP.hh"
#include "QGSP_FTFP_BERT.hh"
#include "QGSP_INCLXX.hh"
#include "QGSP_INCLXX_HP.hh"

#include "FTFP_BERT.hh"
#include "FTFP_BERT_ATL.hh"
#include "FTFP_BERT_HP.hh"
#include "FTFP_BERT_TRV.hh"
#include "FTFP_INCLXX.hh"
#include "FTFP_INCLXX_HP.hh"

#include <QDebug>

int main(int argc, char** argv)
{
    SessionManager & SM = SessionManager::getInstance();

    if (argc < 4) SM.terminateSession("Need 3 arguments: WorkingDir, ConfigFileName(no dir, should be in WorkingDir), Id(int)");

    SM.ReadConfig(argv[1], argv[2], atoi(argv[3]));
    const bool & bGui = SM.Settings.RunSet.GuiMode;

    G4UIExecutive * ui = nullptr;
    if (bGui) ui = new G4UIExecutive(argc, argv);

    G4RunManager * runManager = new G4RunManager;
    G4GDMLParser parser;
    parser.Read(SM.WorkingDir + "/" + SM.Settings.RunSet.GDML, false); //false - no validation
    // need to implement own G4excpetion-based handler class  ->  SM.terminateSession("Error parsing GDML file");
    SM.WorldPV = parser.GetWorldVolume();
    runManager->SetUserInitialization(new DetectorConstruction(SM.WorldPV));
    SM.updateMaterials(); // if present in config, indicated materials will be replaced with those from nist database
    //runManager->PhysicsHasBeenModified();

    const std::string & pl = SM.Settings.G4Set.PhysicsList;
    G4VModularPhysicsList *          physicsList = nullptr;
    if      (pl == "QGSP_BIC")       physicsList = new QGSP_BIC();
    else if (pl == "QGSP_BIC_HP")    physicsList = new QGSP_BIC_HP();
    else if (pl == "QGSP_BIC_AllHP") physicsList = new QGSP_BIC_AllHP();
    else if (pl == "QGSP_BERT")      physicsList = new QGSP_BERT();
    else if (pl == "QGSP_BERT_HP")   physicsList = new QGSP_BERT_HP();
    else if (pl == "QGSP_FTFP_BERT") physicsList = new QGSP_FTFP_BERT();
    else if (pl == "QGSP_INCLXX")    physicsList = new QGSP_INCLXX();
    else if (pl == "QGSP_INCLXX_HP") physicsList = new QGSP_INCLXX_HP();
    else if (pl == "FTFP_BERT")      physicsList = new FTFP_BERT();
    else if (pl == "FTFP_BERT_ATL")  physicsList = new FTFP_BERT_ATL();
    else if (pl == "FTFP_BERT_HP")   physicsList = new FTFP_BERT_HP();
    else if (pl == "FTFP_BERT_TRV")  physicsList = new FTFP_BERT_TRV();
    else if (pl == "FTFP_INCLXX")    physicsList = new FTFP_INCLXX();
    else if (pl == "FTFP_INCLXX_HP") physicsList = new FTFP_INCLXX_HP();
    else
    {
        G4PhysListFactory factory;
        if (factory.IsReferencePhysList(pl))
            physicsList = factory.GetReferencePhysList(pl);
    }
    if (!physicsList)
        SM.terminateSession("Unknown reference physics list");
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    runManager->SetUserInitialization(physicsList);

    runManager->SetUserInitialization(new ActionInitialization());

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->ApplyCommand("/run/initialize");
    UImanager->ApplyCommand("/control/verbose 0");
    UImanager->ApplyCommand("/run/verbose 0");

    if (SM.activateNeutronThermalScatteringPhysics())
    {
        runManager->PhysicsHasBeenModified();
        UImanager->ApplyCommand("/run/initialize"); //needed?
    }

    if (bGui)
    {
        UImanager->ApplyCommand("/hits/verbose 2");
        UImanager->ApplyCommand("/tracking/verbose 2");
        UImanager->ApplyCommand("/control/saveHistory");
    }

    SM.startSession();

    G4VisManager* visManager = nullptr;

    if (!SM.Settings.RunSet.GuiMode)
        SM.runSimulation();
    else
    {
        visManager = new G4VisExecutive("Quiet");
        visManager->Initialize();
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
    }

    delete visManager;
    delete runManager;
    delete ui;

    SM.endSession();
}
