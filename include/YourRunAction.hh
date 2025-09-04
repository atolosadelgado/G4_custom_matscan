#ifndef YOURRUNACTION_HH
#define YOURRUNACTION_HH

#include "G4UserRunAction.hh"

#include "AveragedHistogramCollection.hh"

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
  std::string _ofilename;

};

#endif
