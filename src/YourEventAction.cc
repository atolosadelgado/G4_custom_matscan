#include "YourEventAction.hh"
#include "YourRunAction.hh"

#include "G4EventManager.hh"

YourEventAction::YourEventAction(YourRunAction * myRunAction, MyPrimaryGenerator * gen)
  : G4UserEventAction(),
  fRunAction(myRunAction), fPrimaryGenerator(gen) {}


YourEventAction::~YourEventAction() {}

void YourEventAction::BeginOfEventAction(const G4Event* /*anEvent*/) {

  // Initialize once, afterwards it will simply reset the histograms
  fHistogramCollectionProfileZ_map.Initialize("E_vs_z", nbins, zmin, zmax);
  fHistogramCollectionProfileXY_map.Initialize("XY_vs_z", nbins, zmin, zmax);
  // check if it is test beam, if so, change offset used in FillEnergyProfileZ method
  SetOffset();
}


void YourEventAction::EndOfEventAction(const G4Event* /*anEvent*/) {

  for( auto & it : fHistogramCollectionProfileZ_map.histogramCollection_map){
    TH1D & profile_histogram = *(it.second);
    G4Material * mat = it.first;
    fRunAction->fProfileZHistograms.UpdateAverageAndMean(profile_histogram, mat);
  }

  for( const auto & [mat, profileEnergyR2_h] : fHistogramCollectionProfileXY_map.histogramCollection_map){

    auto & profileZ_h = fHistogramCollectionProfileZ_map.histogramCollection_map.at( mat );

    std::unique_ptr sigma_r_h = this->MakeRMS_from_M2_Mean(profileEnergyR2_h, profileZ_h );

    fRunAction->fProfileXYHistograms.UpdateAverageAndMean(*sigma_r_h.get(), mat);
  }
}


void YourEventAction::FillEnergyProfileZ(G4double eDep_MeV, G4double zpos_mm, G4Material * mat)
{
  fHistogramCollectionProfileZ_map.Fill(eDep_MeV,zpos_mm - zoffset_mm,mat);
}

void YourEventAction::FillEnergyProfileXY(G4double eDep_MeV, G4ThreeVector avestep_position, G4Material* mat)
{
  G4double x_centered_mm = avestep_position.x() / CLHEP::mm - fPrimaryGenerator->x0_mm;
  G4double y_centered_mm = avestep_position.y() / CLHEP::mm - fPrimaryGenerator->y0_mm;
  G4double zpos_mm       = avestep_position.z() / CLHEP::mm;
  G4double radius_squared = x_centered_mm*x_centered_mm + y_centered_mm*y_centered_mm;
  G4double energyWeighted_r2 = eDep_MeV * radius_squared;
  fHistogramCollectionProfileXY_map.Fill( energyWeighted_r2 , zpos_mm - zoffset_mm,mat);

}

std::unique_ptr<TH1D> YourEventAction::MakeRMS_from_M2_Mean(const std::unique_ptr<TH1D>& M2, const std::unique_ptr<TH1D>& Mean)
{

    // we need to create a temporary histogram sigma_r
    // that is actually the RMS of the shower radius (as function of Z)
    // is computed sigma_r as sqrt( M2 /  Mean )
    // where M2 = sum E *r*r; Mean = <E>

    // create histogram to be returned
    std::unique_ptr<TH1D> sigma_r_h ( (TH1D*)M2.get()->Clone() );
    // if histogram is created in a TDirectory, ROOT will try to delete it as well
    // to avoid double deletion, set directory to null
    sigma_r_h->SetDirectory(0);

    // normalize histogram by the mean
    sigma_r_h->Divide(Mean.get());

    // compute sqrt
    for(int i =1; i<= sigma_r_h->GetNbinsX(); ++i)
    {
      double sigma_rr = sigma_r_h->GetBinContent(i);
      if(0 == sigma_rr)continue;
      double sigma_rr_error = sigma_r_h->GetBinError(i);
      sigma_r_h->SetBinContent(i, sqrt(sigma_rr) );
      sigma_r_h->SetBinError(i, 0.5*sigma_rr_error/sqrt(sigma_rr) );
    }
    // sigma_r_h->SetTitle("RMS (mm) as function of Z (mm)");
    // sigma_r_h->SetXTitle("Z (mm)");
    // sigma_r_h->SetYTitle("RMS (mm)");
    return sigma_r_h;
}

