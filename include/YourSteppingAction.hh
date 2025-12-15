#ifndef YOURSTEPPINGACTION_HH
#define YOURSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

#include "NistRangeTable.hh"

class YourEventAction;
class G4Material;
class G4Electron;

class YourSteppingAction : public G4UserSteppingAction {
  public:
    YourSteppingAction(YourEventAction* evtAction);
    ~YourSteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;
    void ApplyElectronRangeRejectionInLeadAndCopper(const G4Step* step);
  private:

    YourEventAction*             fYourEventAction;
    NistRangeTable               rangeTablePb_;
    G4Material* matLead_;
    NistRangeTable               rangeTableCu_;
    G4Material* matCopper_;
    G4Electron * electron_definition;

};

#endif
