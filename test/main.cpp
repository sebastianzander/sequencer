#include <string>
#include <vector>
#include <sequencer/sequencer.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace sequencer_n
{
    namespace test_n
    {
        using namespace std;
        using ::testing::Pointwise;
        using ::testing::DoubleNear;

        TEST(SequenceGenerator, EmptyRuleString)
        {
            const auto actual = generate("", { 5 });
            ASSERT_EQ(actual.empty(), true);
        }

        TEST(SequenceGenerator, EmptyRuleList)
        {
            const auto actual = generate(vector<string> {}, { 5 });
            ASSERT_EQ(actual.empty(), true);
        }

        TEST(SequenceGenerator, ZeroSequenceLength)
        {
            const auto actual = generate("2*n", { 0 });
            ASSERT_EQ(actual.empty(), true);
        }

        TEST(SequenceGenerator, ArithmeticSequenceExplicit)
        {
            const auto expected = sequence_t { 2, 5, 8, 11, 14 };
            const auto actual = generate("3*n + 2", { 5 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceGenerator, GeometricSequenceExplicit)
        {
            const auto expected = sequence_t { 32, 64, 128, 256, 512 };
            const auto actual = generate("2^n", { 5, 5 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceGenerator, GeometricSequenceConditionalExplicit)
        {
            const auto expected = sequence_t { 1, -5, 25, -125, 625 };
            const auto actual = generate("5^n * (mod(n, 2) == 0 ? 1 : -1)", { 5 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceGenerator, GeometricSequenceCleverExplicit)
        {
            const auto expected = sequence_t { 1, -5, 25, -125, 625 };
            const auto actual = generate("5^n * cos(_pi * n)", { 5 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceGenerator, FibonacciSequence)
        {
            const auto expected = sequence_t { 0, 1, 1, 2, 3, 5, 8, 13 };
            const auto actual = generate({ "s(0) = 0", "s(1) = 1", "s(n) = s(n-1) + s(n-2)" }, { 8 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceGenerator, GenericRuleSequence)
        {
            const auto expected = sequence_t { 1, 6, 3, 8, 5, 10, 7, 12 };
            const auto actual = generate({ "s(0) = 1", "s(2*n + 1) = s(n-1) + 5", "s(2*n) = s(n-1) - 3" }, { 8 });
            ASSERT_THAT(actual, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, EmptySequence)
        {
            const auto expected = sequence_t {};
            const auto actual = solve({}, {});
            ASSERT_EQ(actual.predictions.empty(), true);
        }

        TEST(SequenceSolver, SingleTermSequence)
        {
            const auto expected = sequence_t { 42, 42, 42 };
            const auto actual = solve({ 42 }, { 3 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, ArithmeticSequence)
        {
            const auto expected = sequence_t { 7, 9 };
            const auto actual = solve({ 1, 3, 5 }, { 2 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, GeometricSequence)
        {
            const auto expected = sequence_t { 625, -3125 };
            const auto actual = solve({ 1, -5, 25, -125 }, { 2 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, FibonacciSequence)
        {
            const auto expected = sequence_t { 21, 34, 55 };
            const auto actual = solve({ 0, 1, 1, 2, 3, 5, 8, 13 }, { 3 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, LinearRecursiveSequenceOrder3)
        {
            const auto expected = sequence_t { 20, 9, -53, -40 };
            const auto actual = solve({ 0, 1, 3, 0, -7, -1 }, { 4 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }

        TEST(SequenceSolver, ConditionalSequence)
        {
            const auto expected = sequence_t { 21, 23, 25, 29 };
            const auto actual = solve({ 1, 3, 5, 9, 11, 13, 15, 19 }, { 4 });
            ASSERT_NE(actual.predictions.empty(), true);
            ASSERT_THAT(actual.predictions[0].predictedContinuation, Pointwise(DoubleNear(1.0e-5), expected));
        }
    }
}
