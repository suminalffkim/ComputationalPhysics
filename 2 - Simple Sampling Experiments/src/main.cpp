#include <cmath>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>
#include <string>
#include <execution>
#include "histogram.h"

/**
 * The Mersenne Twister 19937 uniform random number generator with random seed.
 */
static thread_local std::mt19937 generator {std::random_device()()};

/**
 * A uniform distribution [0, 1] from which the generate can sample.
 */
static std::uniform_int_distribution uniform_distribution {0, 1};

/**
 * The iterator on the integer interval [0, 32) used for the number of coinflips
 */
static std::ranges::iota_view flips {0, 32};

/**
 * Copies the given span of double precision numbers into the output
 * CSV file with the given name.
 *
 * @param histogram The histogram object
 * @param name The name of the output file.
 */
void write_output(const histogram::Histogram & histogram, const std::string & name) {
    std::ofstream output;
    output.open("output/" + name + ".csv");

    output << histogram << std::endl;
    output.close();
}

/**
 * Generates a range of size_t indices on the interval [0, S). Used to replace
 * for loops with std::for_each with parallel execution policy.
 *
 * @tparam S The size of the sequence
 * @return The range [0, S) of the sequence
 */
template <std::size_t S>
std::vector<std::size_t> sequence() {
    std::vector<std::size_t> sequence (S);
    std::iota(sequence.begin(), sequence.end(), 0);
    return sequence;
}

/**
 * Generates a uniform random real number on the interval [0, 1) by flipping a coin 32 times.
 * The results of this flip are accumulated according to $real = \sum^{32}_{j=1}{\frac{f_j}{2^{j}}}$.
 *
 * @return A uniform random real number on the interval [0, 1)
 */
double uniform_real()
{
    return std::accumulate(flips.begin(), flips.end(), 0.0, [&] (const double acc, const int j) {
        return acc + uniform_distribution(generator) / pow(2, j + 1);
    });
}

/**
 * Generates a sequence of uniform reals in parallel and writes the result to a CSV file.
 *
 * @tparam S The size of the sequence of uniform reals.
 */
template <std::size_t S>
void sequence_of_uniform_reals()
{
    std::cout << "\t S is " << S << std::endl;
    histogram::Histogram histogram { 100 };

    static std::vector<std::size_t> numbers = sequence<S>();
    std::for_each(std::execution::par_unseq, numbers.begin(), numbers.end(), [&] ([[maybe_unused]] const std::size_t _) {
        histogram.add(uniform_real());
    });

    write_output(histogram, "sequence" + std::to_string(S));
}

/**
 * Generates a biased random real number on the interval [0, 1) by flipping a coin 32 times.
 * The results of this flip are accumulated according to $real = \sum^{32}_{j=1}{\frac{f_j}{2^{j}}}$.
 *
 * @param distributions The array of biased distributions. Has the same length as the number of coinflips used.
 * @return A biased random real number on the interval [0, 1)
 */
double biased_real(std::array<std::discrete_distribution<>, 32> & distributions)
{
    return std::accumulate(flips.begin(), flips.end(), 0.0, [&] (const double acc, const int j) {
        return acc + distributions.at(j)(generator) / pow(2, j + 1);
    });
}

/**
 * Generates a sequence of biased reals in parallel and writes the result to a CSV file. The biased distributions are
 * calculated once for evey possible value of j and then passed to the function calculating the biased real number.
 *
 * @tparam S The size of the sequence of biased reals.
 * @param lambda The lambda parameter of the bias.
 */
template <std::size_t S>
void sequence_of_biased_reals(const double lambda)
{
    std::cout << "\t Lambda is " << lambda << std::endl;
    histogram::Histogram histogram { 100 };

    std::array<std::discrete_distribution<>, 32> distributions {};
    std::transform(flips.begin(), flips.end(), distributions.begin(), [=] (const int j) {
        double p = 1.0 / (1.0 + exp(-lambda / pow(2, j + 1)));
        return std::discrete_distribution({p, 1.0 - p});
    });

    static std::vector<std::size_t> numbers = sequence<S>();
    std::for_each(std::execution::par_unseq, numbers.begin(), numbers.end(), [&] ([[maybe_unused]] const std::size_t _) {
       histogram.add(biased_real(distributions));
    });

    write_output(histogram, "sequence_biased" + std::to_string(static_cast<int>(lambda * 10)));
}

/**
 * Runs problems 2.1 and 2.2 from problem set 2.
 *
 * @return Result codes which indicates to the OS if the execution was successful.
 */
int main()
{
    std::filesystem::create_directory("output");

    std::cout << "2.1: Generating sequences of real numbers" << std::endl;
    sequence_of_uniform_reals<100>();
    sequence_of_uniform_reals<10000>();
    sequence_of_uniform_reals<1000000>();

    std::cout << "2.2: Generating biased sequences of real numbers" << std::endl;
    sequence_of_biased_reals<1000000>(0.0);
    sequence_of_biased_reals<1000000>(0.5);
    sequence_of_biased_reals<1000000>(1.0);
    sequence_of_biased_reals<1000000>(2.0);
}