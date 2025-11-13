
#include "YourRunAction.hh"

#include "G4EventManager.hh"
#include "YourEventAction.hh"

#include "TH1D.h"

#include "G4Material.hh"

YourRunAction::YourRunAction(std::string ofilename):
          G4UserRunAction(),
          _ofilename(ofilename),
          counterAll(""),          // all
          counterElectrons("e-"),  // only e-
          counterGammas("gamma")   // only gamma
          {}


YourRunAction::~YourRunAction() {}


void YourRunAction::BeginOfRunAction(const G4Run* ) {

    G4EventManager * evt_manager = G4EventManager::GetEventManager();
    if(nullptr == evt_manager) throw std::runtime_error("Event manager not found, null pointer!");

    YourEventAction * your_evt_action = static_cast<YourEventAction*>( evt_manager->GetUserEventAction() );
    if(nullptr == your_evt_action) throw std::runtime_error("Custom event action not found, null pointer!");


    auto nbins_zprofile = your_evt_action->nbins_zprofile;
    auto zmin_zprofile = your_evt_action->zmin_zprofile;
    auto zmax_zprofile = your_evt_action->zmax_zprofile;

    this->fProfileZHistograms.Initialize("E_vs_z_averaged", "Deposited energy as function of Z; Z (mm); E (MeV)", nbins_zprofile, zmin_zprofile, zmax_zprofile);
    this->fProfileXYHistograms.Initialize("R_vs_z_averaged", "Radius (XY, energy weighted) as function of Z; Z (mm); R (mm)", nbins_zprofile, zmin_zprofile, zmax_zprofile);

    auto nbins_timeEnergy_prof = your_evt_action->fHistogramCollectionTimeVsEdep_map.nbins;
    auto tmin_timeEnergy_prof = your_evt_action->fHistogramCollectionTimeVsEdep_map.zmin;
    auto tmax_timeEnergy_prof = your_evt_action->fHistogramCollectionTimeVsEdep_map.zmax;
    this->fProfileTimeEnergyHistograms.Initialize("E_vs_t_averaged","Deposited energy as function of time; Time (ps); E(MeV)",
                                                  nbins_timeEnergy_prof, tmin_timeEnergy_prof, tmax_timeEnergy_prof);

    hSamplingFraction = std::make_unique<TH1D>("hSamplingFraction","Visible energy, normalized by deposited energy;;E_{vis}/E_{dep}",50000,0.,0.01);
    // to avoid double deletion, set null directory
    hSamplingFraction->SetDirectory(nullptr);

    hEnergyLeakage = std::make_unique<TH1D>("hEnergyLeakage","Fraction of leaked energy;;1-E_{vis}/E_{0}",50000,0.,0.1);
    hEnergyLeakage->SetDirectory(nullptr);

    hTimePerEvent_ms = std::make_unique<TH1D>("hTimePerEvent","Time per event;Time (ms);",2e5, 0.,1e4);

    if( 0 == sensitive_mats.size() )
    {
        sensitive_mats.push_back( G4Material::GetMaterial("Silicon") );
        sensitive_mats.push_back( G4Material::GetMaterial("H_Scintillator") );
    }

    if( 0 == nonDetector_mats.size() )
    {
        nonDetector_mats.push_back( G4Material::GetMaterial("Air") );
        nonDetector_mats.push_back( G4Material::GetMaterial("Galactic") );
    }

}

void YourRunAction::EndOfRunAction(const G4Run* ){
    this->WriteOutputFile();
}

void YourRunAction::WriteOutputFile()
{
    TFile * ofile = TFile::Open(_ofilename.c_str(), "recreate");
    // take ownership from hEvis and pass it to the TFile
    TH1D* raw_ptr = hSamplingFraction.get();
    raw_ptr->SetDirectory(0);
    raw_ptr->Write();
    raw_ptr = hEnergyLeakage.get();
    raw_ptr->SetDirectory(0);
    raw_ptr->Write();
    raw_ptr = hTimePerEvent_ms.get();
    raw_ptr->SetDirectory(0);
    raw_ptr->Write();

    this->counterAll.WriteHistogram(ofile);
    this->counterElectrons.WriteHistogram(ofile);
    this->counterGammas.WriteHistogram(ofile);

    ofile->Close();


    this->fProfileZHistograms.SaveRootfile(_ofilename);
    this->fProfileXYHistograms.SaveRootfile(_ofilename);
    this->fProfileTimeEnergyHistograms.SaveRootfile(_ofilename);
}


