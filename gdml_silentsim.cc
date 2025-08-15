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
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
class G01PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    G01PrimaryGeneratorAction(): G4VUserPrimaryGeneratorAction() { fParticleGun = new G4ParticleGun(1); }
    ~G01PrimaryGeneratorAction(){delete fParticleGun;}
    virtual void GeneratePrimaries(G4Event* anEvent){std::cout << __PRETTY_FUNCTION__ << std::endl; fParticleGun->GeneratePrimaryVertex(anEvent);}
private:
    G4ParticleGun* fParticleGun;
};
//________________________________________________________________________________
#include "G4VUserActionInitialization.hh"
#include "SecondaryCounterActions.hh"
class YourActionInitialization : public G4VUserActionInitialization {
public:
    YourActionInitialization(std::string ofilename): G4VUserActionInitialization(), fOfilename(ofilename) { }
    ~YourActionInitialization() override {}
    void Build() const override {
        SetUserAction(new G01PrimaryGeneratorAction());
        RunActionForSecondaries * run =  new RunActionForSecondaries(fOfilename);
        // SecondaryCounterTrackingAction * trk = new SecondaryCounterTrackingAction();
        TrkActionForSecondaries * trk = new TrkActionForSecondaries();
        EvtActionForSecondaries * evt = new EvtActionForSecondaries(trk,run);
        SetUserAction(run);
        SetUserAction(evt);
        SetUserAction(trk);
    }
    std::string fOfilename;

};
//________________________________________________________________________________

int main(int argc, char** argv)
{
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

    std::string ofilename_with_secondary_stats = std::string(argv[1])
                                                 +"_"
                                                 +std::string(argv[2])
                                                 +".root";

    YourDetectorConstructor * user_detector_constructor = new YourDetectorConstructor();
    user_detector_constructor->LoadGDML(argv[1]);
    runManager->SetUserInitialization(user_detector_constructor);
    runManager->SetUserInitialization(new FTFP_BERT);
    runManager->SetUserInitialization(new YourActionInitialization(ofilename_with_secondary_stats));
    runManager->Initialize();

    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // if BeamOn(0) is not there, it crashes...
    runManager->BeamOn(0);



    // option for regions
    if( std::string( argv[2] ) == "original_cuts")
        define_original_hgcal_region();
    else if( std::string( argv[2] ) == "new_cuts")
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



    // if macrofile is provided, use it, otherwise open visualization
    if (argc == 4)  // batch mode
    {
        G4String command = "/control/execute ";
        G4String fileName = argv[3];
        UImanager->ApplyCommand(command + fileName);
    }
    else  // interactive mode
    {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
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

