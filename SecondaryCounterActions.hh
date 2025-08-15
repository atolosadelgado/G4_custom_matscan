#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"

// SecondaryEscapeCounter.hh
#pragma once
#include <map>
#include <string>
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

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

#include "G4UserTrackingAction.hh"
class TrkActionForSecondaries : public G4UserTrackingAction {
public:
    TrkActionForSecondaries()
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


#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "TH1D.h"
#include "TFile.h"

class RunActionForSecondaries : public G4UserRunAction {
public:
    RunActionForSecondaries(std::string ofilename)
        : fOfilename(ofilename) {}

    virtual ~RunActionForSecondaries() {
        if (fFile) {
            fFile->Write();
            fFile->Close();
            delete fFile;
        }
    }

    virtual void BeginOfRunAction(const G4Run*) override {
        fFile = new TFile(fOfilename.c_str(), "update");
        fHistTotal = (TH1D*) fFile->Get("hTotalSecondaries");
        if(nullptr == fHistTotal)
            fHistTotal = new TH1D("hTotalSecondaries", "Total secondaries per event", 1000, 0, 1e5);
        fHistElectrons = (TH1D*) fFile->Get("hElectrons");
        if(nullptr == fHistElectrons)
            fHistElectrons = new TH1D("hElectrons", "Electrons per event", 1000, 0, 1e5);
        fHistGammas = (TH1D*) fFile->Get("hGammas");
        if(nullptr == fHistGammas)
        fHistGammas = new TH1D("hGammas", "Gammas per event", 1000, 0, 1e5);

        fHistTotalEscaping = (TH1D*) fFile->Get("hTotalSecondariesEscaping");
        if(nullptr == fHistTotalEscaping)
            fHistTotalEscaping = new TH1D("hTotalSecondariesEscaping", "Total secondaries Escaping per event", 1000, 0, 1e5);
        fHistElectronsEscaping = (TH1D*) fFile->Get("hElectronsEscaping");
        if(nullptr == fHistElectronsEscaping)
            fHistElectronsEscaping = new TH1D("hElectronsEscaping", "Electrons Escaping per event", 1000, 0, 1e5);
        fHistGammasEscaping = (TH1D*) fFile->Get("hGammasEscaping");
        if(nullptr == fHistGammasEscaping)
        fHistGammasEscaping = new TH1D("hGammasEscaping", "Gammas Escaping per event", 1000, 0, 1e5);
    }

    virtual void EndOfRunAction(const G4Run*) override {
        fFile->Write();
        fFile->Close();
        delete fFile;
        fFile = nullptr;
    }

    TH1D* GetHistTotal() { return fHistTotal; }
    TH1D* GetHistElectrons() { return fHistElectrons; }
    TH1D* GetHistGammas() { return fHistGammas; }
    TH1D* GetHistTotalEscaping() { return fHistTotalEscaping; }
    TH1D* GetHistElectronsEscaping() { return fHistElectronsEscaping; }
    TH1D* GetHistGammasEscaping() { return fHistGammasEscaping; }

private:
    TFile* fFile = {nullptr};
    TH1D* fHistTotal = {nullptr};
    TH1D* fHistElectrons = {nullptr};
    TH1D* fHistGammas = {nullptr};
    TH1D* fHistTotalEscaping = {nullptr};
    TH1D* fHistElectronsEscaping = {nullptr};
    TH1D* fHistGammasEscaping = {nullptr};
    std::string fOfilename;
};

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4ios.hh"

class EvtActionForSecondaries : public G4UserEventAction {
public:
    EvtActionForSecondaries(TrkActionForSecondaries* trackingAction, RunActionForSecondaries* runAction)
        : fTrackingAction(trackingAction), fRunAction(runAction) {}

    virtual ~EvtActionForSecondaries() {}

    virtual void BeginOfEventAction(const G4Event*) override {
        fTrackingAction->Reset();
    }

    virtual void EndOfEventAction(const G4Event* ) override {

        fRunAction->GetHistTotal()->Fill(fTrackingAction->GetCounterAll().GetTotalSecondaries());
        fRunAction->GetHistElectrons()->Fill(fTrackingAction->GetCounterElectrons().GetTotalSecondaries());
        fRunAction->GetHistGammas()->Fill(fTrackingAction->GetCounterGammas().GetTotalSecondaries());

        fRunAction->GetHistTotalEscaping()->Fill(fTrackingAction->GetCounterAll().GetEscapingSecondaries());
        fRunAction->GetHistElectronsEscaping()->Fill(fTrackingAction->GetCounterElectrons().GetEscapingSecondaries());
        fRunAction->GetHistGammasEscaping()->Fill(fTrackingAction->GetCounterGammas().GetEscapingSecondaries());
    }

private:
    // SecondaryCounterTrackingAction* fTrackingAction;
    TrkActionForSecondaries* fTrackingAction;
    RunActionForSecondaries* fRunAction;
};

#include "G4VUserActionInitialization.hh"
class YourActionInitialization : public G4VUserActionInitialization {
public:
    YourActionInitialization(std::string ofilename): G4VUserActionInitialization(), fOfilename(ofilename) { }
    ~YourActionInitialization() override {}
    void Build() const override {
        SetUserAction(new G01PrimaryGeneratorAction());
        RunActionForSecondaries * run =  new RunActionForSecondaries(fOfilename);
        // SecondaryCounterTrackingAction * trk = new SecondaryCounterTrackingAction();
        TrkActionForSecondaries * trk = new TrkActionForSecondaries();
        EvtActionForSecondaries * evt = new EvtActionForSecondaries(trk,run);
        SetUserAction(run);
        SetUserAction(evt);
        SetUserAction(trk);
    }
    std::string fOfilename;

};


// class SecondaryCounterTrackingAction : public G4UserTrackingAction {
// public:
//     SecondaryCounterTrackingAction()
//         : fTotalSecondaries(0),
//           fElectrons(0), fPositrons(0), fGammas(0),
//           fElectronsSi(0), fPositronsSi(0), fGammasSi(0),
//           fElectronsOther(0), fPositronsOther(0), fGammasOther(0)
//     {}
//
//     virtual ~SecondaryCounterTrackingAction() {}
//
//     virtual void PreUserTrackingAction(const G4Track* track) override {
//         if (track->GetParentID() > 0) { // Secundario
//             fTotalSecondaries++;
//
//             G4String particleName = track->GetDefinition()->GetParticleName();
//             G4String materialName = track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName();
//
//             bool isSi = (materialName == "Silicon");
//
//             if (particleName == "e-") {
//                 fElectrons++;
//                 if (isSi) fElectronsSi++;
//                 else fElectronsOther++;
//             } else if (particleName == "e+") {
//                 fPositrons++;
//                 if (isSi) fPositronsSi++;
//                 else fPositronsOther++;
//             } else if (particleName == "gamma") {
//                 fGammas++;
//                 if (isSi) fGammasSi++;
//                 else fGammasOther++;
//             }
//         }
//     }
//
//     void Reset() {
//         fTotalSecondaries = 0;
//         fElectrons = fPositrons = fGammas = 0;
//         fElectronsSi = fPositronsSi = fGammasSi = 0;
//         fElectronsOther = fPositronsOther = fGammasOther = 0;
//     }
//
//     // Accesores
//     G4int GetTotalSecondaries() const { return fTotalSecondaries; }
//
//     G4int GetElectrons() const { return fElectrons; }
//     G4int GetPositrons() const { return fPositrons; }
//     G4int GetGammas() const { return fGammas; }
//
//     G4int GetElectronsSi() const { return fElectronsSi; }
//     G4int GetPositronsSi() const { return fPositronsSi; }
//     G4int GetGammasSi() const { return fGammasSi; }
//
//     G4int GetElectronsOther() const { return fElectronsOther; }
//     G4int GetPositronsOther() const { return fPositronsOther; }
//     G4int GetGammasOther() const { return fGammasOther; }
//
// private:
//     G4int fTotalSecondaries;
//
//     G4int fElectrons;
//     G4int fPositrons;
//     G4int fGammas;
//
//     G4int fElectronsSi;
//     G4int fPositronsSi;
//     G4int fGammasSi;
//
//     G4int fElectronsOther;
//     G4int fPositronsOther;
//     G4int fGammasOther;
// };
