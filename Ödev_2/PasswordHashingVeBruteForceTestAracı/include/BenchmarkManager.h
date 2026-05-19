#pragma once
#include "HashAlgorithm.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

/**
 * @class BenchmarkManager
 * @brief Compares hashing algorithm performance.
 *
 * Demonstrates COMPOSITION: holds a collection of HashAlgorithm instances
 * and iterates polymorphically.
 */
class BenchmarkManager {
public:
    struct AlgorithmStats {
        std::string algorithmName;
        size_t outputBits;
        int iterations;
        double totalMs;
        double avgMs;
        double hashesPerSecond;
        double verifyAvgMs;
    };

    struct BenchmarkReport {
        std::vector<AlgorithmStats> results;
        std::string timestamp;
        int iterationsRun;
        std::string fastestAlgorithm;
        std::string recommendation;
    };

    BenchmarkManager();

    void addAlgorithm(std::shared_ptr<HashAlgorithm> algo);
    BenchmarkReport runBenchmark(int iterations = 1000);

    // Display a formatted table
    static void printReport(const BenchmarkReport& report);
    static std::string reportToCSV(const BenchmarkReport& report);
    static std::string reportToText(const BenchmarkReport& report);

private:
    std::vector<std::shared_ptr<HashAlgorithm>> m_algorithms;

    AlgorithmStats benchmarkSingle(
        std::shared_ptr<HashAlgorithm> algo,
        int iterations,
        const std::string& testInput
    ) const;
};
