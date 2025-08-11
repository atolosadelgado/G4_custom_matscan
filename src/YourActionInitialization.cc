
#include "YourActionInitialization.hh"


#include "G01PrimaryGeneratorAction.hh"

#include "YourRunAction.hh"
#include "YourEventAction.hh"
#include "YourSteppingAction.hh"



YourActionInitialization::YourActionInitialization()
:   G4VUserActionInitialization(){ }


YourActionInitialization::~YourActionInitialization() {}

// NOT CALLED IN SEQUENTIAL MODE i.e. ONLY FOR MT:
// - ONLY FOR creating RunAction for the Master thread
// - (will be invoked immediately by the only one G4MTRunManager
//   when the ActionInitialization object is registered in it in the main)
// - this RunAction might be or not the same as for worker threads below
// - see more under the G4VUserActionInitialization class section at
//   https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
void YourActionInitialization::BuildForMaster() const {
  // we will use the same for the Master as for the Workers (IsMaster() can be
  // used to see if a given RunAction object is the one that belongs to the
  // Master thread or not: see for example YourRunAction::EndOfRunAction
  // method for an example of doing something only for the Master RunAction
  // object but not for the Worker RunAction objects)
  //
  // the primary-generator not used in the master RunAction only for the workers
  SetUserAction(new YourRunAction());
}

// Create all User Actions here:
// - for sequential mode (will be invoked immediately by the only one G4RunManager
//   when the ActionInitialization object is registered in it in the main)
// - for worker threads (will be invoked later by all worker G4RunManager-s)
void YourActionInitialization::Build() const {
  // Set UserPrimaryGeneratorAction
  G01PrimaryGeneratorAction* primaryAction = new G01PrimaryGeneratorAction();
  SetUserAction(primaryAction);
  // Set UserRunAction
  YourRunAction* runAction = new YourRunAction();
  SetUserAction(runAction);
  // Set UserEventAction
  YourEventAction* eventAction = new YourEventAction(runAction);
  SetUserAction(eventAction);
  // Set UserSteppingAction
  SetUserAction( new YourSteppingAction(eventAction) );
}
