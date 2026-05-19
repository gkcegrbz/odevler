#pragma once
#include "HashAlgorithm.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class DictionaryAttackSimulator
 * @brief Educational dictionary attack demonstration.
 *
 * Uses a small built-in wordlist to show why common passwords are easily cracked.
 * Demonstrates COMPOSITION with HashAlgorithm.
 */
class DictionaryAttackSimulator {
public:
    struct LookupResult {
        bool found;
        std::string matchedWord;
        size_t wordsChecked;
        double elapsedSeconds;
        size_t dictionarySize;
    };

    explicit DictionaryAttackSimulator(std::shared_ptr<HashAlgorithm> algorithm);

    LookupResult attack(const std::string& targetHash) const;

    // Check if a plaintext password is in the dictionary (no hashing)
    bool isCommonPassword(const std::string& password) const;

    size_t dictionarySize() const;
    const std::vector<std::string>& dictionary() const;

    // Allow injecting a custom (small) dictionary for testing
    void setCustomDictionary(const std::vector<std::string>& words);

private:
    std::shared_ptr<HashAlgorithm> m_algorithm;
    std::vector<std::string> m_wordlist;

    static std::vector<std::string> buildDefaultWordlist();
};
