
#include "YourEventAction.hh"

#include "G4EventManager.hh"

#ifdef G4MULTITHREADED
#  include "G4MTRunManager.hh"
#  include "G4Threading.hh"
#else
#  include "G4RunManager.hh"
#endif

// #include "YourRun.hh"
#include "YourRunAction.hh"
#include "G4Run.hh"

YourEventAction::YourEventAction(YourRunAction * myRunAction)
  : G4UserEventAction(),
  fRunAction(myRunAction) {}


YourEventAction::~YourEventAction() {}


// Beore each event: reset per-event variables
void YourEventAction::BeginOfEventAction(const G4Event* /*anEvent*/) {

  // Get current event ID
  G4int eventID  = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();

  // Get current run ID
  G4int runID    = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();

  // Get thread ID (0 for sequential mode)
#ifdef G4MULTITHREADED
  G4int threadID = G4Threading::G4GetThreadId();
#else
  G4int threadID = 0;
#endif

  // use these IDs to create unique histogram name
  G4String histo_basename = "E_vs_z";
  G4String histo_name = histo_basename + threadID + runID + eventID;

  // define histogram bining and range
  G4int nbins = 3000;
  G4double zmin = 3000*CLHEP::mm;
  G4double zmax = 6000*CLHEP::mm;

  // create histogram
  eventEnergyProfileZ = std::make_unique<tools::histo::h1d>(histo_name, nbins, zmin, zmax);
}


// After each event:
// fill the data collected for this event into the Run global (thread local)
// data Run data object (i.e. into YourRun)
void YourEventAction::EndOfEventAction(const G4Event* /*anEvent*/) {
  // // get the current Run object and cast it to YourRun (because for sure this is its type)
  // YourRun* currentRun = static_cast< YourRun* > ( G4RunManager::GetRunManager()->GetNonConstCurrentRun() );
  //   // add the quantities to the (thread local) run global YourRun object
  //   currentRun->AddEnergyDepositPerEvent( fEdepPerEvt );
  //   currentRun->AddChTrackLengthPerEvent( fChTrackLengthPerEvt );
  //   currentRun->FillEdepHistogram( fEdepPerEvt );

  // move object and ownership to run action
  fRunAction->UpdateAveragedProfileHistogram( std::move(eventEnergyProfileZ) );
  // eventEnergyProfileZ is now null
}

void YourEventAction::FillEnergyProfileZ(G4double eDep_MeV, G4double zpos_mm)
{
  eventEnergyProfileZ->fill(zpos_mm, eDep_MeV);
}
