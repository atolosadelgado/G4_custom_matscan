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
/// \file persistency/gdml/G01/include/G01DetectorConstruction.hh
/// \brief Definition of the G01DetectorConstruction class
//
//
//
//

#ifndef _G01DETECTORCONSTRUCTION_H_
#define _G01DETECTORCONSTRUCTION_H_

#include "G4VUserDetectorConstruction.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"

/// Detector construction allowing to use the geometry read from the GDML file

class G01DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    G01DetectorConstruction(G4VPhysicalVolume* setWorld = 0) {
        fWorld_pv = setWorld; }
    void SetWorldPV(G4VPhysicalVolume* setWorld = 0) {
        fWorld_pv = setWorld; }
    void SetDetectorPV(G4VPhysicalVolume* setDetector = 0) ;
    void CreateWorld();
    /// if there is more than 1 placed volume corresponding to the subdetector, use this function
    /// to iterate in the placed volumes at a certain level and if match a name pattern, place them
    /// in our world
    void ExtractAndPlaceSubDet(G4VPhysicalVolume* rootPV, G4int maxlevel, G4String subdetname) { cms_subdet_name = subdetname; TraverseGeoTreeAndCopy(rootPV, 0, maxlevel); }
    void TraverseGeoTreeAndCopy(G4VPhysicalVolume * pv, G4int level, G4int maxlevel);


    virtual G4VPhysicalVolume* Construct() { return fWorld_pv; }

  private:
    G4VPhysicalVolume* fWorld_pv;
    G4LogicalVolume *  fWorld_lv;
    G4int cms_subdet_geolevel = {3};
    G4String cms_subdet_name = {"HGCal"};
};

void G01DetectorConstruction::CreateWorld(){
  if(fWorld_pv)
  {
    fWorld_lv = fWorld_pv->GetLogicalVolume();
    return;
  }
    // 1. Material for the world: low density hydrogen defined by "hand"
  G4double zet      = 1.0;
  G4double amass    = 1.01*CLHEP::g/CLHEP::mole;
  G4double density  = CLHEP::universe_mean_density;
  G4double pressure = 3.e-18*CLHEP::pascal;
  G4double tempture = 2.73*CLHEP::kelvin;
  G4Material* materialWorld  = new G4Material("Galactic", zet, amass, density,
                                              kStateGas, tempture, pressure);

  // 2. Create the world and the target (both will be box):
  // a. world
  G4double worldXSize = 25*CLHEP::m;
  G4double worldYZSize = worldXSize;
  G4Box*              worldSolid   = new G4Box("solid-World",  // name
                                            0.5*worldXSize,   // half x-size
                                            0.5*worldYZSize,  // half y-size
                                            0.5*worldYZSize); // half z-size
  G4LogicalVolume*    worldLogical = new G4LogicalVolume(worldSolid,     // solid
                                                          materialWorld,  // material
                                                          "logic-World"); // name
  G4VPhysicalVolume*  worldPhyscal = new G4PVPlacement(nullptr,                 // (no) rotation
                                                        G4ThreeVector(0.,0.,0.), // translation
                                                        worldLogical,            // its logical volume
                                                        "World",                 // its name
                                                        nullptr,                 // its mother volume
                                                        true,                   // not used
                                                        0);                      // cpy number
      fWorld_pv = worldPhyscal;
      fWorld_lv = worldLogical;
}

void G01DetectorConstruction::SetDetectorPV(G4VPhysicalVolume * input_detectorPV )
{

    if( ! fWorld_lv )
      CreateWorld();
    G4LogicalVolume * detector_lv = input_detectorPV->GetLogicalVolume();
    G4RotationMatrix * detector_rotation_matrix = input_detectorPV->GetObjectRotation();
    G4ThreeVector detector_translation_vector = input_detectorPV->GetObjectTranslation();
    G4String detector_name = input_detectorPV->GetName();
    /*G4VPhysicalVolume*  detectorPV = */new G4PVPlacement(detector_rotation_matrix,
                                                          detector_translation_vector,
                                                          detector_lv,             // its logical volume
                                                          detector_name,           // its name
                                                          fWorld_lv,            // its mother volume
                                                          true,                    // check overlaps
                                                          0);                      // cpy number


}

void G01DetectorConstruction::TraverseGeoTreeAndCopy(G4VPhysicalVolume * pv, G4int level, G4int maxlevel)
{
    if (!pv) return;
    std::cout << "\t ++ starting TraverseGeoTreeAndCopy with " << pv->GetName() << std::endl;

    G4LogicalVolume* lv = pv->GetLogicalVolume();
    for (int i = 0; i < lv->GetNoDaughters(); ++i) {
        G4VPhysicalVolume* daughter = lv->GetDaughter(i);
//         if (level == cms_subdet_geolevel-1)
        {
            // Children of this level are level 'level+1'
            if (daughter->GetName().find(cms_subdet_name) != std::string::npos) {
              SetDetectorPV(daughter);
              std::cout << "\t ++ New HGCal piece " << daughter->GetName() << std::endl;
            }
        }
        if( level < maxlevel)
          TraverseGeoTreeAndCopy(daughter, level + 1, maxlevel);
    }

}

#endif

