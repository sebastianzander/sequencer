#ifndef SEQUENCER_SOLVER_H
#define SEQUENCER_SOLVER_H

#include <functional>
#include "forward.h"

namespace sequencer_n
{
    solution_t solve(const sequence_t &sequence, const solverContext_t &context);
}

#endif //SEQUENCER_SOLVER_H
