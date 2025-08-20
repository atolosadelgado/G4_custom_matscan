#ifndef AVERAGEDHISTOGRAM
#define AVERAGEDHISTOGRAM

#include <memory>
#include <vector>
#include <iostream>

#include <cstring> // for strcmp

#include "TH1D.h"
#include "TFile.h"

#include <memory>
#include <vector>
#include <string>
#include <cmath>

// --------------------
// Small class for Welford's online algorithm
// --------------------
inline void testWelford(int N ) ;
class WelfordAccumulator {
public:
    WelfordAccumulator() : n(0), mean(0.0), M2(0.0) {}

    void add(double x) {
        ++n;
        double delta = x - mean;
        mean += delta / n;
        double delta2 = x - mean;
        M2 += delta * delta2;
    }

    int count() const { return n; }

    /// Get Mean
    double getMean() const { return mean; }

    double getVariance() const { return (n > 1) ? (M2 / n) : 0.0; }

    /// Get Standard Error of the Mean
    double getSEM() const { return (n > 1) ? (std::sqrt(getVariance()) / std::sqrt(n - 1)) : 0.0; }


    void reset() { n = 0; mean = 0.0; M2 = 0.0; }

    static void run_test(){ testWelford(1000); };

private:
    int n;
    double mean;
    double M2;
};

// --------------------
// Histogram wrapper using Welford accumulators per bin
// --------------------
class AveragedHistogram {
public:
    AveragedHistogram(const std::string& name, int nbins, double xmin, double xmax)
        : name(name), nbins(nbins) {
        accumulators.resize(nbins + 1); // bins are 1-indexed in ROOT
        hist = std::make_unique<TH1D>(name.c_str(), "", nbins, xmin, xmax);
    }

    void update(const TH1D& h) {
        for (int i = 1; i <= nbins; ++i) {
            accumulators[i].add(h.GetBinContent(i));
        }
    }

    void finalize(TFile* file) {
        TH1D* raw_ptr = hist.release();

        for (int i = 1; i <= nbins; ++i) {
            raw_ptr->SetBinContent(i, accumulators[i].getMean());
            raw_ptr->SetBinError(i, accumulators[i].getSEM());
        }

        raw_ptr->SetDirectory(file);
        raw_ptr->Write();
    }

    void reset() {
        for (auto& acc : accumulators) {
            acc.reset();
        }
    }

    // Retrieve mean error for a given bin
    double getMeanError(int bin) const {
        if (bin >= 1 && bin <= nbins) {
            return accumulators[bin].getSEM();
        }
        return 0.0;
    }

private:
    std::string name;
    int nbins;
    std::vector<WelfordAccumulator> accumulators;
    std::unique_ptr<TH1D> hist;
};

// --------------------
// Code for testing Welford's algorithm implementation
// --------------------

#include <random>
#include <iostream>
#include <vector>
#include <cmath>

// Two-pass algorithm for variance and mean
struct TwoPassResult {
    double mean;
    double variance;
    double stdError;
};

inline  TwoPassResult twoPass(const std::vector<double>& data) {
    int n = data.size();
    double mean = 0.0;
    for (auto x : data) mean += x;
    mean /= n;

    double var = 0.0;
    for (auto x : data) {
        double diff = x - mean;
        var += diff * diff;
    }
    var /= n;

    double stderr = (n > 1) ? (std::sqrt(var) / std::sqrt(n - 1)) : 0.0;

    return { mean, var, stderr };
}

// Test function
inline void testWelford(int N = 10000) {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    std::mt19937 gen(42); // deterministic RNG
    std::normal_distribution<double> dist(0.0, 1.0); // mean=0, sigma=1

    std::vector<double> data;
    data.reserve(N);

    WelfordAccumulator acc;
    for (int i = 0; i < N; ++i) {
        double x = dist(gen);
        data.push_back(x);
        acc.add(x);
    }

    auto twoPassRes = twoPass(data);

    // tolerances
    const double tolMean = 1e-7;
    const double tolVar  = 1e-7;
    const double tolErr  = 1e-7;

    if (std::fabs(acc.getMean() - twoPassRes.mean) > tolMean) {
        throw std::runtime_error("Mean mismatch");
    }

    if (std::fabs(acc.getVariance() - twoPassRes.variance) > tolVar) {
        throw std::runtime_error("Variance mismatch, var(Welford)=" + std::to_string(acc.getVariance()) + ", var(2pass)=" + std::to_string( twoPassRes.variance) );
    }

    if (std::fabs(acc.getSEM() - twoPassRes.stdError) > tolErr) {
        throw std::runtime_error("SEM mismatch, SEM(Welford)=" + std::to_string(acc.getSEM()) + ", SEM(2pass)=" + std::to_string( twoPassRes.stdError) );
    }

}


#endif
