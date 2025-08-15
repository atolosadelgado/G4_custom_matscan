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
  const G4double zpos_mm   = theStep->GetPreStepPoint()->GetPosition().z() / CLHEP::mm;
  G4Material * mat = theStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetMaterial();
  fYourEventAction->FillEnergyProfileZ(eDep_MeV, zpos_mm -2.5e4*CLHEP::mm /*offset of geometry*/, mat);
}
