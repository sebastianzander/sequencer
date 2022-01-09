#include <sstream>

#include "sequencer/types.h"
#include "sequencer/utils.h"

namespace sequencer_n
{
    bool approximatelyEqual(const sequence_t &a, const sequence_t &b, const double epsilon)
    {
        if(a.size() != b.size())
            return false;
        
        bool equal = true;
        for(size_t i = 0; i < a.size() && equal; i++)
            equal &= approximatelyEqual(a[i], b[i], epsilon);
            
        return equal;
    }

    bool approximatelyEqualLeastCommon(const sequence_t &standard, const sequence_t &suitor, const double epsilon)
    {
        if(standard.size() < suitor.size())
            return false;
        
        bool equal = true;
        for(size_t i = 0; i < suitor.size() && equal; i++)
            equal &= approximatelyEqual(standard[i], suitor[i], epsilon);
            
        return equal;
    }
}
