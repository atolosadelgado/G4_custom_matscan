#ifndef SECONDARYESCAPECOUNTER_HH
#define SECONDARYESCAPECOUNTER_HH

#include "G4Track.hh"

class SecondaryEscapeCounter {
public:
    SecondaryEscapeCounter(const G4String& particleName = "")
        : fParticleNameFilter(particleName),
          fTotalSecondaries(0), fEscapingSecondaries(0) {}

    void RegisterCreation(const G4Track* track) {
        if (track->GetParentID() > 0) { // Secundario
            if (fParticleNameFilter.empty() || track->GetDefinition()->GetParticleName() == fParticleNameFilter) {
                fTotalSecondaries++;
                fCreationVolume[track->GetTrackID()] = track->GetVolume()->GetName();
            }
        }
    }

    void RegisterEnd(const G4Track* track) {
        if (track->GetParentID() > 0) {
            if (fParticleNameFilter.empty() || track->GetDefinition()->GetParticleName() == fParticleNameFilter) {
                auto it = fCreationVolume.find(track->GetTrackID());
                if (it != fCreationVolume.end()) {
                    G4String creationVol = it->second;
                    G4VPhysicalVolume* endVol = nullptr;
                    if (track->GetStep() != nullptr) {
                        endVol = track->GetVolume();
                    }
                    if (!endVol || endVol->GetName() != creationVol) {
                        fEscapingSecondaries++;
                    }
                    fCreationVolume.erase(it);
                }
            }
        }
    }

    void Reset() {
        fTotalSecondaries = 0;
        fEscapingSecondaries = 0;
        fCreationVolume.clear();
    }

    // Getters
    G4int GetTotalSecondaries() const { return fTotalSecondaries; }
    G4int GetEscapingSecondaries() const { return fEscapingSecondaries; }
    const G4String& GetParticleNameFilter() const { return fParticleNameFilter; }

private:
    G4String fParticleNameFilter; // empty = all
    G4int fTotalSecondaries;
    G4int fEscapingSecondaries;
    std::map<G4int, G4String> fCreationVolume;
};

#endif
