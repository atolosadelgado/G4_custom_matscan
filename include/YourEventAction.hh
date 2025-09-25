
#ifndef YOUREVENTACTION_HH
#define YOUREVENTACTION_HH

class G4Material;

#include "G4UserEventAction.hh"
#include "YourRunAction.hh"

#include "HistogramCollection.hh"
#include "G4ThreeVector.hh"
#include "MyPrimaryGenerator.hh"

class YourEventAction : public G4UserEventAction {
public:

  YourEventAction(YourRunAction * myRunAction, MyPrimaryGenerator* gen);
  ~YourEventAction() override;

  void BeginOfEventAction(const G4Event* evt) override;
  void EndOfEventAction(const G4Event* evt) override;

  // Fill profile histogram
  void FillEnergyProfileZ(G4double eDep_MeV, G4double zpos_mm, G4Material * mat);

  // Fill profile histogram
  void FillEnergyProfileXY(G4double eDep_MeV, G4ThreeVector avestep_position, G4Material * mat);

  // define histogram bining and range
  G4int nbins = 180000;
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
  std::unique_ptr<TH1D> MakeRMS_from_M2_Mean(const std::unique_ptr<TH1D> & M2, const std::unique_ptr<TH1D> & Mean);

private:
  YourRunAction * fRunAction;
  MyPrimaryGenerator * fPrimaryGenerator;
  double event_energy_in_sensitiveVols_MeV = 0;
  double event_energy_in_allVols_MeV = 0;

  // map of material-histogram, to be filled during one
  // event, and reset before starting next one
  HistogramCollection fHistogramCollectionProfileZ_map;
  HistogramCollection fHistogramCollectionProfileXY_map;


};

#endif
