#ifndef YOURRUNACTION_HH
#define YOURRUNACTION_HH

#include "G4UserRunAction.hh"

#include "AveragedHistogramCollection.hh"

#include "SecondaryEscapeCounter.hh"

#include <memory>

class G4Material;

class YourRunAction : public G4UserRunAction {

  public:

    YourRunAction(std::string ofilename);
    virtual ~YourRunAction();
    void   BeginOfRunAction(const G4Run* run) override;
    void   EndOfRunAction(const G4Run* run) override;

  // collection of material-histogram, where the histogram
  // is averaged over the run (average of event histograms)
  AveragedHistogramCollection fProfileZHistograms;
  AveragedHistogramCollection fProfileXYHistograms;
  AveragedHistogramCollection fProfileTimeEnergyHistograms;

  std::unique_ptr<TH1D> hSamplingFraction;
  std::unique_ptr<TH1D> hEnergyLeakage;
  std::string _ofilename;


  std::vector<G4Material*> sensitive_mats;
  std::vector<G4Material*> nonDetector_mats;

  SecondaryEscapeCounter counterAll;
  SecondaryEscapeCounter counterElectrons;
  SecondaryEscapeCounter counterGammas;

};

#endif
