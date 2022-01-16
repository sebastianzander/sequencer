#ifndef SEQUENCER_UTILS_H
#define SEQUENCER_UTILS_H

#include <cmath>
#include "dll.h"
#include "forward.h"

namespace sequencer_n
{
    inline double calculateSuitableEpsilon(const double a) {
        return std::pow(10, std::round(std::log10(std::abs(a))) - 5.0);
    }

    inline double calculateSuitableEpsilon(const sequence_t &sequence) {
        return sequence.empty() ? 0.0 : calculateSuitableEpsilon(sequence.back());
    }

    inline bool approximatelyEqual(double a, double b, const double epsilon) {
        return std::fabs(a - b) <= ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
    }

    bool SEQUENCER_CPP_API approximatelyEqual(const sequence_t &a, const sequence_t &b, const double epsilon);
    bool SEQUENCER_CPP_API approximatelyEqualLeastCommon(const sequence_t &standard, const sequence_t &suitor, const double epsilon);

    inline bool virtuallyInteger(double a) {
        const double epsilon = calculateSuitableEpsilon(a);
        return approximatelyEqual(a, std::round(a), epsilon);
    }

    inline bool virtuallyInteger(double a, const double epsilon) {
        return approximatelyEqual(a, std::round(a), epsilon);
    }
}

#endif //SEQUENCER_UTILS_H
