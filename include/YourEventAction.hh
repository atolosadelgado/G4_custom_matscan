
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
  G4int nbins = 3000;
  G4double zmin = 3000*CLHEP::mm;
  G4double zmax = 6000*CLHEP::mm;

private:
  YourRunAction * fRunAction;

  HistogramCollection fHistogramCollection_map;


};

#endif
