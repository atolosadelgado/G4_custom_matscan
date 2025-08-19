#ifndef AVERAGEDHISTOGRAM
#define AVERAGEDHISTOGRAM

#include "TH1D.h"
#include "TFile.h"

// #include <tools/histo/h1d>

#include <memory>
#include <vector>
#include <iostream>

#include <cstring> // for strcmp

class AveragedHistogram {
public:
    AveragedHistogram(const std::string& name, int nbins, double xmin, double xmax)
        : name(name), nbins(nbins), count(0) {
        mean.assign(nbins + 1, 0.0);
        M2.assign(nbins + 1, 0.0);
        hist = std::make_unique<TH1D>(name.c_str(), "", nbins, xmin, xmax);
    }

    void update(const TH1D & h) {
        count++;
        // std::cout << "\t ++ " << hist->GetName() << std::endl;

        for (int i = 1; i <= nbins; ++i) {
//             double a_entries, a_Sw, a_Sw2, a_Sxw, a_Sx2w;
//             h.get_bin_content(i, a_entries, a_Sw, a_Sw2, a_Sxw, a_Sx2w);
            double x = h.GetBinContent(i);
            double delta = x - mean[i];
            mean[i] += delta / count;
            double delta2 = x - mean[i];
            M2[i] += delta * delta2;
            // if (strcmp(hist->GetName(), "E_vs_z_averaged_Silicon") == 0)
                // std::cout << '\t' << h.GetBinContent(i) << std::endl;
        }

    }

    void finalize(TFile* file) {
        // ROOT takes over ownership when doing SetDirectory(file)
        // release the smart pointer here to avoid problems
        TH1D* raw_ptr = hist.release();

        for (int i = 1; i <= nbins; ++i) {
            double variance = (count > 1) ? (M2[i] / count) : 0.0;
            double mean_error = (count > 1) ? (std::sqrt(variance) / std::sqrt(count-1)) : 0.0;
            raw_ptr->SetBinContent(i, mean[i]);
            raw_ptr->SetBinError(i, mean_error);
        }
        raw_ptr->SetDirectory(file);
        raw_ptr->Write();
    }

    void reset() {
        for (int i = 1; i <= nbins; ++i) {
            mean[i]=0;
            M2[i]=0;
        }
        count=0;
    }

private:
    std::string name;
    int nbins;
    int count;
    std::vector<double> mean;
    std::vector<double> M2;
    std::unique_ptr<TH1D> hist;
};


#endif
