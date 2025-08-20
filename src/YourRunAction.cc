
#include "YourRunAction.hh"

#include "G4EventManager.hh"
#include "YourEventAction.hh"


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

    this->fProfileHistograms.Initialize("E_vs_z_averaged", nbins, zmin, zmax);

}

void YourRunAction::EndOfRunAction(const G4Run* ){
    this->fProfileHistograms.SaveRootfile(_ofilename);

}


