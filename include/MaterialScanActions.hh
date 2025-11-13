#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"

// SecondaryEscapeCounter.hh
// #pragma once
#include <map>
#include <string>
#include "G4VPhysicalVolume.hh"

#include "G4UserRunAction.hh"
#include "G4Run.hh"

struct matscan_step_info_t{
    G4double steplength;
    G4String volname;
    G4String matname;
};

class RunActionMatScan : public G4UserRunAction {
public:
    RunActionMatScan(std::string ofilename)
        : fOfilename(ofilename) {}

    virtual ~RunActionMatScan() {}

    virtual void BeginOfRunAction(const G4Run*) override {}
    virtual void EndOfRunAction  (const G4Run*) override {}
    void AddStep(matscan_step_info_t stepinfo){ stepinfo_v.push_back(stepinfo); }
    void FlushToFileAndReset();
    std::string fOfilename;
    std::vector<matscan_step_info_t> stepinfo_v;
};

void RunActionMatScan::FlushToFileAndReset()
{
    std::ofstream ofile(fOfilename, std::ios::app);
    for( auto & s : stepinfo_v)
        ofile << s.steplength << "\t" << s.volname << "\t" << s.matname << std::endl;
    stepinfo_v.clear();
}



#include "G4UserEventAction.hh"
#include "G4Event.hh"

class EventActionMatScan : public G4UserEventAction {
public:
    EventActionMatScan(RunActionMatScan* runAction)
        : fRunAction(runAction) {}

    virtual ~EventActionMatScan() {}

    virtual void BeginOfEventAction(const G4Event*) override {}

    virtual void EndOfEventAction  (const G4Event*) override { fRunAction->FlushToFileAndReset();}

private:
    RunActionMatScan* fRunAction;
};

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"

class StepActionMatScan : public G4UserSteppingAction {
public:
    StepActionMatScan(RunActionMatScan * runaction) : fRunAction(runaction) {}
    virtual ~StepActionMatScan() {}
    void UserSteppingAction(const G4Step* step) override;
private:
    RunActionMatScan * fRunAction;
};

void StepActionMatScan::UserSteppingAction(const G4Step* step)
{
    auto steplength = step->GetStepLength();
    auto volname = step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
    auto matname = step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetMaterial()->GetName();
    fRunAction->AddStep( {steplength, volname, matname} );
}


#include "MyPrimaryGenerator.hh"

#include "G4VUserActionInitialization.hh"
class ActionInitializationForMatScan : public G4VUserActionInitialization {
public:
    ActionInitializationForMatScan(std::string ofilename): G4VUserActionInitialization(), fOfilename(ofilename) { }
    ~ActionInitializationForMatScan() override {}
    void Build() const override {
        SetUserAction(new MyPrimaryGenerator());
        RunActionMatScan * run =  new RunActionMatScan(fOfilename);
        EventActionMatScan * evt = new EventActionMatScan(run);
        StepActionMatScan * stp  = new StepActionMatScan(run);
        SetUserAction(run);
        SetUserAction(evt);
        SetUserAction(stp);
    }
    std::string fOfilename;

};
