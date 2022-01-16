#include <algorithm>
#include <functional>
#include <regex>
#include <map>
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
        static regex abstractIndexPattern(R"(^(?:\+?(\d+)\s*\*?\s*)?n(?:\+(\d+))?$)", regex_constants::optimize);
        static regex indexPattern(R"(^[\s0]*(\d+)$)", regex_constants::optimize);

        sequence_t sequence {};
        //map<size_t, double> constants;
        map_c constants;
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
                        n = numElements;
                        parser.SetExpr(bodyStripped.c_str());
                        double term = parser.Eval();
                        constants.constants.insert({ index, term });
                        numElements++;
                    }
                    else if(regex_search(paramStripped, indexMatch, abstractIndexPattern))
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

            if(specifiedGap != UNSPECIFIED_GAP)
            {
                for(size_t index = context.startIndex; index < endIndex; index++)
                {
                    n = static_cast<double>(index);
                    size_t offset = index % specifiedGap;
                    double term;

                    auto findConstant = constants.constants.find(index);
                    if(findConstant != constants.constants.end())
                        term = findConstant->second;
                    else
                    {
                        auto findPattern = patterns.find(offset);
                        string pattern = findPattern != patterns.end() ? findPattern->second : anyPattern;

                        parser.SetExpr(pattern);
                        term = parser.Eval();
                    }

                    constants.constants.insert({ index, term });
                    sequence.push_back(term);
                }
            }
            else
            {
                for(const auto &[index, term] : constants.constants)
                    sequence.push_back(term);
            }
        }
        
        return sequence;
    }
}
