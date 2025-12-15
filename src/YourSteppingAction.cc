#include "YourSteppingAction.hh"

#include "YourEventAction.hh"

#include "G4Step.hh"
#include "G4EventManager.hh"
#include "G4Electron.hh"

YourSteppingAction::YourSteppingAction(YourEventAction* evtAction)
:   G4UserSteppingAction(),
    fYourEventAction(evtAction),
    rangeTablePb_("estar_pb.txt",11.35),
    rangeTableCu_("estar_cu.txt",8.96),
    electron_definition(G4Electron::Electron())
    {

    matLead_ = G4Material::GetMaterial("Lead");
    if (!matLead_) {
        G4Exception("YourSteppingAction::YourSteppingAction",
                    "MissingMaterial", FatalException,
                    "No Material named Lead found");
    }
    matCopper_ = G4Material::GetMaterial("Copper");
    if (!matCopper_) {
        G4Exception("YourSteppingAction::YourSteppingAction",
                    "MissingMaterial", FatalException,
                    "No Material named Copper found");
    }


    }


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

  // Implementation of range rejection
  ApplyElectronRangeRejectionInLeadAndCopper(theStep);
  // double safety = theStep->GetPostStepPoint()->GetSafety();
  // G4TrackingManager * fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
  // G4SteppingManager* fpSteppingManager = fpTrackingManager->GetSteppingManager();
  // auto secs = fpSteppingManager->GetfSecondary();
  // auto secs = fpTrackingManager->GimmeSecondaries();
}

#include "G4TransportationManager.hh"

void YourSteppingAction::ApplyElectronRangeRejectionInLeadAndCopper(const G4Step* step)
{
    auto pre = step->GetPreStepPoint();
    NistRangeTable * aTable;

    if (pre->GetMaterial() == matLead_)
      aTable = &rangeTablePb_;
    else if (pre->GetMaterial() == matCopper_)
      aTable = &rangeTableCu_;
    else
        return;

    double safety = pre->GetSafety();

    // const G4ThreeVector prestep_position = step->GetPreStepPoint()->GetPosition();
    // const G4ThreeVector poststep_position = step->GetPostStepPoint()->GetPosition();
    // G4ThreeVector avestep_position = 0.5*(prestep_position+poststep_position);
    // avestep_position = G4ThreeVector(avestep_position.x(), avestep_position.y(), avestep_position.z());
    // double safety_solid = pre->GetPhysicalVolume()->GetLogicalVolume()->GetSolid()->DistanceToOut(avestep_position);


    const G4Track* track = step->GetTrack();
    const G4StepPoint* prePoint = step->GetPreStepPoint();
    const G4TouchableHandle& touch = prePoint->GetTouchableHandle();

    // Volumen físico actual (donde está el punto pre-step)
    G4VPhysicalVolume* physVol = touch->GetVolume();
    if (!physVol) return;

    // Obtener el sólido del volumen
    G4VSolid* solid = physVol->GetLogicalVolume()->GetSolid();
    if (!solid) return;

    // Posición y dirección en coordenadas globales
    G4ThreeVector globalPos = prePoint->GetPosition();
    // G4ThreeVector globalDir = track->GetMomentumDirection();

    // Transformación top (history) -> usar TransformPoint / TransformAxis
    // GetTopTransform() devuelve el transform apropiado para convertir
    // coordenadas globales a las locales del volumen "top" en la historia.
    const G4AffineTransform& topTransform = touch->GetHistory()->GetTopTransform();

    // Punto y dirección en coordenadas locales del sólido
    G4ThreeVector localPos   = topTransform.TransformPoint(globalPos);
    // G4ThreeVector localDir   = topTransform.TransformAxis(globalDir);

    G4double safety_solid = solid->DistanceToOut(localPos);




    // std::cout << "\tsafety distance (Track, Solid) : \t"
              // << safety << "\t"
              // << safety_solid << std::endl;

    if (safety_solid <= 0.0)
        return;

    double thrE =  aTable->EnergyFromRange(safety_solid / CLHEP::mm);
    // if range outside data, thrE is inifinity
    if( std::numeric_limits<double>::infinity() == thrE)
      return;

    const auto* secondaries = step->GetSecondaryInCurrentStep();
    if (!secondaries || secondaries->empty())
        return;

    for (auto sec : *secondaries)
    {
        if (sec->GetDefinition() != electron_definition)
            continue;

        // warning removing constantness to change track status
        if (sec->GetKineticEnergy() < thrE)
        {
            const_cast<G4Track*>(sec)->SetTrackStatus(fStopAndKill);
            // G4cout << "E electron ID (step)"
            //         << sec->GetKineticEnergy()
            //         << " " << sec->GetPosition().z()
            //         << " " << sec->GetTrackStatus()
            //         << G4endl;

        }
    }
}
