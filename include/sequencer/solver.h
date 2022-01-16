#ifndef SEQUENCER_SOLVER_H
#define SEQUENCER_SOLVER_H

#include <functional>
#include "dll.h"
#include "forward.h"

namespace sequencer_n
{
    solution_t SEQUENCER_CPP_API solve(const sequence_t &sequence, const solverContext_t &context);
}

#endif //SEQUENCER_SOLVER_H
