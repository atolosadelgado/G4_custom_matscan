// for building the detector
#include "G4GDMLParser.hh"
// default physics
#include "FTFP_BERT.hh"
// run manager
#include "G4RunManagerFactory.hh"
// UI + vis
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "G4PhysicalVolumeStore.hh"

#include <G4LogicalVolumeStore.hh>

struct mat_cut_couple_t{
    // Constructor with 1 cut delegates to ctor with cut per particle
    mat_cut_couple_t(std::string imatname, std::string ilvname_pattern, double icut_mm)
        : mat_cut_couple_t(std::move(imatname), std::move(ilvname_pattern),
                           icut_mm, icut_mm, icut_mm) {}

    // Constructor with cuts per particle
    mat_cut_couple_t(std::string imatname, std::string ilvname_pattern,
                     double icutg_mm, double icute_mm, double icutp_mm)
        : matname(std::move(imatname)),
          lvname_pattern(std::move(ilvname_pattern)),
          cutg_mm(icutg_mm), cute_mm(icute_mm), cutp_mm(icutp_mm) {}
    std::string matname;
    std::string lvname_pattern;
    double cutg_mm;
    double cute_mm;
    double cutp_mm;
};
using matcut_couples_t = std::vector<mat_cut_couple_t>;
matcut_couples_t LoadMaterialCuts(const std::string & ifilename);
void define_hgcal_subregions_per_material(matcut_couples_t & m);
void define_original_hgcal_region();


//________________________________________________________________________________
#include "G4VUserDetectorConstruction.hh"
#include "G4GDMLParser.hh"
class YourDetectorConstructor : public G4VUserDetectorConstruction
{
public:
    void LoadGDML(std::string gdml_filename){
        G4GDMLParser Parser;
        Parser.Read(gdml_filename, false);
        this->worldPV = Parser.GetWorldVolume();
    };
    void SetProductionCutType(std::string _s){productioncut_type=_s;};
    virtual G4VPhysicalVolume* Construct(){
            // option for regions
            if( productioncut_type == "original_cuts")
                define_original_hgcal_region();
            else if( productioncut_type == "new_cuts"){
                auto mat_cut_map = LoadMaterialCuts("material_cut_mm.txt");
        //         define_hgcal_subregions(mat_cut_map);
                define_hgcal_subregions_per_material(mat_cut_map);
            }
            else
                std::cout << "Warning, no regions are being defined\n";
        return worldPV;}
    G4VPhysicalVolume * worldPV = {nullptr};
    std::string productioncut_type;
};
//________________________________________________________________________________

#include "SecondaryCounterActions.hh"
#include "SecondaryCounterPerMaterialActions.hh"
#include "YourActionInitialization.hh"
#include "MaterialScanActions.hh"

#include "MyPrimaryGenerator.hh"
#include "G4VUserActionInitialization.hh"
class YourActionInitializationForGunOnly : public G4VUserActionInitialization {
public:
    YourActionInitializationForGunOnly(): G4VUserActionInitialization(){ }
    ~YourActionInitializationForGunOnly() override {}
    void Build() const override { SetUserAction(new MyPrimaryGenerator()); }
};

//________________________________________________________________________________
#include <iostream>
#include <string>

std::string get_basename(const std::string& filepath) {
    // find last slash
    size_t lastSlash = filepath.find_last_of("/\\");
    std::string filename = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);

    // find last dot
    size_t lastDot = filename.find_last_of('.');
    if (lastDot == std::string::npos) {
        // no extension
        return filename;
    }

    return filename.substr(0, lastDot);
}

//________________________________________________________________________________

void help(int argc, char** argv){
    std::cout << "Usage:" << std::endl;
    std::cout << "\t" << argv[0] << " <geometry.gdml> <cut_type> <action_type> <physics option> <gun.mac>" << std::endl;
    std::cout << "\t  <cut_type>: extra regions defined inside the application. Options: original_cuts, new_cuts, no_cuts" << std::endl;
    std::cout << "\t  <action_type>: action for the Geant4 application. Options: secondaries (for just stats of secondaries), secondaries_permaterial, profile (for shower profile), matscan" << std::endl;
    std::cout << "\t  <physics option>: name of physics list to be used (e.g., FTFP_BERT_EMZ)" << std::endl;
}

#include "G4PhysListFactory.hh"

int main(int argc, char** argv)
{
    help(argc, argv);
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

    auto geometry_filename = argv[1];
    auto action_type = std::string( argv[3] );
    auto productioncut_type = std::string( argv[2] );
    auto physics_list_name = std::string( argv[4] );
    bool vis_mode = false;

    G4String g4macro_filename;

    // if g4 macro file is provided
    if( argc == 6 )
        g4macro_filename =G4String( argv[5] );
    else
        vis_mode = true;

    // create ofilename based on input parameters
    std::string ofilename = action_type;
        ofilename += "_";
        ofilename += get_basename(geometry_filename);
        ofilename += "_";
        ofilename += productioncut_type;
        ofilename += "_";
        ofilename += physics_list_name;
        ofilename += ".root";


    YourDetectorConstructor * user_detector_constructor = new YourDetectorConstructor();
    user_detector_constructor->LoadGDML(geometry_filename);
    user_detector_constructor->SetProductionCutType(productioncut_type);
    runManager->SetUserInitialization(user_detector_constructor);


    G4PhysListFactory pl_factory;
    auto physics_list = pl_factory.GetReferencePhysList( physics_list_name );
    if( ! physics_list ) throw std::runtime_error("No physics list named <"+ physics_list_name+"> found");
    runManager->SetUserInitialization(physics_list);
    if( action_type == "secondaries"){
        runManager->SetUserInitialization(
            new YourActionInitializationForSecondaries(ofilename)
        );
    }
    else if( action_type == "profile"){
        runManager->SetUserInitialization(
            new YourActionInitialization(ofilename)
        );
    }
    else if( action_type == "secondaries_permaterial" ){
        runManager->SetUserInitialization(new secondaryCounterPerMaterial_YourActionInit(ofilename));
    }
    else if( action_type == "matscan" )
    {
        runManager->SetUserInitialization(new ActionInitializationForMatScan(ofilename));
    }
    else{
        runManager->SetUserInitialization(new YourActionInitializationForGunOnly());
        std::cerr << "No actions!" << std::endl;
    }
        // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->ApplyCommand("/cuts/verbose 3");

    runManager->Initialize();

    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // if BeamOn(0) is not there, it crashes...
    runManager->BeamOn(0);

    // {
    //     G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
    //     TH1D * hNdaughers = new TH1D("hNdaughers","",1e4,0,1e4);
    //     TH1D * hNentities = new TH1D("hNentities","",1e4,0,1e4);
    //     TH1D * hNEmplacements = new TH1D("hNEmplacements","",1e4,0,1e4);
    //     for( auto & lv : *lv_store){
    //         hNdaughers->Fill( lv->GetNoDaughters() );
    //         hNentities->Fill( lv->TotalVolumeEntities() );
    //     }
    //     std::map<const G4LogicalVolume*, int> lvUseCount;
    //     G4PhysicalVolumeStore * pv_store = G4PhysicalVolumeStore::GetInstance();
    //     for (auto pv : *pv_store) {
    //         if (pv) {
    //             const G4LogicalVolume* lv = pv->GetLogicalVolume();
    //             lvUseCount[lv]++;
    //         }
    //     }
    //     for( auto [lv, n] : lvUseCount)
    //     {
    //         hNEmplacements->Fill(n);
    //         if(n>500)
    //             std::cout << "Warning, placed more than 500 times: " << lv->GetName() << std::endl;
    //     };
    //
    //     TFile * ofile = new TFile("hgcal_daughtervolumes.root","recreate");
    //     hNdaughers->Write();
    //     hNentities->Write();
    //     hNEmplacements->Write();
    //     ofile->Close();
    // }




    if(vis_mode)
    {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    }
    else
    {
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + g4macro_filename);
    }


    //________________________________________________________________________________

    delete visManager;
    delete runManager;

    return 0;
}


#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include "G4RegionStore.hh"

#include <sstream>
std::vector<std::string> tokenizeByTab(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '\t')) {
        tokens.push_back(token);
    }
    return tokens;
}
#include <fstream>
matcut_couples_t LoadMaterialCuts(const std::string& matcut_filename)
{
    matcut_couples_t material_cut_mm_map;
    std::ifstream ifile(matcut_filename);
    if (!ifile) {
        std::cerr << "Error opening file: " << matcut_filename << std::endl;
        return material_cut_mm_map;
    }

    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty()) continue;
        // ignore lines starting by #
        if ( '#' == line.at(0) ) continue;

        auto tokens = tokenizeByTab(line);

        if (tokens.size() != 3 && tokens.size() != 5) {
            std::cerr << "Warning: ignoring line, " << line << std::endl;
            continue;
        }

        try {

            std::string matname = tokens[0];
            std::string lvname_pattern = tokens[1];
            if(tokens.size() == 3){
                double cut_mm = std::stod(tokens[2]); // string to double
                material_cut_mm_map.push_back({matname, lvname_pattern, cut_mm});
            }
            else if(tokens.size() == 5){
                double cutg_mm = std::stod(tokens[2]); // string to double
                double cute_mm = std::stod(tokens[3]); // string to double
                double cutp_mm = std::stod(tokens[4]); // string to double
                material_cut_mm_map.push_back({matname, lvname_pattern, cutg_mm,cute_mm,cutp_mm});
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing line: " << line << " (" << e.what() << ")" << std::endl;
        }
    }

    return material_cut_mm_map;
}


bool isExactName(const std::string& pattern) {
    const std::string regex_metachars = ".^$*+?()[]{}|\\";
    for (char c : pattern) {
        if (regex_metachars.find(c) != std::string::npos)
            return false; // it has this metacharacter
    }
    return true; // no regex_metachars, exact name
}

void define_material_region(mat_cut_couple_t & couple)
{
    std::string imatname = couple.matname;
    std::string lvname_pattern_str = couple.lvname_pattern;
    auto HGCalEEmatRegion = new G4Region("Region" + lvname_pattern_str + imatname);
    // assign cuts
    auto HGCalEEmatcuts = new G4ProductionCuts();
    // Set cut values (in mm)
    HGCalEEmatcuts->SetProductionCut(couple.cutg_mm * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
    HGCalEEmatcuts->SetProductionCut(couple.cute_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
    HGCalEEmatcuts->SetProductionCut(couple.cute_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
    HGCalEEmatcuts->SetProductionCut(couple.cutp_mm * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
    HGCalEEmatRegion->SetProductionCuts(HGCalEEmatcuts);
    // ----------------------------------------------------------
    // if lvname_pattern_str is an exact name (no regex_metachars), ignore material and define region
    if( isExactName(lvname_pattern_str) )
    {
        G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
        auto HGCal_lv = lv_store->GetVolume(lvname_pattern_str);
        HGCalEEmatRegion->AddRootLogicalVolume(HGCal_lv);
        return;
    }

    // assign root volumes according to G4Material
    const G4Material * mat_ptr = G4Material::GetMaterial(imatname);
    if(! mat_ptr)
        throw std::runtime_error("Input material <" + imatname + "> does not exist");

    // lambda to check if name of LV starts by "HGCal"
    // and therefore the volume belong to it
    std::regex lvname_pattern(lvname_pattern_str);
    auto MatchLVregex = [&](G4LogicalVolume * lv)-> bool {
        // return true;
        return std::regex_match( lv->GetName(), lvname_pattern);
    };

    G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
    for (const auto& lv : *lv_store)
    {
        if( (lv->GetMaterial() == mat_ptr) && MatchLVregex(lv) )
            HGCalEEmatRegion->AddRootLogicalVolume(lv);
    }
    return;
}

void define_hgcal_subregions_per_material(matcut_couples_t & matcut_couples)
{
    for( auto & couple : matcut_couples)
         define_material_region(couple);
}


void define_original_hgcal_region()
{

    double cut_in_mm = 0.03;
    auto HGCalRegion = new G4Region("HGCalRegion");
    // assign cuts
    auto HGCalcuts = new G4ProductionCuts();
    // Set cut values (in mm)
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("gamma"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e-"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("e+"));
    HGCalcuts->SetProductionCut(cut_in_mm * CLHEP::mm, G4ProductionCuts::GetIndex("proton"));
    HGCalRegion->SetProductionCuts(HGCalcuts);
    // ----------------------------------------------------------
    // assign root volumes
    G4LogicalVolumeStore * lv_store = G4LogicalVolumeStore::GetInstance();
    auto HGCal_lv = lv_store->GetVolume("HGCal");
    HGCalRegion->AddRootLogicalVolume(HGCal_lv);

}

