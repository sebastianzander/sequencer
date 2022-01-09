#ifndef SEQUENCER_GENERATOR_H
#define SEQUENCER_GENERATOR_H

#include <string>
#include "forward.h"

namespace sequencer_n
{
    sequence_t generate(const std::string &description, const size_t length);
}

#endif //SEQUENCER_GENERATOR_H
