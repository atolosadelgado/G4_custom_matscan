#include "FTFP_BERT.hh"
#include "G01DetectorConstruction.hh"
#include "G01PrimaryGeneratorAction.hh"
#include "YourActionInitialization.hh"

#include "G4GDMLParser.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RunManagerFactory.hh"
#include "G4TransportationManager.hh"
#include "G4Types.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include <vector>


void define_subregions();


int main(int argc, char** argv)
{
  G4cout << G4endl;
  G4cout << "Usage: load_gdml <intput_gdml_file:mandatory>"
         << " <output_gdml_file:optional>" << G4endl;
  G4cout << G4endl;

  if (argc < 2) {
    G4cout << "Error! Mandatory input file is not specified!" << G4endl;
    G4cout << G4endl;
    return -1;
  }

  G4GDMLParser parser;

  // Uncomment the following if wish to avoid names stripping
  // parser.SetStripFlag(false);

  // Uncomment the following and set a string with proper absolute path and
  // schema filename if wishing to use alternative schema for parsing validation
  // parser.SetImportSchema("");

  parser.SetOverlapCheck(false);
  parser.Read(argv[1]);



  auto* runManager = G4RunManagerFactory::CreateRunManager();


  /// Interact with CMS geometry...
  G01DetectorConstruction * user_detector_constructor = new G01DetectorConstruction();
  G4VPhysicalVolume * world_pv = parser.GetWorldVolume();
  // load the whole GDML or just a subdetector
  G4String detector_pv_name = "HGCal";
  bool set_world = (0==detector_pv_name.size());
  if( set_world ) {
    std::cout << "placing the world..." << std::endl;
    user_detector_constructor->SetWorldPV(world_pv);
  }
  else{
    std::cout << "placing subdetectors  ..." << std::endl;
    G4VPhysicalVolume * detector_pv = parser.GetPhysVolume(detector_pv_name);
    user_detector_constructor->SetDetectorPV(detector_pv);
  }


  runManager->SetUserInitialization(user_detector_constructor);
  runManager->SetUserInitialization(new FTFP_BERT);
  runManager->SetUserInitialization(new YourActionInitialization());
  runManager->Initialize();

  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();



  define_subregions();


  runManager->BeamOn(0);


  if (argc == 3)  // batch mode
  {
    G4String command = "/control/execute ";
    G4String fileName = argv[2];
    UImanager->ApplyCommand(command + fileName);
  }
  else  // interactive mode
  {
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;

  return 0;
}


#include <G4LogicalVolumeStore.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include "G4RegionStore.hh"

void define_subregions()
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
}
