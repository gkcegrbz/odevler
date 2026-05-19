#pragma once
#include "HashAlgorithm.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>

/**
 * @class BruteForceSimulator
 * @brief Educational, intentionally-limited brute force simulation.
 *
 * IMPORTANT: This is strictly educational. Hard limits prevent real-world abuse:
 *   - Max password length: 5 characters
 *   - Small, configurable charsets only
 *   - Attempts capped at 10,000,000
 *
 * Demonstrates COMPOSITION: holds a shared_ptr<HashAlgorithm> and
 * uses it polymorphically. Also uses std::atomic for thread safety.
 */
class BruteForceSimulator {
public:
    // Attack profile enum
    enum class AttackProfile {
        NumericOnly,        // 0-9 (10 chars)
        LowercaseOnly,      // a-z (26 chars)
        LowercaseAndDigits  // a-z + 0-9 (36 chars)
    };

    // Simulation result value object
    struct SimulationResult {
        bool found;
        std::string foundPassword;
        long long totalAttempts;
        long long totalCombinations;
        double elapsedSeconds;
        double attemptsPerSecond;
        std::string estimatedFullTime;
        int maxLengthTested;
        std::string charsetUsed;
        std::string algorithmUsed;
    };

    // Callback for progress updates (optional)
    using ProgressCallback = std::function<void(long long attempts, double elapsed)>;

    // Hard limits (intentional)
    static constexpr int MAX_PASSWORD_LENGTH = 5;
    static constexpr long long MAX_ATTEMPTS = 10'000'000LL;

    explicit BruteForceSimulator(std::shared_ptr<HashAlgorithm> algorithm);

    SimulationResult simulate(
        const std::string& targetHash,
        AttackProfile profile,
        int maxLength = 4,
        ProgressCallback callback = nullptr
    );

    // Complexity calculator (standalone, no side effects)
    static long long calculateCombinations(size_t charsetSize, int maxLength);
    static std::string profileToString(AttackProfile p);
    static std::string formatNumber(long long n);

private:
    std::shared_ptr<HashAlgorithm> m_algorithm;

    std::string getCharset(AttackProfile profile) const;
    void generateCandidates(
        const std::string& charset,
        const std::string& targetHash,
        int maxLen,
        SimulationResult& result,
        std::atomic<bool>& found,
        std::mutex& resultMutex,
        ProgressCallback callback
    ) const;
};
