#ifndef SECONDARYESCAPECOUNTER_HH
#define SECONDARYESCAPECOUNTER_HH

#include "G4Track.hh"

#include "TH1D.h"
#include "TFile.h"

class SecondaryEscapeCounter {
public:
    SecondaryEscapeCounter(const G4String & particleName = "")
        : fParticleNameFilter(particleName),
          fTotalSecondaries(0), fEscapingSecondaries(0) {
              std::string fHistTotalName;
              std::string fHistEscapingName;
              std::string fHistZposName;
              if(fParticleNameFilter.empty())
              {
                  fHistTotalName = "hTotalSecondaries";
                  fHistEscapingName = "hTotalSecondariesEscaping";
                  fHistZposName = "fHistZpos";
                  zpos = true;
              }
              else {
                  std::string s = particleName;
                  // remove non alpha characters
                  s.erase(std::remove_if(s.begin(), s.end(),
                           [](unsigned char c){ return !std::isalpha(c); }),
                    s.end());
                  fHistTotalName = "hTotal" + s;
                  fHistEscapingName = "hTotalEscaping" + s;
              }
              fHistTotal = std::make_unique<TH1D>(fHistTotalName.c_str(),fHistTotalName.c_str(), 30000, 0, 3e6);
              fHistEscaping = std::make_unique<TH1D>(fHistEscapingName.c_str(),fHistEscapingName.c_str(), 30000, 0, 3e6);
              if(true == zpos){
                fHistZpos = std::make_unique<TH1D>(fHistZposName.c_str(),fHistZposName.c_str(), 180000, 0, 3000.0000);
                fHistZpos->SetDirectory(0);
                }

              fHistTotal->SetDirectory(0);
              fHistEscaping->SetDirectory(0);


        }

    void RegisterCreation(const G4Track* track) {
        if( G4TrackStatus::fStopAndKill == track->GetTrackStatus() ) return;
        if( nullptr != fHistZpos.get() ) fHistZpos->Fill(track->GetPosition().z()/CLHEP::mm - 3000.0000);
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

    void FillHistogramsAndResetCounters() {
        fHistTotal->Fill(fTotalSecondaries);
        fHistEscaping->Fill(fEscapingSecondaries);
        fTotalSecondaries = 0;
        fEscapingSecondaries = 0;
        fCreationVolume.clear();
    }

    void WriteHistogram(TFile * f) {
        f->cd();
        fHistTotal->Write();
        fHistEscaping->Write();
        if( nullptr != fHistZpos.get() ) fHistZpos->Write();
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

    std::unique_ptr<TH1D> fHistTotal;
    std::unique_ptr<TH1D> fHistEscaping;
    std::unique_ptr<TH1D> fHistZpos;
    bool zpos = false;
};

#endif
