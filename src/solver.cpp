#include <iostream>
#include <numeric>
#include <set>
#include <map>
#include <sstream>
#include <fmt/core.h>
#include <Eigen/Eigen>

#include "sequencer/solver.h"
#include "sequencer/types.h"
#include "sequencer/utils.h"

namespace sequencer_n
{
    void calculatePredictions(const sequence_t &sequence, sequence_t &predictions, const size_t predictionCount, 
        std::function<double(double, size_t)> formula)
    {
        auto lastNumber = sequence.back();

        // calculate predictionCount new predicted numbers and add them to the result
        for(size_t i = 0; i < predictionCount; i++)
        {
            const auto newIndex = sequence.size() + predictions.size();   
            auto number = formula(lastNumber, newIndex);

            predictions.push_back(number);
            lastNumber = number;
        }
    }

    void calculateRecursivePredictions(const sequence_t &sequence, sequence_t &predictions, const size_t predictionCount, 
        std::function<double(const sequence_t &, size_t)> formula)
    {
        sequence_t combined = sequence;

        // calculate predictionCount new predicted numbers and add them to the result
        for(size_t i = 0; i < predictionCount; i++)
        {
            const auto newIndex = combined.size();   
            auto number = formula(combined, newIndex);

            combined.push_back(number);
            predictions.push_back(number);
        }
    }

    solution_t solve(const sequence_t &sequence, const solverContext_t &context)
    {
        solution_t result {};

        if(sequence.size() <= 1)
        {
            if(sequence.size() == 1)
            {
                prediction_t prediction;
                prediction.sequenceType = sequenceType_t::UNSPECIFIED;
                prediction.predictedContinuation.push_back(sequence[0]);
                result.predictions.emplace_back(prediction);
            }
            return result;
        }

        struct artefact_t
        {
            double difference;
            double ratio;
        };

        std::vector<artefact_t> artefacts;

        double last = sequence[0];
        double lastDifference = DBL_MAX;
        double lastRatio = DBL_MAX;

        bool constantDifference = true;
        bool constantRatio = true;
        bool containsDecimals = !virtuallyInteger(last);

        // gather differences and ratios between terms
        for(size_t i = 1; i < sequence.size(); i++)
        {
            double current = sequence[i];
            double difference = current - last;
            double ratio = current / last;

            containsDecimals |= !virtuallyInteger(current);

            artefacts.emplace_back(artefact_t { difference, ratio });

            if(i > 1)
            {
                constantDifference &= approximatelyEqual(difference, lastDifference, calculateSuitableEpsilon(difference));
                constantRatio &= approximatelyEqual(ratio, lastRatio, calculateSuitableEpsilon(ratio));
            }

            lastDifference = difference;
            lastRatio = ratio;
            last = current;
        }

        const bool ambiguous = constantDifference && constantRatio;

        if(ambiguous && context.allowMultiplePredictions || constantDifference)
        {
            prediction_t prediction {};
            prediction.sequenceType = sequenceType_t::ARITHMETIC;

            const auto constant = artefacts[0].difference;
            calculatePredictions(sequence, prediction.predictedContinuation, context.requiredPredictedContinuationCount, 
                [constant](double previousNumber, size_t newIndex) -> double {
                    return previousNumber + constant;
                });

            prediction.descriptionList.emplace_back(fmt::format("s(n) = s(n-1) {} {}", constant < 0 ? "-" : "+", std::abs(constant)));
            result.predictions.emplace_back(prediction);
            
            if(!context.allowMultiplePredictions)
                return result;
        }
        
        if(ambiguous && context.allowMultiplePredictions || constantRatio)
        {
            prediction_t prediction {};
            prediction.sequenceType = sequenceType_t::GEOMETRIC;

            const auto constant = artefacts[0].ratio;
            calculatePredictions(sequence, prediction.predictedContinuation, context.requiredPredictedContinuationCount, 
                [constant](double previousNumber, size_t newIndex) -> double {
                    return previousNumber * constant;
                });

            prediction.descriptionList.emplace_back(fmt::format("s(n) = {}s(n-1)", constant));
            result.predictions.emplace_back(prediction);
            
            if(!context.allowMultiplePredictions)
                return result;
        }

        std::vector<pattern_t> foundPatterns;
        uint32_t foundGap = 0;

        for(uint32_t gap = artefacts.size() - 1; gap > 1; gap--)
        {
            std::vector<pattern_t> patterns;
            bool equal = true;

            for(size_t offset = gap; offset < artefacts.size() && equal; offset++)
            {
                const auto &a = artefacts[offset % gap];
                const auto &b = artefacts[offset];
                equal &= a.difference == b.difference || a.ratio == b.ratio;

                if(equal)
                {
                    pattern_t pattern;
                    pattern.gap = gap;
                    pattern.offset = offset;
                    pattern.operation = a.difference == b.difference ? operation_t::ADDITION : operation_t::MULTIPLICATION;
                    pattern.operand = a.difference == b.difference ? a.difference : a.ratio;
                    patterns.emplace_back(pattern);
                }
            }

            if(equal)
            {
                foundPatterns = patterns;
                foundGap = gap;
                break;
            }
        }

        if(!foundPatterns.empty())
        {
            const auto gap = foundGap;

            bool differencesOnly = true;
            bool ratiosOnly = true;
            bool quirkyRatios = false;

            for(const auto &pattern : foundPatterns)
            {
                differencesOnly &= pattern.operation == operation_t::ADDITION;
                ratiosOnly &= pattern.operation == operation_t::MULTIPLICATION;
                
                if(!quirkyRatios && pattern.operation == operation_t::MULTIPLICATION && 
                    !virtuallyInteger(pattern.operand, std::round(pattern.operand)))
                    quirkyRatios = true;
            }

            for(uint32_t i = 0; i < 2; i++)
            {
                prediction_t prediction {};
                prediction.sequenceType = sequenceType_t::MIXED;

                std::vector<pattern_t> patterns;

                for(uint32_t offset = 0; offset < foundGap; offset++)
                {
                    pattern_t pattern;
                    
                    if(offset < foundPatterns.size())
                        pattern = foundPatterns[offset];
                    else
                    {
                        const auto &artefact = artefacts[offset];
                        const bool differenceFirst = differencesOnly | quirkyRatios;

                        bool quirkyRatio = !virtuallyInteger(artefact.ratio);

                        pattern.operation = quirkyRatio ? operation_t::ADDITION : operation_t::MULTIPLICATION;
                        pattern.operand = quirkyRatio ? artefact.difference : artefact.ratio;
                    }

                    std::string description = fmt::format("s({}n+{}) = ", gap, (offset + 1) % gap);
                    if(pattern.operation == operation_t::MULTIPLICATION)
                        description += fmt::format("{}", pattern.operand);
                    description += "s(n-1)";
                    if(pattern.operation == operation_t::ADDITION)
                        description += fmt::format(" {} {}", pattern.operand < 0 ? "-" : "+", std::abs(pattern.operand));

                    prediction.descriptionList.emplace_back(description);
                    patterns.emplace_back(pattern);
                }

                calculatePredictions(sequence, prediction.predictedContinuation, context.requiredPredictedContinuationCount, 
                    [&patterns, gap](double previousNumber, size_t newIndex) -> double {
                        auto index = (newIndex - 1) % gap;
                        auto pattern = patterns[index];
                        if(pattern.operation == operation_t::ADDITION)
                            return previousNumber + pattern.operand;
                        else //if(pattern.operation == operation_t::MULTIPLICATION)
                            return previousNumber * pattern.operand;
                    });

                result.predictions.emplace_back(prediction);
                
                if(!context.allowMultiplePredictions)
                    return result;

                if(gap == foundPatterns.size())
                    break;
            }
        }

        // quick and dirty approach to set a reasonable epsilon value for approximate equality comparison
        const double equalityEpsilon = calculateSuitableEpsilon(sequence);

        // calculate maximum determinable linear recurrence order for the given sequence
        const int maximumOrder = static_cast<int>(sequence.size()) >> 1;

        /* check if sequence is linear recursive:
         *  sequence size needs to be at least order*2 in order to have enough rows in our sparse matrix 
         *  to find a linear combination and thus all linear recursive coefficients */
        for(int order = maximumOrder; order > 0; order--)
        {
            using namespace Eigen;
            using Triplet = Triplet<double>;

            SparseMatrix<double> matrix(order, order);
            VectorXd constants(order);
            std::vector<Triplet> tripletList;
            tripletList.reserve(order * order);

            for(int i = 0; i < order; i++)
            {
                for(int j = 0, k = i; j < order; j++, k++)
                    tripletList.push_back(Triplet(i, j, sequence[k]));
                constants[i] = sequence[i + order];
            }

            matrix.setFromTriplets(tripletList.begin(), tripletList.end());      
            BiCGSTAB<SparseMatrix<double>> solver;
            
            solver.compute(matrix);

            // decomposition failed
            if(solver.info() != Success)
                continue;

            VectorXd coefficientsVector = solver.solve(constants);

            // solving failed
            if(solver.info() != Success)
                continue;

            if(coefficientsVector.size() > 0)
            {
                prediction_t prediction {};
                prediction.sequenceType = sequenceType_t::LINEAR_RECURSIVE;

                // convert VectorXd to vector<double>
                std::vector<double> coefficients(coefficientsVector.size());
                VectorXd::Map(&coefficients[0], coefficientsVector.size()) = coefficientsVector;

                calculateRecursivePredictions(sequence, prediction.predictedContinuation, context.requiredPredictedContinuationCount, 
                    [&coefficients](const sequence_t &sequence, size_t newIndex) -> double {
                        double result = 0.0;
                        for(size_t i = 0; i < coefficients.size(); i++)
                        {
                            double coefficient = coefficients[i];
                            size_t index = newIndex - coefficients.size() + i;
                            result += coefficient * sequence[index];
                        }
                        return result;
                    });

                bool predictionAlreadyMade = false;

                for(auto it = result.predictions.begin(); it != result.predictions.end() && !predictionAlreadyMade; it++)
                    predictionAlreadyMade |= approximatelyEqual(prediction.predictedContinuation, it->predictedContinuation, equalityEpsilon);

                if(predictionAlreadyMade)
                    continue;

                for(uint32_t i = 0; i < order; i++)
                    prediction.descriptionList.emplace_back(fmt::format("s({}) = {}", i, sequence[i]));

                std::stringstream ss;
                ss << "s(n) =";
                for(size_t i = 0; i < coefficients.size(); i++)
                {
                    double coefficient = coefficients[coefficients.size() - i - 1];
                    std::string coeffStr = std::to_string(std::abs(coefficient));
                    coeffStr.erase(coeffStr.find_last_not_of('0') + 1, std::string::npos);
                    coeffStr.erase(coeffStr.find_last_not_of('.') + 1, std::string::npos);

                    ss << " ";
                    if(i == 0 && coefficient < 0) ss << "-";
                    else if(i > 0) ss << (coefficient < 0 ? "- " : "+ ");
                    if(!approximatelyEqual(std::abs(coefficient), 1.0, 0.0001))
                        ss << coeffStr;
                    ss << "s(n-" << (i + 1) << ")";
                }

                prediction.descriptionList.emplace_back(ss.str());
                result.predictions.emplace_back(prediction);
                
                if(!context.allowMultiplePredictions)
                    return result;
            }
        }

        return result;
    }
}
