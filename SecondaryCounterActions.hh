#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"

class SecondaryCounterTrackingAction : public G4UserTrackingAction {
public:
    SecondaryCounterTrackingAction()
        : fTotalSecondaries(0),
          fElectrons(0), fPositrons(0), fGammas(0),
          fElectronsSi(0), fPositronsSi(0), fGammasSi(0),
          fElectronsOther(0), fPositronsOther(0), fGammasOther(0)
    {}

    virtual ~SecondaryCounterTrackingAction() {}

    virtual void PreUserTrackingAction(const G4Track* track) override {
        if (track->GetParentID() > 0) { // Secundario
            fTotalSecondaries++;

            G4String particleName = track->GetDefinition()->GetParticleName();
            G4String materialName = track->GetVolume()->GetLogicalVolume()->GetMaterial()->GetName();

            bool isSi = (materialName == "Silicon");

            if (particleName == "e-") {
                fElectrons++;
                if (isSi) fElectronsSi++;
                else fElectronsOther++;
            } else if (particleName == "e+") {
                fPositrons++;
                if (isSi) fPositronsSi++;
                else fPositronsOther++;
            } else if (particleName == "gamma") {
                fGammas++;
                if (isSi) fGammasSi++;
                else fGammasOther++;
            }
        }
    }

    void Reset() {
        fTotalSecondaries = 0;
        fElectrons = fPositrons = fGammas = 0;
        fElectronsSi = fPositronsSi = fGammasSi = 0;
        fElectronsOther = fPositronsOther = fGammasOther = 0;
    }

    // Accesores
    G4int GetTotalSecondaries() const { return fTotalSecondaries; }

    G4int GetElectrons() const { return fElectrons; }
    G4int GetPositrons() const { return fPositrons; }
    G4int GetGammas() const { return fGammas; }

    G4int GetElectronsSi() const { return fElectronsSi; }
    G4int GetPositronsSi() const { return fPositronsSi; }
    G4int GetGammasSi() const { return fGammasSi; }

    G4int GetElectronsOther() const { return fElectronsOther; }
    G4int GetPositronsOther() const { return fPositronsOther; }
    G4int GetGammasOther() const { return fGammasOther; }

private:
    G4int fTotalSecondaries;

    G4int fElectrons;
    G4int fPositrons;
    G4int fGammas;

    G4int fElectronsSi;
    G4int fPositronsSi;
    G4int fGammasSi;

    G4int fElectronsOther;
    G4int fPositronsOther;
    G4int fGammasOther;
};

#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "TH1D.h"
#include "TFile.h"

class MyRunAction : public G4UserRunAction {
public:
    MyRunAction(std::string ofilename)
        : fFile(nullptr), fHistTotal(nullptr),
          fHistElectrons(nullptr), fHistPositrons(nullptr), fHistGammas(nullptr),fOfilename(ofilename) {}

    virtual ~MyRunAction() {
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
        fHistPositrons = (TH1D*) fFile->Get("hPositrons");
        if(nullptr == fHistPositrons)
        fHistPositrons = new TH1D("hPositrons", "Positrons per event", 1000, 0, 1e5);
        fHistGammas = (TH1D*) fFile->Get("hGammas");
        if(nullptr == fHistGammas)
        fHistGammas = new TH1D("hGammas", "Gammas per event", 1000, 0, 1e5);
    }

    virtual void EndOfRunAction(const G4Run*) override {
        fFile->Write();
        fFile->Close();
        delete fFile;
        fFile = nullptr;
    }

    // Accesores para EventAction
    TH1D* GetHistTotal() { return fHistTotal; }
    TH1D* GetHistElectrons() { return fHistElectrons; }
    TH1D* GetHistPositrons() { return fHistPositrons; }
    TH1D* GetHistGammas() { return fHistGammas; }

private:
    TFile* fFile;
    TH1D* fHistTotal;
    TH1D* fHistElectrons;
    TH1D* fHistPositrons;
    TH1D* fHistGammas;
    std::string fOfilename;
};

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4ios.hh"

class MyEventAction : public G4UserEventAction {
public:
    MyEventAction(SecondaryCounterTrackingAction* trackingAction, MyRunAction* runAction)
        : fTrackingAction(trackingAction), fRunAction(runAction) {}

    virtual ~MyEventAction() {}

    virtual void BeginOfEventAction(const G4Event*) override {
        fTrackingAction->Reset();
    }

    virtual void EndOfEventAction(const G4Event* ) override {
        // Llenar histogramas
        fRunAction->GetHistTotal()->Fill(fTrackingAction->GetTotalSecondaries());
        fRunAction->GetHistElectrons()->Fill(fTrackingAction->GetElectrons());
        fRunAction->GetHistPositrons()->Fill(fTrackingAction->GetPositrons());
        fRunAction->GetHistGammas()->Fill(fTrackingAction->GetGammas());
    }

private:
    SecondaryCounterTrackingAction* fTrackingAction;
    MyRunAction* fRunAction;
};

