//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file persistency/gdml/G01/load_gdml.cc
/// \brief Main program of the persistency/gdml/G01 example
//
//
//
//
// --------------------------------------------------------------
//      GEANT 4 - load_gdml
//
// --------------------------------------------------------------

#include "FTFP_BERT.hh"
#include "G01ActionInitialization.hh"
#include "G01DetectorConstruction.hh"
#include "G01PrimaryGeneratorAction.hh"

#include "G4GDMLParser.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RunManagerFactory.hh"
#include "G4TransportationManager.hh"
#include "G4Types.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include <vector>

void print_lv_and_material_info();

void define_subregions();

void print_aux(const G4GDMLAuxListType* auxInfoList, G4String prepend = "|")
{
  for (std::vector<G4GDMLAuxStructType>::const_iterator iaux = auxInfoList->begin();
       iaux != auxInfoList->end(); iaux++)
  {
    G4String str = iaux->type;
    G4String val = iaux->value;
    G4String unit = iaux->unit;

    G4cout << prepend << str << " : " << val << " " << unit << G4endl;

    if (iaux->auxList) print_aux(iaux->auxList, prepend + "|");
  }
  return;
}

// --------------------------------------------------------------

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

  // std::cout << "HGCal pv... " << parser.GetPhysVolume("HGCal");
  // std::cin.ignore();

  if (argc > 4) {
    G4cout << "Error! Too many arguments!" << G4endl;
    G4cout << G4endl;
    return -1;
  }

  auto* runManager = G4RunManagerFactory::CreateRunManager();


  /// Interact with CMS geometry...
  G01DetectorConstruction * user_detector_constructor = new G01DetectorConstruction();
  G4VPhysicalVolume * world_pv = parser.GetWorldVolume();
  // load the whole GDML or just a subdetector
  G4String detector_pv_name = "HGCal";
  G4int maxlevel = 4;
  bool set_world = (0==detector_pv_name.size());
  if( set_world ) {
    std::cout << "placing the world..." << std::endl;
    user_detector_constructor->SetWorldPV(world_pv);
  }
  else{

    std::cout << "placing subdetectors  ..." << std::endl;
    G4VPhysicalVolume * detector_pv = parser.GetPhysVolume(detector_pv_name);
    user_detector_constructor->SetDetectorPV(detector_pv);
    // user_detector_constructor->ExtractAndPlaceSubDet(world_pv, maxlevel, detector_pv_name);
  }


  runManager->SetUserInitialization(user_detector_constructor);
  runManager->SetUserInitialization(new FTFP_BERT);
  runManager->SetUserInitialization(new G01ActionInitialization());
  runManager->Initialize();

  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  ///////////////////////////////////////////////////////////////////////
  //
  // Example how to retrieve Auxiliary Information
  //

  G4cout << std::endl;

  const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
  std::vector<G4LogicalVolume*>::const_iterator lvciter;
  for (lvciter = lvs->begin(); lvciter != lvs->end(); lvciter++) {
    G4GDMLAuxListType auxInfo = parser.GetVolumeAuxiliaryInformation(*lvciter);

    if (auxInfo.size() > 0)
      G4cout << "Auxiliary Information is found for Logical Volume :  " << (*lvciter)->GetName()
             << G4endl;

    print_aux(&auxInfo);
  }

  // now the 'global' auxiliary info
  G4cout << std::endl;
  G4cout << "Global auxiliary info:" << std::endl;
  G4cout << std::endl;

  print_aux(parser.GetAuxList());

  G4cout << std::endl;

  //
  // End of Auxiliary Information block
  //
  ////////////////////////////////////////////////////////////////////////

  define_subregions();


  runManager->BeamOn(0);
  // print_lv_and_material_info();


  // example of writing out

  if (argc >= 3) {
    /*
         G4GDMLAuxStructType mysubaux = {"mysubtype", "mysubvalue", "mysubunit", 0};
         G4GDMLAuxListType* myauxlist = new G4GDMLAuxListType();
         myauxlist->push_back(mysubaux);

         G4GDMLAuxStructType myaux = {"mytype", "myvalue", "myunit", myauxlist};
         parser.AddAuxiliary(myaux);


         // example of setting auxiliary info for world volume
         // (can be set for any volume)

         G4GDMLAuxStructType mylocalaux = {"sometype", "somevalue", "someunit", 0};

         parser.AddVolumeAuxiliary(mylocalaux,
           G4TransportationManager::GetTransportationManager()
           ->GetNavigatorForTracking()->GetWorldVolume()->GetLogicalVolume());
    */

    parser.SetRegionExport(true);

    // parser.SetMaxExportLevel(3);
    parser.SetOutputFileOverwrite(true);
    //     parser.SetEnergyCutsExport(true);
    //     parser.SetOutputFileOverwrite(true);
    parser.Write(argv[2], G4TransportationManager::GetTransportationManager()
                            ->GetNavigatorForTracking()
                            ->GetWorldVolume()
                            ->GetLogicalVolume());
  }

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

  delete visManager;
  delete runManager;

  return 0;
}


#include <G4LogicalVolumeStore.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>

#include <map>
#include <vector>
#include <string>
#include <iostream>

void print_lv_and_material_info()
{
  G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();

  // map that links a material with a vector of lv
  std::map<std::string, std::vector<std::string>> material_to_lv_map;

  for (const auto& lv : *lv_store)
  {
    std::string lv_name = lv->GetName();
    G4Material* material = lv->GetMaterial();

    std::string material_name = material ? material->GetName() : "NoMaterial";

    // insert lv name according to its material name
    material_to_lv_map[material_name].push_back(lv_name);
  }

  // print map content
  std::cout << "\nLogical volumes per material:\n";
  for (const auto& pair : material_to_lv_map)
  {
    std::cout << "Material: " << pair.first << "\n";
    for (const auto& name : pair.second)
    {
      std::cout << "  - " << name << "\n";
    }
  }

  std::cout << "\nNumber of Logical Volumes per Material:\n";
  for (const auto& pair : material_to_lv_map)
  {
    std::cout << pair.first << "\t" << pair.second.size() << std::endl;
  }

  // // create an out file with the lv names for each material
  // for (const auto& pair : material_to_lv_map)
  // {
  //   std::string material_name = pair.first;
  //   std::vector<std::string> lv_names = pair.second;
  //
  //   // create safe name, removing spaces or slashes
  //   std::string filename = material_name;
  //   std::replace(filename.begin(), filename.end(), ' ', '_');      // remove spaces
  //   std::replace(filename.begin(), filename.end(), '/', '_');      // remove slashes
  //   filename = "hgcal_lv_madeof_" + filename + ".txt";
  //
  //   std::ofstream outfile(filename);
  //   if (!outfile)
  //   {
  //     std::cerr << "Error: cannot open " << filename << "\n";
  //     continue;
  //   }
  //
  //   outfile << "Logical Volumes with material: " << material_name << "\n\n";
  //   for (const auto& name : lv_names)
  //   {
  //     outfile << "- " << name << "\n";
  //   }
  //
  //   outfile.close();
  // }

}

#include "G4RegionStore.hh"

void define_subregions()
{
  // // Print regions
  // G4RegionStore* region_store = G4RegionStore::GetInstance();
  //
  // for (const auto& region : *region_store)
  // {
  //   std::cout << "Region: " << region->GetName() << std::endl;
  //
  //   int number_of_lv = region->GetNumberOfRootVolumes();
  //   auto lv_region_it = region->GetRootLogicalVolumeIterator();
  //
  //   for (int i = 0; i < number_of_lv; ++i)
  //   {
  //     G4LogicalVolume* lv = *(lv_region_it + i);
  //     std::cout << "  - " << lv->GetName() << std::endl;
  //   }
  // }

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
