// for building the detector
#include "G4GDMLParser.hh"
// default physics
#include "FTFP_BERT.hh"
// run manager
#include "G4RunManagerFactory.hh"
// UI + vis
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

void define_hgcal_subregions();
void define_original_hgcal_region();


//________________________________________________________________________________
#include "G4VUserDetectorConstruction.hh"
#include "G4GDMLParser.hh"
class YourDetectorConstructor : public G4VUserDetectorConstruction
{
public:
    void LoadGDML(std::string gdml_filename){
        G4GDMLParser Parser;
        Parser.Read(gdml_filename, false);
        this->worldPV = Parser.GetWorldVolume();
    };
    virtual G4VPhysicalVolume* Construct(){return worldPV;}
    G4VPhysicalVolume * worldPV = {nullptr};
};
//________________________________________________________________________________

#include "SecondaryCounterActions.hh"
#include "YourActionInitialization.hh"

//________________________________________________________________________________

void help(int argc, char** argv){
    std::cout << "Usage:" << std::endl;
    std::cout << "\t" << argv[0] << "<geometry.gdml> <cut_type> <action_type> <physics option> <gun.mac>" << std::endl;
    std::cout << "\t  <cut_type>: extra regions defined inside the application. Options: original_cuts, new_cuts, no_cuts" << std::endl;
    std::cout << "\t  <action_type>: action for the Geant4 application. Options: secondaries (for just stats of secondaries), profile (for shower profile)" << std::endl;
    std::cout << "\t  <physics option>: name of physics list to be used (e.g., FTFP_BERT_EMZ)" << std::endl;
}

#include "G4PhysListFactory.hh"

int main(int argc, char** argv)
{
    help(argc, argv);
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

    auto geometry_filename = argv[1];
    auto action_type = std::string( argv[3] );
    auto productioncut_type = std::string( argv[2] );
    auto physics_list_name = std::string( argv[4] );
    bool vis_mode = false;

    G4String g4macro_filename;

    // if g4 macro file is provided
    if( argc == 6 )
        g4macro_filename =G4String( argv[5] );
    else
        vis_mode = true;


    YourDetectorConstructor * user_detector_constructor = new YourDetectorConstructor();
    user_detector_constructor->LoadGDML(geometry_filename);
    runManager->SetUserInitialization(user_detector_constructor);


    G4PhysListFactory pl_factory;
    auto physics_list = pl_factory.GetReferencePhysList( physics_list_name );
    if( ! physics_list ) throw std::runtime_error("No physics list named <"+ physics_list_name+"> found");
    runManager->SetUserInitialization(physics_list);
    if( action_type == "secondaries"){
        std::string ofilename_with_secondary_stats = std::string(argv[1])
                                                 +"_"
                                                 +std::string(argv[2])
                                                 +"_"
                                                 +"secondaryStats"
                                                 +".root";
        runManager->SetUserInitialization(
            new YourActionInitializationForSecondaries(ofilename_with_secondary_stats)
        );
    }
    else if( action_type == "profile"){
        runManager->SetUserInitialization(
            new YourActionInitialization()
        );
    }
    else{
        std::cerr << "No actions!" << std::endl;
    }
    runManager->Initialize();

    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // if BeamOn(0) is not there, it crashes...
    runManager->BeamOn(0);

    // option for regions
    if( productioncut_type == "original_cuts")
        define_original_hgcal_region();
    else if( productioncut_type == "new_cuts")
        define_hgcal_subregions();
    else
        std::cout << "Warning, no regions are being defined\n";

    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    G4ProductionCutsTable::GetProductionCutsTable()->UpdateCoupleTable(user_detector_constructor->worldPV);

    // this line makes program crash
    // runManager->ReinitializeGeometry(true);


    // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();



    if(vis_mode)
    {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    }
    else
    {
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + g4macro_filename);
    }


    //________________________________________________________________________________

    delete visManager;
    delete runManager;

    return 0;
}


#include <G4LogicalVolumeStore.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include "G4RegionStore.hh"

void define_hgcal_subregions()
{

    // 1. Define master region of HGCal
    {
        auto HGCalRegion = new G4Region("HGCalRegion");
        // assign cuts
        auto HGCalcuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalcuts->SetProductionCut(3 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalcuts->SetProductionCut(3 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalcuts->SetProductionCut(3 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalcuts->SetProductionCut(3 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalRegion->SetProductionCuts(HGCalcuts);
        // ----------------------------------------------------------
        // assign root volumes
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        auto HGCal_lv = lv_store->GetVolume("HGCal");
        HGCalRegion->AddRootLogicalVolume(HGCal_lv);
    }

    // 2. Define subregion of EE for sensitive parts made of silicon
    {
        auto HGCalEEsiliconRegion = new G4Region("HGCalEEsiliconRegion");
        // assign cuts
        auto HGCalEEsiliconCuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalEEsiliconCuts->SetProductionCut(0.03 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalEEsiliconCuts->SetProductionCut(0.03 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalEEsiliconCuts->SetProductionCut(0.03 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalEEsiliconCuts->SetProductionCut(0.03 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalEEsiliconRegion->SetProductionCuts(HGCalEEsiliconCuts);
        // ----------------------------------------------------------
        // assign root volumes
        const G4Material * si_material = G4Material::GetMaterial("Silicon");
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        for (const auto& lv : *lv_store)
        {
            if( lv->GetMaterial() == si_material )
                HGCalEEsiliconRegion->AddRootLogicalVolume(lv);
        }
    }


    // 3. Define subregion of EE for passive parts near silicon
    // -- TODO: this approach is conservative since even volumes further from the silicon
    // --       are included here. Volumes far away from the silicon may have higher threshold
    {
        auto HGCalEEkaptonCopperRegion = new G4Region("HGCalEEkaptonCopperRegion");
        // assign cuts
        auto HGCalEEkaptonCopperCuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalEEkaptonCopperCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalEEkaptonCopperCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalEEkaptonCopperCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalEEkaptonCopperCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalEEkaptonCopperRegion->SetProductionCuts(HGCalEEkaptonCopperCuts);
        // ----------------------------------------------------------
        // assign root volumes
        const G4Material * kapton_material = G4Material::GetMaterial("Kapton");
        const G4Material * copper_material = G4Material::GetMaterial("Copper");
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        for (const auto& lv : *lv_store)
        {
            if( lv->GetMaterial() == kapton_material || lv->GetMaterial() == copper_material )
                HGCalEEkaptonCopperRegion->AddRootLogicalVolume(lv);
        }
    }

    // 4. Define subregion of EE for absorber WCu
    {
        auto HGCalEEwcuRegion = new G4Region("HGCalEEwcuRegion");
        // assign cuts
        auto HGCalEEwcuCuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalEEwcuCuts->SetProductionCut(0.3 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalEEwcuCuts->SetProductionCut(0.3 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalEEwcuCuts->SetProductionCut(0.3 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalEEwcuCuts->SetProductionCut(0.3 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalEEwcuRegion->SetProductionCuts(HGCalEEwcuCuts);
        // ----------------------------------------------------------
        // assign root volumes
        const G4Material * si_material = G4Material::GetMaterial("WCu");
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        for (const auto& lv : *lv_store)
        {
            if( lv->GetMaterial() == si_material )
                HGCalEEwcuRegion->AddRootLogicalVolume(lv);
        }
    }
    // 5. Define subregion of EE for absorber Pb
    {
        auto HGCalEEwcuRegion = new G4Region("HGCalEEpbRegion");
        // assign cuts
        auto HGCalEEwcuCuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalEEwcuCuts->SetProductionCut(5.0 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalEEwcuCuts->SetProductionCut(2.0 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalEEwcuCuts->SetProductionCut(2.0 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalEEwcuCuts->SetProductionCut(5.0 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalEEwcuRegion->SetProductionCuts(HGCalEEwcuCuts);
        // ----------------------------------------------------------
        // assign root volumes
        const G4Material * si_material = G4Material::GetMaterial("Lead");
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        for (const auto& lv : *lv_store)
        {
            if( lv->GetMaterial() == si_material )
                HGCalEEwcuRegion->AddRootLogicalVolume(lv);
        }
    }
    // 6. Define subregion of EE for absorber StainlessSteel
    {
        auto HGCalEEwcuRegion = new G4Region("HGCalEEstainlesstealRegion");
        // assign cuts
        auto HGCalEEwcuCuts = new G4ProductionCuts();
        // Set cut values (in mm)
        HGCalEEwcuCuts->SetProductionCut(0.5 * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
        HGCalEEwcuCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
        HGCalEEwcuCuts->SetProductionCut(0.1 * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
        HGCalEEwcuCuts->SetProductionCut(2 * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
        HGCalEEwcuRegion->SetProductionCuts(HGCalEEwcuCuts);
        // ----------------------------------------------------------
        // assign root volumes
        const G4Material * si_material = G4Material::GetMaterial("StainlessSteel");
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        for (const auto& lv : *lv_store)
        {
            if( lv->GetMaterial() == si_material )
                HGCalEEwcuRegion->AddRootLogicalVolume(lv);
        }
    }
}

void define_original_hgcal_region()
{

    double cut_in_mm = 0.03;
    auto HGCalRegion = new G4Region("HGCalRegion");
    // assign cuts
    auto HGCalcuts = new G4ProductionCuts();
    // Set cut values (in mm)
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
    HGCalRegion->SetProductionCuts(HGCalcuts);
    // ----------------------------------------------------------
    // assign root volumes
    G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
    auto HGCal_lv = lv_store->GetVolume("HGCal");
    HGCalRegion->AddRootLogicalVolume(HGCal_lv);

}

