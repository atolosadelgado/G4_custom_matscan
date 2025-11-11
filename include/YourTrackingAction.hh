#ifndef YOURTRACKINGACTION_HH
#define YOURTRACKINGACTION_HH

#include "SecondaryEscapeCounter.hh"
#include "G4UserTrackingAction.hh"

class YourTrackingAction : public G4UserTrackingAction {
public:
    YourTrackingAction()
        : counterAll(""),          // all
          counterElectrons("e-"),  // only e-
          counterGammas("gamma")   // only gamma
    {}

    void PreUserTrackingAction(const G4Track* track) override {
        counterAll.RegisterCreation(track);
        counterElectrons.RegisterCreation(track);
        counterGammas.RegisterCreation(track);
    }

    void PostUserTrackingAction(const G4Track* track) override {
        counterAll.RegisterEnd(track);
        counterElectrons.RegisterEnd(track);
        counterGammas.RegisterEnd(track);
    }

    void Reset() {
        counterAll.Reset();
        counterElectrons.Reset();
        counterGammas.Reset();
    }

    const SecondaryEscapeCounter& GetCounterAll() const { return counterAll; }
    const SecondaryEscapeCounter& GetCounterElectrons() const { return counterElectrons; }
    const SecondaryEscapeCounter& GetCounterGammas() const { return counterGammas; }

private:
    SecondaryEscapeCounter counterAll;
    SecondaryEscapeCounter counterElectrons;
    SecondaryEscapeCounter counterGammas;
};

#endif
