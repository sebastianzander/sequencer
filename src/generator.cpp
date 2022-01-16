#include <algorithm>
#include <functional>
#include <regex>
#include <map>
#include <muParser.h>
#include "sequencer/generator.h"
#include "sequencer/types.h"

namespace sequencer_n
{
    class map_c
    {
    public:
        static inline double s(const double index)
        {
            const int _index = (int)round(index);
            auto find = constants.find(_index);
            return find != constants.end() ? find->second : DBL_MAX;
        }

        static std::map<size_t, double> constants;
    };

    std::map<size_t, double> map_c::constants;

    sequence_t generate(const std::vector<std::string> &description, const generatorContext_t &context)
    {
        using namespace std;

        regex rulePattern(R"(^s\(([^\)]+)\)\s*=\s*(.+)$)", regex_constants::optimize);
        regex explicitRulePattern(R"(^(s\(n\)\s*=\s*)?((?!s\().+)$)", regex_constants::optimize);
        regex indexPattern(R"([\s0]*(\d+))", regex_constants::optimize);
        regex decimalPattern(R"([\s0]*(\d+))", regex_constants::optimize);

        sequence_t sequence {};
        //map<size_t, double> constants;
        map_c constants;
        size_t numElements = 0;

        if(description.empty() || context.sequenceLength == 0)
            return sequence;

        /* auto s = [&constants, &sequence](const double i) {
            const int index = (int)round(i);
            auto find = constants.find(index);
            return find != constants.end() ? find->second : sequence[index];
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
            bodyStripped.erase(remove_if(bodyStripped.begin(), bodyStripped.end(), [](unsigned char x){return std::isspace(x);}), bodyStripped.end());

            parser.SetExpr(bodyStripped.c_str());

            const size_t endIndex = context.startIndex + context.sequenceLength;
            for(size_t index = context.startIndex; index < endIndex; index++)
            {
                n = static_cast<double>(index);
                double term = parser.Eval();
                sequence.push_back(term);
            }
        }
        else
        {
            for(const auto &line : description)
            {
                if(regex_search(line, matches, rulePattern))
                {
                    string param = matches[1].str();
                    string paramStripped = param;
                    paramStripped.erase(remove_if(paramStripped.begin(), paramStripped.end(), [](unsigned char x){return std::isspace(x);}), paramStripped.end());

                    string body = matches[2].str();
                    string bodyStripped = body;
                    bodyStripped.erase(remove_if(bodyStripped.begin(), bodyStripped.end(), [](unsigned char x){return std::isspace(x);}), bodyStripped.end());

                    smatch indexMatch;
                    if(regex_search(paramStripped, indexMatch, indexPattern))
                    {
                        size_t index = std::stoi(indexMatch[1].str());
                        n = numElements;
                        parser.SetExpr(bodyStripped.c_str());
                        double result = parser.Eval();
                        constants.constants.insert({ index, result });
                        numElements++;
                    }
                }
            }

            for(const auto &[index, term] : constants.constants)
                sequence.push_back(term);
        }
        
        return sequence;
    }
}
