#include <algorithm>
#include <functional>
#include <regex>
#include <map>
#include <set>
#include <muParser.h>
#include "sequencer/generator.h"
#include "sequencer/types.h"

namespace sequencer_n
{
    constexpr size_t UNSPECIFIED_GAP = static_cast<size_t>(-1);
    constexpr size_t ANY_OFFSET = static_cast<size_t>(-1);

    class map_c
    {
    public:
        static inline double s(const double index)
        {
            const int _index = static_cast<int>(round(index));
            auto find = constants.find(_index);
            return find != constants.end() ? find->second : DBL_MAX;
        }

        static std::map<size_t, double> constants;
    };

    std::map<size_t, double> map_c::constants;

    sequence_t generate(const std::vector<std::string> &description, const generatorContext_t &context)
    {
        using namespace std;

        static regex rulePattern(R"(^\s*s\(([^\)]+)\)\s*=\s*(.+)$)", regex_constants::optimize);
        static regex explicitRulePattern(R"(^\s*(s\(n\)\s*=\s*)?((?!s\().+)$)", regex_constants::optimize);
        static regex genericIndexPattern(R"(^(?:\+?(\d+)\s*\*?\s*)?n(?:\+(\d+))?$)", regex_constants::optimize);
        static regex indexPattern(R"(^[\s0]*(\d+)$)", regex_constants::optimize);

        sequence_t sequence {};
        //map<size_t, double> constants;
        map_c termMap;
        auto &constants = termMap.constants;
        size_t numElements = 0;
        const size_t endIndex = context.startIndex + context.sequenceLength;

        if(description.empty() || context.sequenceLength == 0)
            return sequence;

        auto isWhiteSpace = [](unsigned char x) {
            return std::isspace(x);
        };

        /* auto s = [&constants](const double index) {
            const int _index = static_cast<int>(round(index));
            auto find = constants.find(_index);
            return find != constants.end() ? find->second : DBL_MAX;
        }; */

        auto mod = [](double number, double divisor) -> double {
            return fmod(number, divisor);
        };

        mu::Parser parser;

        double n = 0;
        parser.DefineVar("n", &n);
        parser.DefineFun("s", map_c::s);
        parser.DefineFun("mod", mod);

        smatch matches;
        const string &firstLine = description[0];

        if(description.size() == 1 && regex_search(firstLine, matches, explicitRulePattern))
        {
            string body = matches[2].str();
            string bodyStripped = body;
            bodyStripped.erase(remove_if(bodyStripped.begin(), bodyStripped.end(), isWhiteSpace), bodyStripped.end());

            parser.SetExpr(bodyStripped.c_str());

            for(size_t index = context.startIndex; index < endIndex; index++)
            {
                n = static_cast<double>(index);
                double term = parser.Eval();
                sequence.push_back(term);
            }
        }
        else
        {
            map<size_t, string> patterns;
            string anyPattern = "";
            size_t specifiedGap = UNSPECIFIED_GAP;

            for(const auto &line : description)
            {
                if(regex_search(line, matches, rulePattern))
                {
                    string param = matches[1].str();
                    string paramStripped = param;
                    paramStripped.erase(remove_if(paramStripped.begin(), paramStripped.end(), isWhiteSpace), paramStripped.end());

                    string body = matches[2].str();
                    string bodyStripped = body;
                    bodyStripped.erase(remove_if(bodyStripped.begin(), bodyStripped.end(), isWhiteSpace), bodyStripped.end());

                    smatch indexMatch;
                    if(regex_search(paramStripped, indexMatch, indexPattern))
                    {
                        size_t index = std::stoi(indexMatch[1].str());
                        auto find = constants.find(index);
                        if(find != constants.end())
                        {
                            cerr << "\033[31mError: term at index " << index << " already defined "
                                "earlier and cannot be overwritten\033[0m" << endl;
                            return {};
                        }

                        n = numElements;
                        parser.SetExpr(bodyStripped.c_str());
                        double term = parser.Eval();

                        constants.insert({ index, term });
                        numElements++;
                    }
                    else if(regex_search(paramStripped, indexMatch, genericIndexPattern))
                    {
                        size_t gap = indexMatch[1].matched ? std::stoi(indexMatch[1].str()) : 1;
                        size_t offset = indexMatch[2].matched ? std::stoi(indexMatch[2].str()) : ANY_OFFSET;

                        if(specifiedGap != UNSPECIFIED_GAP && specifiedGap != gap)
                        {
                            cerr << "\033[31mError: pattern gap size previously set to " << specifiedGap << 
                                " and cannot be variable within a sequence/set of rules\033[0m" << endl;
                            return {};
                        }

                        if(offset == ANY_OFFSET)
                            anyPattern = bodyStripped;
                        else
                        {
                            auto find = patterns.find(offset);
                            if(find != patterns.end())
                            {
                                cerr << "\033[31mError: pattern offset " << offset << " already defined "
                                    "earlier and cannot be overwritten\033[0m" << endl;
                                return {};
                            }

                            patterns.insert({ offset, bodyStripped });
                        }
                        
                        specifiedGap = gap;
                    }
                }
            }

            int32_t minOffset = numeric_limits<int32_t>::max();
            int32_t maxOffset = numeric_limits<int32_t>::min();
            int32_t offsetSign = 0;
            set<int32_t> indexOffsets;

            // check if a given pattern is valid with respect to the constants it references
            auto checkPattern = [&minOffset, &maxOffset, &offsetSign, &indexOffsets](const string &pattern)
            {
                if(pattern.empty())
                    return false;
                
                static regex sArgExprPattern(R"((?:[^\w]|^)s\(([^\)]*)\))", regex_constants::optimize);
                
                mu::Parser sArgExprParser;
                double n = 0.0;
                sArgExprParser.DefineVar("n", &n);

                for(sregex_iterator i = sregex_iterator(pattern.begin(), pattern.end(), sArgExprPattern); 
                    i != sregex_iterator(); i++ )
                {
                    smatch matches = *i;
                    if(!matches[1].matched)
                        continue;

                    string sArgExpr = matches[1].str();

                    // if the s function argument depends on n
                    if(sArgExpr.find("n") != string::npos)
                    {
                        sArgExprParser.SetExpr(sArgExpr);

                        // evaluate the offset to n
                        const int32_t indexOffset = static_cast<int32_t>(sArgExprParser.Eval());
                        const int32_t sign = indexOffset < 0 ? -1 : 1;

                        if(indexOffset < minOffset)
                            minOffset = indexOffset;
                        if(indexOffset > maxOffset)
                            maxOffset = indexOffset;

                        if(offsetSign == 0)
                            offsetSign = sign;
                        else if(offsetSign != sign)
                        {
                            cerr << "\033[31mError: cannot generate sequence: set of rules not allowed to "
                                "depend on preceding and subsequent terms at the same time!\033[0m" << endl;
                            return false;
                        }

                        indexOffsets.insert(indexOffset);
                    }
                }

                return true;
            };

            if(!checkPattern(anyPattern))
                return {};
            
            for(const auto &[index, pattern] : patterns)
                if(!checkPattern(pattern))
                    return {};

            const auto numIndexOffsets = indexOffsets.size();
            const auto numConstants = constants.size();

            if(numIndexOffsets > numConstants)
            {
                cerr << "\033[31mError: cannot generate sequence: rules give " << numConstants << 
                    " constant " << (numConstants == 1 ? "term" : "terms") << " but generic rules "
                    "reference " << numIndexOffsets << "\033[0m" << endl;
                return {};
            }

            // sequence rules are generic (i.e. some contain an s(..) that depends on n)
            if(specifiedGap != UNSPECIFIED_GAP)
            {
                int32_t startIndex = context.startIndex;

                if(!constants.empty())
                {
                    const auto firstIndex = constants.begin()->first;
                    const auto lastIndex = constants.rbegin()->first;
                    const bool consecutiveIndexes = 1 + lastIndex - firstIndex == constants.size();

                    if(consecutiveIndexes)
                        startIndex = lastIndex + 1;
                }
                else if(!indexOffsets.empty())
                {
                    const auto minOffset = *indexOffsets.begin();
                    startIndex = context.startIndex + minOffset;
                }

                for(size_t index = startIndex; index < endIndex; index++)
                {
                    n = static_cast<double>(index);
                    size_t offset = index % specifiedGap;
                    double term;

                    auto findConstant = constants.find(index);
                    if(findConstant != constants.end())
                        term = findConstant->second;
                    else
                    {
                        auto findPattern = patterns.find(offset);
                        string pattern = findPattern != patterns.end() ? findPattern->second : anyPattern;

                        parser.SetExpr(pattern);
                        term = parser.Eval();
                    }

                    constants.insert({ index, term });
                    
                    if(index >= context.startIndex)
                        sequence.push_back(term);
                }
            }

            // sequence rules consist of constants only
            else if(!constants.empty())
            {
                const auto firstIndex = constants.begin()->first;
                const auto lastIndex = constants.rbegin()->first;
                const bool indexesWithinRange = context.startIndex >= firstIndex && endIndex - 1 <= lastIndex;

                if(!indexesWithinRange)
                {
                    cerr << "\033[31mError: cannot generate sequence: rules give only constant terms "
                        "but they do not span the requested index range [" << context.startIndex << 
                        "; " << endIndex << ")\033[0m" << endl;
                    return {};
                }

                const bool consecutiveIndexes = 1 + lastIndex - firstIndex == constants.size();
                bool consecutiveIndexesWithinRange = true;

                if(!consecutiveIndexes)
                {
                    auto it = constants.begin();
                    for(; it != constants.end() && it->first != context.startIndex; it++);
                    
                    for(size_t index = context.startIndex; consecutiveIndexesWithinRange && index < endIndex && 
                        it != constants.end(); index++, it++)
                        consecutiveIndexesWithinRange &= index == it->first;
                }

                if(!consecutiveIndexesWithinRange)
                {
                    cerr << "\033[31mError: cannot generate sequence: rules give only constant terms "
                        "within the requested index range but the indexes are not consecutive\033[0m" << endl;
                    return {};
                }

                for(const auto [_4, term] : constants)
                    sequence.push_back(term);
            }

            else
            {
                cerr << "\033[31mError: cannot generate sequence: rules do not provide any constant or "
                    "recurrence pattern\033[0m" << endl;
                return {};
            }
        }
        
        return sequence;
    }
}
