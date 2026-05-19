#include "../include/BenchmarkManager.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <ctime>

BenchmarkManager::BenchmarkManager() {}

void BenchmarkManager::addAlgorithm(std::shared_ptr<HashAlgorithm> algo) {
    if (!algo) throw std::invalid_argument("Algorithm cannot be null.");
    m_algorithms.push_back(std::move(algo));
}

BenchmarkManager::AlgorithmStats BenchmarkManager::benchmarkSingle(
    std::shared_ptr<HashAlgorithm> algo,
    int iterations,
    const std::string& testInput) const
{
    AlgorithmStats stats{};
    stats.algorithmName = algo->algorithmName();
    stats.outputBits    = algo->outputBits();
    stats.iterations    = iterations;

    // Hash speed
    auto start = std::chrono::high_resolution_clock::now();
    std::string lastHash;
    for (int i = 0; i < iterations; ++i) {
        lastHash = algo->hash(testInput + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    stats.totalMs = std::chrono::duration<double, std::milli>(end - start).count();
    stats.avgMs   = stats.totalMs / iterations;
    stats.hashesPerSecond = (stats.totalMs > 0)
        ? (iterations / (stats.totalMs / 1000.0)) : 0;

    // Verify speed (single hash verify, averaged)
    auto vstart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        algo->verify(testInput, lastHash);
    }
    auto vend = std::chrono::high_resolution_clock::now();
    double vTotal = std::chrono::duration<double, std::milli>(vend - vstart).count();
    stats.verifyAvgMs = vTotal / iterations;

    return stats;
}

BenchmarkManager::BenchmarkReport BenchmarkManager::runBenchmark(int iterations) {
    if (m_algorithms.empty())
        throw std::runtime_error("No algorithms registered for benchmark.");

    BenchmarkReport report{};
    report.iterationsRun = iterations;

    // Timestamp
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    report.timestamp = buf;

    const std::string testInput = "BenchmarkTestPassword2024!";

    for (auto& algo : m_algorithms) {
        report.results.push_back(benchmarkSingle(algo, iterations, testInput));
    }

    // Find fastest
    auto fastest = std::max_element(report.results.begin(), report.results.end(),
        [](const AlgorithmStats& a, const AlgorithmStats& b) {
            return a.hashesPerSecond < b.hashesPerSecond;
        });
    report.fastestAlgorithm = fastest->algorithmName;

    // Recommendation
    report.recommendation =
        "For password storage: use SHA256 with bcrypt/scrypt/Argon2 in production. "
        "MD5 and SHA1 are provided for educational comparison only - they are "
        "cryptographically broken and must NOT be used for real password storage.";

    return report;
}

void BenchmarkManager::printReport(const BenchmarkReport& report) {
    std::cout << "\n";
    std::cout << "  +--------------+--------+------------+------------------+--------------+\n";
    std::cout << "  | Algorithm    | Bits   | Avg ms     | Hashes/sec       | Verify ms    |\n";
    std::cout << "  +--------------+--------+------------+------------------+--------------+\n";
    for (const auto& s : report.results) {
        std::cout << "  | " << std::left << std::setw(12) << s.algorithmName
                  << " | " << std::right << std::setw(6) << s.outputBits
                  << " | " << std::fixed << std::setprecision(4) << std::setw(10) << s.avgMs
                  << " | " << std::setw(16) << (long long)s.hashesPerSecond
                  << " | " << std::setw(12) << s.verifyAvgMs << " |\n";
    }
    std::cout << "  +--------------+--------+------------+------------------+--------------+\n";
    std::cout << "\n  Fastest: " << report.fastestAlgorithm << "\n";
    std::cout << "  Note: " << report.recommendation << "\n\n";
}

std::string BenchmarkManager::reportToCSV(const BenchmarkReport& report) {
    std::ostringstream oss;
    oss << "Algorithm,OutputBits,Iterations,TotalMs,AvgMs,HashesPerSec,VerifyAvgMs\n";
    for (const auto& s : report.results) {
        oss << s.algorithmName << ","
            << s.outputBits << ","
            << s.iterations << ","
            << std::fixed << std::setprecision(4) << s.totalMs << ","
            << s.avgMs << ","
            << (long long)s.hashesPerSecond << ","
            << s.verifyAvgMs << "\n";
    }
    return oss.str();
}

std::string BenchmarkManager::reportToText(const BenchmarkReport& report) {
    std::ostringstream oss;
    oss << "=== BENCHMARK REPORT ===\n";
    oss << "Timestamp  : " << report.timestamp << "\n";
    oss << "Iterations : " << report.iterationsRun << "\n\n";
    for (const auto& s : report.results) {
        oss << "Algorithm    : " << s.algorithmName << "\n";
        oss << "Output Bits  : " << s.outputBits << "\n";
        oss << "Avg Hash (ms): " << std::fixed << std::setprecision(6) << s.avgMs << "\n";
        oss << "Hashes/sec   : " << (long long)s.hashesPerSecond << "\n";
        oss << "Verify (ms)  : " << s.verifyAvgMs << "\n";
        oss << "---\n";
    }
    oss << "Fastest      : " << report.fastestAlgorithm << "\n";
    oss << "Recommendation: " << report.recommendation << "\n";
    return oss.str();
}
