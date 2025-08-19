#ifndef HISTOGRAMCOLLECTION
#define HISTOGRAMCOLLECTION

class G4Material;

// #include <tools/histo/h1d>
#include "TH1D.h"
#include <memory>
#include <map>

class HistogramCollection{

public:
    std::map<G4Material*, std::unique_ptr<TH1D>> histogramCollection_map;

    // define dummy values, to be initialized by dedicated method
    int nbins = {-1};
    double zmin = {-1.0};
    double zmax = {-1.0};

    // initialize histograms. If they already exist, just reset them
    void Initialize(std::string histo_basename, int nbins_input, double zmin_input, double zmax_input);

    // fill histogram corresponding to material "mat" (if null, nothing is done)
    void Fill(double energy_in_MeV, double z_in_mm, G4Material * mat);

    // reset histograms without deleting
    void Reset();



};
#endif
