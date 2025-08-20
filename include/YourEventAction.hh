
#ifndef YOUREVENTACTION_HH
#define YOUREVENTACTION_HH

#include "G4Material.hh"

#include "G4UserEventAction.hh"
#include "YourRunAction.hh"

#include "HistogramCollection.hh"

class YourEventAction : public G4UserEventAction {
public:

  YourEventAction(YourRunAction * myRunAction);
  ~YourEventAction() override;

  void BeginOfEventAction(const G4Event* evt) override;
  void EndOfEventAction(const G4Event* evt) override;

  // Fill profile histogram
  void FillEnergyProfileZ(G4double eDep_MeV, G4double zpos_mm, G4Material * mat);

  // define histogram bining and range
  G4int nbins = 30000;
  G4double zmin = 0000*CLHEP::mm;
  G4double zmax = 3000*CLHEP::mm;
  // this is done
  G4double zoffset_mm = 0;

  void SetOffset(){
    if(!fRunAction) return;
    bool is_test_beam_geometry = fRunAction->_ofilename.find("TBHGCal181Oct") != std::string::npos;
    if( is_test_beam_geometry )
      zoffset_mm = 2.5e4*CLHEP::mm; // offset of test beam geometry
    else
      zoffset_mm = 3.0e3*CLHEP::mm; // offset of full geometry
  }

private:
  YourRunAction * fRunAction;

  HistogramCollection fHistogramCollection_map;


};

#endif
