#ifndef SEQUENCER_GENERATOR_H
#define SEQUENCER_GENERATOR_H

#include <string>
#include "dll.h"
#include "forward.h"

namespace sequencer_n
{
    sequence_t SEQUENCER_CPP_API generate(const std::vector<std::string> &description, const generatorContext_t &context);
    
    inline sequence_t generate(const std::string &description, const generatorContext_t &context) {
        return generate({ description }, context);
    }
}

#endif //SEQUENCER_GENERATOR_H
