#include "NistRangeTable.hh"
#include <fstream>
#include <stdexcept>

#include <limits>

#include <algorithm>

NistRangeTable::NistRangeTable(const std::string& filename, double density_g_cm3)
{
    std::ifstream fin(filename);
    if (!fin) {
        throw std::runtime_error("NistRangeTable error. Cannot open NIST ESTAR table : " + filename);
    }

    // ignore first 5 lines before data starts
    for (int i = 0; i < 8; ++i)
        fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


    // ---------------------------------------------------------
    // Read data: energy and mass range
    // Energy(MeV)  MassRange(g/cm2)
    // ---------------------------------------------------------
    double E, mass_range;
    while (fin >> E >> mass_range)
    {
        // Convertimos rango masico (g/cm2) → rango geometrico (cm)
        double R_cm = mass_range / density_g_cm3;

        // convertimos a mm
        double R_mm = R_cm * 10.0;

        table_.push_back({R_mm, E});
    }

    if (table_.size() < 2) {
        throw std::runtime_error("NistRangeTable error. Less than 2 data points : " + filename);
    }
    rangeMin_ = table_.front().range_mm;
    rangeMax_ = table_.back().range_mm;

}

double NistRangeTable::EnergyFromRange(double range_mm) const
{
    // if range outside of NIST data, return infinity threshold (ie, no kill)
    if (range_mm <= rangeMin_ || range_mm >= rangeMax_)
        return std::numeric_limits<double>::infinity();

    // ancillary function for the binary search
    auto comp = [](const RangeEnergy& a, double value){
        return a.range_mm < value;
    };

    // lower_bound: first element range_mm >= range_mm_input
    auto it = std::lower_bound(table_.begin(), table_.end(),
                               range_mm, comp);

    // it points to first element "curr"
    // prev = previous element
    const auto& curr = *it;
    const auto& prev = *(it - 1);

    // linear interpolation
    double slope =  (curr.energy_MeV - prev.energy_MeV) / (curr.range_mm - prev.range_mm);
    return prev.energy_MeV + slope * (range_mm - prev.range_mm) ;
}

