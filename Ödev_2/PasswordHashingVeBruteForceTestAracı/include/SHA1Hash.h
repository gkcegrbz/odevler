#pragma once
#include "HashAlgorithm.h"

/**
 * @class SHA1Hash
 * @brief SHA-1 hashing algorithm implementation.
 *
 * Educational note: SHA-1 is deprecated for cryptographic use.
 * Included to demonstrate algorithm progression and comparative weaknesses.
 */
class SHA1Hash : public HashAlgorithm {
public:
    SHA1Hash() = default;
    ~SHA1Hash() override = default;

    std::string hash(const std::string& input) const override;
    bool verify(const std::string& input, const std::string& expectedHash) const override;
    std::string algorithmName() const override;
    size_t outputBits() const override;

private:
    void processBlock(const uint8_t* block, uint32_t state[5]) const;
    static uint32_t rotateLeft(uint32_t value, unsigned int count);
};
