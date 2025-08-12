
#include "YourRunAction.hh"

// #include "YourRun.hh"
#include "G4Run.hh"

// #include "Randomize.hh"

YourRunAction::YourRunAction()
:   G4UserRunAction()
    // fYourDetector(det),
    // fYourPrimary(prim),
    // fYourRun(nullptr),
    // fIsEdepHistogramUICmdInvoked(false)
    {
  /* histo manager could be created here */
  // Create our own UI messenger object that will interact to this Run-Action
  // to set some properties that will be used to update YourRun object (generated
  // by calling this Run-Action::GenerateRun method) in the BeginOfRunAction method.
  // fMessenger = new YourRunActionMessenger(this);
}

YourRunAction::~YourRunAction() {
  /* histo manager must be deleted here then*/
  // delete all dynamically allocated objects here
  // delete fMessenger;
}


// G4Run* YourRunAction::GenerateRun() {
//   fYourRun = new YourRun(fYourDetector, fYourPrimary);
//   return fYourRun;
// }


void YourRunAction::BeginOfRunAction(const G4Run* /*run*/) {
//     // Show Rndm status (only for the Master thread)
// //    if ( IsMaster() ) {
// //      G4Random::showEngineStatus();
// //    }
//     //
//     // Make sure that the Gun position is correct: the user can change the target
//     // thickness between construction of objects and start of the run.
//     // note: primary generator is set in the CTR only for the Worker threads in the
//     //       ActionInitialization (left null for Master in the BuildForMaster())
//     if ( fYourPrimary ) {
//         fYourPrimary->UpdatePosition();
//     }
//     // Update the properties of the Energy-deposit histogram member of YourRun,
//     // that is already available at this point: Only if the user invoked the UI
//     // command /yourApp/runAction/edepHistoto set properties of the Edep-histo.
//     if ( fIsEdepHistogramUICmdInvoked ) {
//       // user defined the properties of the Edep-histo by invoking the UI command
//       fYourRun->SetEdepHisto(fEdepHistFileName, fEdepHistMinEnergy,
//                              fEdepHistMaxEnergy, fEdepHistNumBins);
//     }
//     //
//     // G4AnalysisManager* analysisManager OpenFile
}

#include <cmath>  // For std::sqrt
#include "TH1D.h"
#include "TFile.h"

// void YourRunAction::EndOfRunAction(const G4Run*) {
//
//   const tools::histo::h1d& hist = *(runEnergyProfileZ_vector.back());
//   G4int nbins = hist.GetNbins();
//   G4int n_histograms = runEnergyProfileZ_vector.size();
//   double n_histograms_sqrt = std::sqrt(n_histograms);
//
//   TH1D * eDepProfile_averaged = new TH1D("eDepProfile_averaged","", nbins, 3000, 6000);
//
//   for (int i = 1; i <= nbins; ++i) {   // assuming bins start at 1
//       double mean_value = 0.0;
//
//       // 1. Calculate mean
//       for (const auto& h : runEnergyProfileZ_vector) {
//           mean_value += h->GetBinContent(i);
//       }
//       mean_value /= n_histograms;
//
//       // 2. Calculate variance
//       double variance = 0.0;
//       if( 1 < n_histograms)
//       {
//         for (const auto& h : runEnergyProfileZ_vector) {
//             double diff = h->GetBinContent(i) - mean_value;
//             variance += diff * diff;
//         }
//         variance /= n_histograms;
//       }
//
//       // 3. Calculate mean error (standard error)
//       double mean_error = std::sqrt(variance) / n_histograms_sqrt;
//       eDepProfile_averaged->SetBinContent(i,mean_value);
//       eDepProfile_averaged->SetBinError(i,  mean_error);
//
//
//   }
//   TFile * ofile = TFile::Open("out.root", "update");
//   eDepProfile_averaged->SetDirectory(ofile);
//   eDepProfile_averaged->Write();
//   ofile->Close();
//
//
// }

// void YourRunAction::MoveProfileToRunAction(std::unique_ptr<tools::histo::h1d> h) {
//     runEnergyProfileZ_vector.push_back(std::move(h));
//     std::cout << "." << std::flush;
//
//     // std::cout << "\t New Histogram!\n";
//     // const tools::histo::h1d& hist = *(runEnergyProfileZ_vector.back());
//     // G4int nbins = hist.GetNbins();
//     // for(int i = 1; i<nbins; ++i){
//     //   std::cout << hist.GetBinCenter(i) << '\t' << hist.GetBinContent(i) << std::endl;
//     // }
// }

void YourRunAction::InitializeAveragedProfileHistogram(int number_of_bins)
{
  if( 0 == mean.size() || 0 == M2.size() )
  {
    count = 0;
    mean.assign(nbins + 1, 0.0); // +1 if bins start at 1
    M2.assign(nbins + 1, 0.0);
  }
}

void YourRunAction::UpdateAveragedProfileHistogram(std::unique_ptr<tools::histo::h1d> h)
{
  count++;
  for (int i = 1; i <= nbins; ++i) {
      double x = h->GetBinContent(i);
      double delta = x - mean[i];
      mean[i] += delta / count;
      double delta2 = x - mean[i];
      M2[i] += delta * delta2;
  }
}

void YourRunAction::FinalizeAveragedProfileHistogram()
{

    TH1D * eDepProfile_averaged = new TH1D("eDepProfile_averaged","", nbins, 3000, 6000);
    for (int i = 1; i <= nbins; ++i) {
        double variance = (count > 1) ? (M2[i] / count) : 0.0;
        double mean_error = (count > 1) ? (std::sqrt(variance) / std::sqrt(count)) : 0.0;

        eDepProfile_averaged->SetBinContent(i, mean[i]);
        eDepProfile_averaged->SetBinError(i, mean_error);
    }

    TFile * ofile = TFile::Open("out.root", "update");
    eDepProfile_averaged->SetDirectory(ofile);
    eDepProfile_averaged->Write();
    ofile->Close();

}

void YourRunAction::EndOfRunAction(const G4Run* )
{
  this->FinalizeAveragedProfileHistogram();
}


