#ifndef DEMO_TYPES_H
#define DEMO_TYPES_H

#include <sequencer/forward.h>

struct solverTask_t
{
    std::string name;
    std::string description;
    std::string type;
    std::vector<std::string> rules;
    sequencer_n::sequence_t sequence;
    sequencer_n::sequence_t testSequence;
    sequencer_n::sequence_t continuation;
    size_t givenElementCount;
    size_t requiredPredictedContinuationCount;
    bool allowMultiplePredictions = false;
    bool enabled = true;
};

#endif //DEMO_TYPES_H
