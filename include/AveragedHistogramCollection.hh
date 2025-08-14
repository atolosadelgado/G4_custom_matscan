#ifndef AVERAGEDHISTOGRAMCOLLECTION
#define AVERAGEDHISTOGRAMCOLLECTION

#include <map>
#include "AveragedHistogram.hh"
class G4Material;

class AveragedHistogramCollection
{
public:

    std::map<G4Material*, AveragedHistogram> averagedProfilePerMaterial_map;

    // define dummy values, to be initialized by dedicated method
    int nbins = {-1};
    double zmin = {-1.0};
    double zmax = {-1.0};

    // initialize internal histograms of energy profile
    void Initialize(std::string histo_basename, int nbins_input, double zmin_input, double zmax_input);

    // updates internal histograms of energy profile after the event
    void UpdateAverageAndMean(const tools::histo::h1d& h, G4Material * mat);

    // save histograms to a rootfile
    void SaveRootfile(std::string ofilename);

    // keep histograms, but reset the content
    void ResetHistograms();

    // remove histograms from internal map
    void ClearMapOfHistograms();

};

#endif
