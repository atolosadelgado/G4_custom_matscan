
#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"

#include "G4UserSteppingAction.hh"
#include "G4UserRunAction.hh"

#include "TFile.h"
#include "TH2D.h"
#include "G4Material.hh"

namespace secondaryCounterPerMaterial{


class YourRunAction : public G4UserRunAction {
public:
    YourRunAction(std::string ofilename)
        : fOfilename(ofilename) {}

    virtual ~YourRunAction() {
        if (fFile) {
            fFile->Write();
            fFile->Close();
            delete fFile;
        }
    }

    virtual void BeginOfRunAction(const G4Run*) override {
        G4MaterialTable * mat_table = G4Material::GetMaterialTable();
        int nmaterials = mat_table->size();
        fFile = new TFile(fOfilename.c_str(), "recreate");
        fh2ZEdepMat = new TH2D("fh2ZEdepMat", "Z (mm); material origin; energy (MeV)", 30000, 0, 3000, nmaterials, 0 , nmaterials);
        for(int i = 0; i<nmaterials; ++i){
            G4Material * mat = mat_table->at(i);
            mat_ptr_index_map.emplace(mat, i);
            fh2ZEdepMat->GetYaxis()->SetBinLabel(i+1, mat->GetName());
        }
    }

    virtual void EndOfRunAction(const G4Run*) override {
        fFile->Write();
        fFile->Close();
        delete fFile;
        fFile = nullptr;
    }
    void FillEnergyProfileZ(double eDep_MeV, double zpos_mm, G4Material * origin_material)
    {
        fh2ZEdepMat->Fill(zpos_mm - zoffset_mm, mat_ptr_index_map[origin_material], eDep_MeV);
    }

private:
    TFile* fFile = {nullptr};
    /// 2D histogram, X-axis = Z (mm), Y-axis = material origin; content = energy (MeV)
    TH2D* fh2ZEdepMat = {nullptr};
    std::string fOfilename;
    std::map<G4Material*,int> mat_ptr_index_map;
    double zoffset_mm = 3.0e3*CLHEP::mm; // offset of full geometry
};


class YourSteppingAction : public G4UserSteppingAction {
  public:
    YourSteppingAction(YourRunAction* runAction):G4UserSteppingAction(), _runAction(runAction){}
    ~YourSteppingAction() override {}

    void UserSteppingAction(const G4Step* theStep) override {
        G4Material * mat = theStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetMaterial();
        if( "Silicon" != mat->GetName() ) return;

        const G4double eDep_MeV   = theStep->GetTotalEnergyDeposit() / CLHEP::MeV;
        if(0 == eDep_MeV) return;
        const G4double zpos_mm   = theStep->GetPreStepPoint()->GetPosition().z() / CLHEP::mm;
        G4Material * origin_material = theStep->GetTrack()->GetLogicalVolumeAtVertex()->GetMaterial();
        _runAction->FillEnergyProfileZ(eDep_MeV, zpos_mm, origin_material);

    }
    YourRunAction* _runAction;
};// end event action

}; // end namespace secondaryCounterPerMaterial

#include "MyPrimaryGenerator.hh"

#include "G4VUserActionInitialization.hh"
class secondaryCounterPerMaterial_YourActionInit : public G4VUserActionInitialization {
public:
    secondaryCounterPerMaterial_YourActionInit(std::string ofilename): G4VUserActionInitialization(), fOfilename(ofilename) { }
    ~secondaryCounterPerMaterial_YourActionInit() override {}
    void Build() const override {
        SetUserAction(new MyPrimaryGenerator());
        secondaryCounterPerMaterial::YourRunAction * run_action =  new secondaryCounterPerMaterial::YourRunAction(fOfilename);
        secondaryCounterPerMaterial::YourSteppingAction * stp_action = new secondaryCounterPerMaterial::YourSteppingAction(run_action);
        SetUserAction(run_action);
        SetUserAction(stp_action);
    }
    std::string fOfilename;

};
