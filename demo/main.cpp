#include <iostream>
#include <sstream>
#define CXXOPTS_VECTOR_DELIMITER ';'
#include <cxxopts.hpp>
#include <sequencer/sequencer.h>
#include <sequencer/utils.h>

#include "serializer.h"
#include "types.h"

using namespace std;
using namespace sequencer_n;

void printSolution(const solution_t &solution, const sequence_t &sequence, const sequence_t &continuation);
std::ostream& operator<<(std::ostream &os, const sequence_t &sequence);

int main(int argc, char* argv[])
{
    generatorContext_t generatorContext;
    solverContext_t solverContext;
    cxxopts::Options options("demo", "Short program that demonstrates the sequencer library");

    options.add_options()
        ("h,help", "Prints usage information")
        ("r,rules", "Generates a sequence from the given list of semicolon separated sequence rules", cxxopts::value<vector<string>>())
        ("s,sequence", "Solves the sequence specified by the list of semicolon separated integer or real numbers and returns the predicted continuation of the sequence", cxxopts::value<vector<double>>())
        ("m,allow-multiple-predictions", "Enables whether the sequence solver is allowed to return multiple predictions if there is ambiguity or uncertainty", cxxopts::value<bool>()->default_value("false"))
        ("c,count", "Sets the length of the sequence to be generated (if -r) or the length of the predicted continuation of the given sequence (if -s)", cxxopts::value<size_t>()->default_value("1"))
        ("start-index", "Sets the start index of the sequence to be generated (default 0)", cxxopts::value<size_t>()->default_value("0"))
        ("a,run-all-tasks", "Runs all sequence solver tasks from demo.yaml, no matter whether they are enabled or not", cxxopts::value<bool>()->default_value("false"))
        ("tasks", "Runs all sequence solver tasks from demo.yaml specified by the list of semicolon separated task names or descriptions, no matter whether the tasks are enabled or not", cxxopts::value<vector<string>>())
        ("types", "Runs all sequence solver tasks from demo.yaml that belong to any of the given semicolon separated sequence types, no matter whether the tasks are enabled or not", cxxopts::value<vector<string>>())
        ("o,max-recurrence-order", "Sets the maximum order when solving for linear recurrence", cxxopts::value<int>()->default_value("10"));

    auto parsedOptions = options.parse(argc, argv);
    if(argc == 1 || parsedOptions.count("help"))
    {
        cout << options.help() << endl;
        exit(EXIT_SUCCESS);
    }

    vector<string> rules;
    sequence_t sequence;
    vector<solverTask_t> solverTasks;
    vector<string> solverTasksToRun;
    vector<string> solverTypesToRun;

    bool allowMultiplePredictions = parsedOptions["allow-multiple-predictions"].as<bool>();
    bool runAllSolverTasks = parsedOptions["run-all-tasks"].as<bool>();
    size_t count = parsedOptions["count"].as<size_t>();
    
    generatorContext.sequenceLength = count;
    solverContext.maximumRecurrenceOrder = parsedOptions["max-recurrence-order"].as<int>();

    if(parsedOptions.count("rules"))
        rules = parsedOptions["rules"].as<vector<string>>();

    if(parsedOptions.count("sequence"))
        sequence = parsedOptions["sequence"].as<sequence_t>();

    if(parsedOptions.count("tasks"))
        solverTasksToRun = parsedOptions["tasks"].as<vector<string>>();

    if(parsedOptions.count("types"))
        solverTypesToRun = parsedOptions["types"].as<vector<string>>();

    if(parsedOptions.count("start-index"))
        generatorContext.startIndex = parsedOptions["start-index"].as<size_t>();

    try
    {
        YAML::Node demo = YAML::LoadFile("demo.yaml");

        if(demo["solverTasks"])
            solverTasks = demo["solverTasks"].as<vector<solverTask_t>>();
    }
    catch(exception &e)
    {
        cout << "\033[31mException:\033[0m " << e.what() << endl << endl;
        return 1;
    }

    cout << endl;

    if(!rules.empty())
    {
        cout << "Generating " << count << (count == 1 ? " term" : " terms") << " for sequence according to " << 
            (rules.size() == 1 ? "rule" : "rules") << " ..." << endl;

        sequence_t sequence = generate(rules, generatorContext);
        cout << "Generated sequence: \033[93m" << sequence << "\033[0m" << endl;

        cout << endl;
    }
    else if(!sequence.empty())
    {
        cout << "Solving sequence \033[93m" << sequence << "\033[0m ..." << endl;

        solverContext.allowMultiplePredictions = allowMultiplePredictions;
        solverContext.requiredPredictedContinuationCount = count;

        solution_t solution = solve(sequence, solverContext);
        printSolution(solution, sequence, {});

        cout << endl;
    }
    else
    {
        for(const auto &task : solverTasks)
        {
            if(!runAllSolverTasks)
            {
                if(solverTasksToRun.empty() && solverTypesToRun.empty() && !task.enabled)
                    continue;

                bool taskNotInListOfTasksToRun = 
                    std::find(solverTasksToRun.begin(), solverTasksToRun.end(), task.name) == solverTasksToRun.end() && 
                    std::find(solverTasksToRun.begin(), solverTasksToRun.end(), task.description) == solverTasksToRun.end();
                
                bool taskNotInListOfTypesToRun = 
                    std::find(solverTypesToRun.begin(), solverTypesToRun.end(), task.type) == solverTypesToRun.end();
                
                if(taskNotInListOfTasksToRun && taskNotInListOfTypesToRun)
                    continue;
            }

            cout << "Solving sequence \033[93m" << task.testSequence << "\033[0m ..." << endl <<
                "  Name: \033[90m" << task.name << "\033[0m" << endl <<
                "  Description: \033[90m" << task.description << "\033[0m" << endl;

            solverContext.allowMultiplePredictions = task.allowMultiplePredictions;
            solverContext.requiredPredictedContinuationCount = task.requiredPredictedContinuationCount;
            
            solution_t solution = solve(task.testSequence, solverContext);
            printSolution(solution, task.testSequence, task.continuation);

            cout << endl;
        }
    }
    
    return 0;
}

void printSolution(const solution_t &solution, const sequence_t &sequence, const sequence_t &continuation)
{
    const auto predictionsCount = solution.predictions.size();
            
    if(predictionsCount > 0)
    {
        if(predictionsCount > 1)
            cout << "Found \033[96m" << predictionsCount << "\033[0m predictions" << endl;

        bool anyPredictionValid = false;
        sequence_t combined;

        for(const auto &prediction : solution.predictions)
        {
            if(prediction.predictedContinuation.size() > 0)
            {
                cout << "Prediction: \033[94m" << prediction.predictedContinuation << "\033[0m";

                bool predictionApproximatelyEqual = false;
                if(!continuation.empty())
                {
                    const double epsilon = calculateSuitableEpsilon(continuation);
                    predictionApproximatelyEqual = approximatelyEqualLeastCommon(continuation, prediction.predictedContinuation, epsilon);
                    anyPredictionValid |= predictionApproximatelyEqual;

                    cout << (predictionApproximatelyEqual ? " \033[32mvalidated\033[0m" : " \033[31minvalidated\033[0m") << endl;
                }
                else
                    cout << endl;

                for(auto const &description : prediction.descriptionList)
                    cout << "  " << description << endl;

                if(predictionApproximatelyEqual || continuation.empty())
                {
                    combined = sequence;
                    combined.insert(combined.end(), prediction.predictedContinuation.begin(), prediction.predictedContinuation.end());
                }
            }
        }

        if(anyPredictionValid || continuation.empty())
        {
            cout << "Continued sequence: \033[90m" << combined << "\033[0m" << endl;
        }
        else
        {
            cout << "Continuation should be: \033[94m" << continuation << "\033[0m" << endl;
        }
    }
    else
        cout << "\033[94mCould not predict continuation of the given sequence\033[0m" << endl;
}

std::ostream& operator<<(std::ostream &os, const sequence_t &sequence)
{
    if(sequence.size() == 0) os << "{ }";
    else
    {
        os << "{ ";
        for(const auto &term : sequence)
        {
            if(&term != &sequence[0])
                os << ", ";
            os << term;
        }
        os << " }";
    }
    return os;
}
