
#include "YourRunAction.hh"

#include "G4EventManager.hh"
#include "YourEventAction.hh"

#include "TH1D.h"

YourRunAction::YourRunAction(std::string ofilename):G4UserRunAction(),_ofilename(ofilename){}


YourRunAction::~YourRunAction() {}


void YourRunAction::BeginOfRunAction(const G4Run* ) {

    G4EventManager * evt_manager = G4EventManager::GetEventManager();
    if(nullptr == evt_manager) throw std::runtime_error("Event manager not found, null pointer!");

    YourEventAction * your_evt_action = static_cast<YourEventAction*>( evt_manager->GetUserEventAction() );
    if(nullptr == your_evt_action) throw std::runtime_error("Custom event action not found, null pointer!");


    auto nbins = your_evt_action->nbins;
    auto zmin = your_evt_action->zmin;
    auto zmax = your_evt_action->zmax;

    this->fProfileZHistograms.Initialize("E_vs_z_averaged", "Deposited energy as function of Z; Z (mm); E (MeV)", nbins, zmin, zmax);
    this->fProfileXYHistograms.Initialize("R_vs_z_averaged", "Radius (XY, energy weighted) as function of Z; Z (mm); R (mm)", nbins, zmin, zmax);
    hSamplingFraction = std::make_unique<TH1D>("hSamplingFraction","Visible energy, normalized by deposited energy;;E_{vis}/E_{dep}",50000,0.,0.01);
    // to avoid double deletion, set null directory
    hSamplingFraction->SetDirectory(nullptr);

    hEnergyLeakage = std::make_unique<TH1D>("hEnergyLeakage","Fraction of leaked energy;;1-E_{vis}/E_{0}",50000,0.,0.1);
    hEnergyLeakage->SetDirectory(nullptr);

}

void YourRunAction::EndOfRunAction(const G4Run* ){
    this->fProfileZHistograms.SaveRootfile(_ofilename);
    this->fProfileXYHistograms.SaveRootfile(_ofilename);

    TFile * ofile = TFile::Open(_ofilename.c_str(), "update");
    // take ownership from hEvis and pass it to the TFile
    TH1D* raw_ptr = hSamplingFraction.release();
    raw_ptr->SetDirectory(ofile);
    raw_ptr->Write();
    raw_ptr = hEnergyLeakage.release();
    raw_ptr->SetDirectory(ofile);
    raw_ptr->Write();
    ofile->Close();
}


