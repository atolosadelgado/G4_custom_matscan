#ifndef YOURTRACKINGACTION_HH
#define YOURTRACKINGACTION_HH

#include "SecondaryEscapeCounter.hh"
#include "G4UserTrackingAction.hh"
#include "G4Electron.hh"
#include "YourRunAction.hh"

class YourTrackingAction : public G4UserTrackingAction {
public:
    YourTrackingAction(YourRunAction * runAction)
        : G4UserTrackingAction(), fRunAction(runAction) {}

    void PreUserTrackingAction(const G4Track* track) override {
        fRunAction->counterAll.RegisterCreation(track);
        fRunAction->counterElectrons.RegisterCreation(track);
        fRunAction->counterGammas.RegisterCreation(track);
        // if (track->GetDefinition() == G4Electron::Electron() )
            // G4cout << "E electron ID (track)"
            //         << track->GetKineticEnergy()
            //         << " " << track->GetPosition().z()
            //         << " " << track->GetTrackStatus()
            //         << G4endl;

    }

    void PostUserTrackingAction(const G4Track* track) override {
        fRunAction->counterAll.RegisterEnd(track);
        fRunAction->counterElectrons.RegisterEnd(track);
        fRunAction->counterGammas.RegisterEnd(track);
    }

private:
    YourRunAction * fRunAction;
};

#endif
