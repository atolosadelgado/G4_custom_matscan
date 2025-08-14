#ifndef YOURSTEPPINGACTION_HH
#define YOURSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

class YourEventAction;

class YourSteppingAction : public G4UserSteppingAction {
  public:
    YourSteppingAction(YourEventAction* evtAction);
    ~YourSteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

  private:

    YourEventAction*             fYourEventAction;
};

#endif
