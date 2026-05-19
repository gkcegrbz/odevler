#include "../include/BruteForceSimulator.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <mutex>

BruteForceSimulator::BruteForceSimulator(std::shared_ptr<HashAlgorithm> algorithm)
    : m_algorithm(std::move(algorithm))
{
    if (!m_algorithm)
        throw std::invalid_argument("HashAlgorithm cannot be null.");
}

std::string BruteForceSimulator::getCharset(AttackProfile profile) const {
    switch (profile) {
        case AttackProfile::NumericOnly:
            return "0123456789";
        case AttackProfile::LowercaseOnly:
            return "abcdefghijklmnopqrstuvwxyz";
        case AttackProfile::LowercaseAndDigits:
            return "abcdefghijklmnopqrstuvwxyz0123456789";
        default:
            return "0123456789";
    }
}

BruteForceSimulator::SimulationResult BruteForceSimulator::simulate(
    const std::string& targetHash,
    AttackProfile profile,
    int maxLength,
    ProgressCallback callback)
{
    if (targetHash.empty())
        throw std::invalid_argument("Target hash cannot be empty.");
    if (maxLength < 1 || maxLength > MAX_PASSWORD_LENGTH)
        throw std::invalid_argument("Max length must be 1-" +
            std::to_string(MAX_PASSWORD_LENGTH) + " (educational limit).");

    std::string charset = getCharset(profile);
    long long totalCombinations = calculateCombinations(charset.size(), maxLength);

    SimulationResult result{};
    result.totalCombinations = totalCombinations;
    result.maxLengthTested   = maxLength;
    result.charsetUsed       = charset;
    result.algorithmUsed     = m_algorithm->algorithmName();
    result.found             = false;

    std::atomic<bool> foundFlag{false};
    std::mutex resMutex;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Iterative generation (avoids deep recursion)
    // For each candidate length 1..maxLength
    long long attempts = 0;
    bool done = false;

    for (int len = 1; len <= maxLength && !done && !foundFlag; ++len) {
        // Generate all strings of `len` characters from charset
        std::vector<int> indices(len, 0);
        while (true) {
            // Build candidate string
            std::string candidate(len, ' ');
            for (int i = 0; i < len; ++i)
                candidate[i] = charset[indices[i]];

            ++attempts;

            // Check against target hash
            if (m_algorithm->hash(candidate) == targetHash) {
                foundFlag = true;
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - startTime).count();
                std::lock_guard<std::mutex> lock(resMutex);
                result.found          = true;
                result.foundPassword  = candidate;
                result.totalAttempts  = attempts;
                result.elapsedSeconds = elapsed;
                result.attemptsPerSecond = (elapsed > 0) ? attempts / elapsed : 0;
                done = true;
                break;
            }

            if (attempts >= MAX_ATTEMPTS) {
                done = true;
                break;
            }

            // Progress callback every 100k attempts
            if (callback && attempts % 100000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - startTime).count();
                callback(attempts, elapsed);
            }

            // Increment indices (odometer-style)
            int pos = len - 1;
            while (pos >= 0) {
                if (++indices[pos] < (int)charset.size()) break;
                indices[pos] = 0;
                --pos;
            }
            if (pos < 0) break; // Exhausted this length
        }
    }

    if (!result.found) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - startTime).count();
        result.totalAttempts     = attempts;
        result.elapsedSeconds    = elapsed;
        result.attemptsPerSecond = (elapsed > 0) ? attempts / elapsed : 0;
    }

    // Estimate full time if not found
    if (!result.found && result.attemptsPerSecond > 0) {
        double remaining = (totalCombinations - attempts) / result.attemptsPerSecond;
        if (remaining < 60)
            result.estimatedFullTime = std::to_string((int)remaining) + "s";
        else if (remaining < 3600)
            result.estimatedFullTime = std::to_string((int)(remaining/60)) + " min";
        else
            result.estimatedFullTime = std::to_string((int)(remaining/3600)) + " hrs";
    } else {
        result.estimatedFullTime = "N/A";
    }

    return result;
}

long long BruteForceSimulator::calculateCombinations(size_t charsetSize, int maxLength) {
    long long total = 0;
    long long power = 1;
    for (int i = 1; i <= maxLength; ++i) {
        power *= (long long)charsetSize;
        total += power;
    }
    return total;
}

std::string BruteForceSimulator::profileToString(AttackProfile p) {
    switch (p) {
        case AttackProfile::NumericOnly:        return "Numeric Only (0-9)";
        case AttackProfile::LowercaseOnly:      return "Lowercase Only (a-z)";
        case AttackProfile::LowercaseAndDigits: return "Lowercase + Digits (a-z, 0-9)";
        default: return "Unknown";
    }
}

std::string BruteForceSimulator::formatNumber(long long n) {
    std::string s = std::to_string(n);
    int insertPos = (int)s.size() - 3;
    while (insertPos > 0) {
        s.insert(insertPos, ",");
        insertPos -= 3;
    }
    return s;
}
