#include "YourEventAction.hh"
#include "YourRunAction.hh"

#include "G4EventManager.hh"

YourEventAction::YourEventAction(YourRunAction * myRunAction)
  : G4UserEventAction(),
  fRunAction(myRunAction) {}


YourEventAction::~YourEventAction() {}

void YourEventAction::BeginOfEventAction(const G4Event* /*anEvent*/) {

  // Initialize once, afterwards it will simply reset the histograms
  fHistogramCollection_map.Initialize("E_vs_z", nbins, zmin, zmax);
}


void YourEventAction::EndOfEventAction(const G4Event* /*anEvent*/) {

  for( auto & it : fHistogramCollection_map.histogramCollection_map){
    TH1D & profile_histogram = *(it.second);
    G4Material * mat = it.first;
    fRunAction->fProfileHistograms.UpdateAverageAndMean(profile_histogram, mat);
  }
}


void YourEventAction::FillEnergyProfileZ(G4double eDep_MeV, G4double zpos_mm, G4Material * mat)
{
  fHistogramCollection_map.Fill(eDep_MeV,zpos_mm,mat);
}
