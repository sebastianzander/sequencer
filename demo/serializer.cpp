#include <sequencer/types.h>
#include "serializer.h"

using namespace std;
using namespace sequencer_n;

namespace YAML
{
    Node convert<solverTask_t>::encode(const solverTask_t &rhs)
    {
        Node node;
        node.SetStyle(EmitterStyle::Flow);

        node["name"] = rhs.name;
        node["description"] = rhs.description;
        node["type"] = rhs.type;
        node["rules"] = rhs.rules;
        node["sequence"] = rhs.sequence;
        node["continuation"] = rhs.continuation;
        node["givenElementCount"] = rhs.givenElementCount;
        node["requiredPredictedContinuationCount"] = rhs.requiredPredictedContinuationCount;
        node["allowMultiplePredictions"] = rhs.allowMultiplePredictions;
        node["enabled"] = rhs.enabled;
        
        return node;
    }

    bool convert<solverTask_t>::decode(const Node& node, solverTask_t &rhs)
    {
        if(!node.IsMap())
            return false;

        rhs.name = node["name"].as<string>();
        rhs.description = node["description"].as<string>();
        rhs.type = node["type"].as<string>();
        rhs.rules = node["rules"].as<vector<string>>();
        rhs.sequence = node["sequence"].as<sequence_t>();
        rhs.givenElementCount = node["givenElementCount"].as<size_t>();
        rhs.requiredPredictedContinuationCount = node["requiredPredictedContinuationCount"].as<size_t>();
        rhs.allowMultiplePredictions = node["allowMultiplePredictions"].as<bool>();
        rhs.enabled = node["enabled"].as<bool>();

        int missingElementCount = rhs.givenElementCount + rhs.requiredPredictedContinuationCount - rhs.sequence.size();
        if(missingElementCount > 0)
        {
            stringstream ss;
            ss << "givenElementCount + requiredPredictedContinuationCount exceeds element count in sequence for sequence solver task \"" <<
                rhs.name << "\"";
            throw exception(ss.str().c_str());
        }

        rhs.testSequence.insert(rhs.testSequence.begin(), rhs.sequence.begin(), rhs.sequence.begin() + rhs.givenElementCount);
        rhs.continuation.insert(rhs.continuation.begin(), rhs.sequence.begin() + rhs.givenElementCount, rhs.sequence.end());
        
        return true;
    }
}
