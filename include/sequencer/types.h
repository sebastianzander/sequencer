#ifndef SEQUENCER_TYPES_H
#define SEQUENCER_TYPES_H

#include <functional>
#include "forward.h"

namespace sequencer_n
{
    struct namedSequence_t
    {
        std::string name;
        std::string description;
        sequence_t sequence;
    };

    enum class sequenceType_t
    {
        UNSPECIFIED, 
        UNIDENTIFIED, 
        ARITHMETIC, 
        GEOMETRIC, 
        TRIANGULAR, 
        SQUARE, 
        CUBIC, 
        LINEAR_RECURSIVE,
        MIXED
    };

    enum class operation_t
    {
        UNSPECIFIED, 
        ADDITION, 
        MULTIPLICATION
    };

    struct pattern_t
    {
        operation_t operation;
        double operand;
        size_t gap;
        size_t offset;
        size_t recurrence = 0;

        inline bool operator==(const pattern_t &other) const {
            return operation == other.operation && operand == other.operand;
        }

        inline bool operator!=(const pattern_t &other) const {
            return !(operator==(other));
        }
    };

    struct prediction_t
    {
        sequence_t predictedContinuation;
        std::vector<std::string> descriptionList;
        sequenceType_t sequenceType = sequenceType_t::UNSPECIFIED;

        inline bool operator==(const prediction_t &other) const {
            return predictedContinuation == other.predictedContinuation && 
                descriptionList == other.descriptionList && sequenceType == other.sequenceType;
        }

        inline bool operator!=(const prediction_t &other) const {
            return !(operator==(other));
        }
    };

    struct solution_t
    {
        predictions_t predictions;

        inline bool operator==(const solution_t &other) const {
            return predictions == other.predictions;
        }

        inline bool operator!=(const solution_t &other) const {
            return !(operator==(other));
        }
    };

    struct generatorContext_t
    {
        size_t sequenceLength = 1;
        size_t startIndex = 0;
    };

    struct solverContext_t
    {
        size_t requiredPredictedContinuationCount = 1;
        bool allowMultiplePredictions = false;
        int maximumRecurrenceOrder = 5;
    };
}

#endif //SEQUENCER_TYPES_H
