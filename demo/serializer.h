#ifndef DEMO_SERIALIZER_H
#define DEMO_SERIALIZER_H

#define YAML_CPP_DLL
#include <yaml-cpp/yaml.h>
#include "types.h"

namespace YAML
{
    template<>
    struct convert<solverTask_t>
    {
        static Node encode(const solverTask_t &rhs);
        static bool decode(const Node &node, solverTask_t &rhs);
    };
}

#endif //DEMO_SERIALIZER_H
