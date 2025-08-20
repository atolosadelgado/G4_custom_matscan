#ifndef YOURDETECTORCONSTRUCTOR
#define YOURDETECTORCONSTRUCTOR


#include "G4VUserDetectorConstruction.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"
#include "G4GDMLParser.hh"

/// Detector construction allowing to use the geometry read from the GDML file

class YourDetectorConstructor : public G4VUserDetectorConstruction
{
  public:

    void SetGDMLfilename(std::string s){gdml_filename=s;};
    virtual G4VPhysicalVolume* Construct();

  private:
      std::string gdml_filename;

};

G4VPhysicalVolume * YourDetectorConstructor::Construct()
{
  G4GDMLParser Parser;
  Parser.Read(gdml_filename, false);
  auto worldPV = Parser.GetWorldVolume();
  return worldPV;
}


#endif
