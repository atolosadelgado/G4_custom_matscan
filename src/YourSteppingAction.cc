#include "YourSteppingAction.hh"

#include "YourEventAction.hh"

#include "G4Step.hh"


YourSteppingAction::YourSteppingAction(YourEventAction* evtAction)
:   G4UserSteppingAction(),
    fYourEventAction(evtAction) { }


YourSteppingAction::~YourSteppingAction() {}


void YourSteppingAction::UserSteppingAction(const G4Step* theStep) {

  const G4double eDep_MeV   = theStep->GetTotalEnergyDeposit() / CLHEP::MeV;
  if(0 == eDep_MeV) return;
  const G4ThreeVector prestep_position = theStep->GetPreStepPoint()->GetPosition();
  const G4ThreeVector poststep_position = theStep->GetPostStepPoint()->GetPosition();
  G4ThreeVector avestep_position = 0.5*(prestep_position+poststep_position);
  const G4double zpos_mm   = prestep_position.z() / CLHEP::mm;
  // for consistency, take z from prestep
  avestep_position = G4ThreeVector(avestep_position.x(), avestep_position.y(), prestep_position.z());
  G4Material * mat = theStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetMaterial();
  G4double time_ps = theStep->GetPostStepPoint()->GetGlobalTime() / CLHEP::ps;

  fYourEventAction->FillEnergyProfileZ(eDep_MeV, zpos_mm, mat);
  fYourEventAction->FillEnergyProfileXY(eDep_MeV, avestep_position, mat);
  fYourEventAction->FillEnergyTimeProfile(time_ps, eDep_MeV, mat);

}
