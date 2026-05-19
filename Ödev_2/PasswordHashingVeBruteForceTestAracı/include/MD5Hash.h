#pragma once
#include "HashAlgorithm.h"

/**
 * @class MD5Hash
 * @brief MD5 hashing algorithm implementation.
 *
 * Inherits from HashAlgorithm. Educational note: MD5 is cryptographically broken
 * and included only to demonstrate why modern alternatives are preferred.
 */
class MD5Hash : public HashAlgorithm {
public:
    MD5Hash() = default;
    ~MD5Hash() override = default;

    std::string hash(const std::string& input) const override;
    bool verify(const std::string& input, const std::string& expectedHash) const override;
    std::string algorithmName() const override;
    size_t outputBits() const override;

private:
    // Internal MD5 state
    void processBlock(const uint8_t* block, uint32_t state[4]) const;
    static const uint32_t T[64];   // MD5 sine-derived constants
    static const uint32_t S[64];   // Shift amounts
};
