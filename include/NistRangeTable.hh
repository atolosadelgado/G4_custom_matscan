
#ifndef NIST_RANGE_TABLE_HH
#define NIST_RANGE_TABLE_HH

#include <vector>
#include <string>


class NistRangeTable {
public:
    NistRangeTable(const std::string& filename, double density_g_cm3);

    double EnergyFromRange(double range_mm) const;

private:
    struct RangeEnergy {
        double energy_MeV;
        double range_mm;
    };

    std::vector<RangeEnergy> table_;
    double rangeMin_;  // mm
    double rangeMax_;  // mm
};

#endif
