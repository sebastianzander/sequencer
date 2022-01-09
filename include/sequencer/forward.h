#ifndef SEQUENCER_FORWARD_H
#define SEQUENCER_FORWARD_H

#include <vector>

namespace sequencer_n
{
    enum class sequenceType_t;
    enum class operation_t;

    struct namedSequence_t;
    struct pattern_t;
    struct prediction_t;
    struct solution_t;
    struct solverContext_t;

    using sequence_t = std::vector<double>;
    using predictions_t = std::vector<prediction_t>;
}

#endif //SEQUENCER_FORWARD_H
