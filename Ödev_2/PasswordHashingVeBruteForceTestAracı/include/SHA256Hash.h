#pragma once
#include "HashAlgorithm.h"

/**
 * @class SHA256Hash
 * @brief SHA-256 hashing algorithm implementation.
 *
 * The recommended algorithm in this toolkit. SHA-256 is part of SHA-2 family
 * and remains cryptographically strong for general-purpose use.
 */
class SHA256Hash : public HashAlgorithm {
public:
    SHA256Hash() = default;
    ~SHA256Hash() override = default;

    std::string hash(const std::string& input) const override;
    bool verify(const std::string& input, const std::string& expectedHash) const override;
    std::string algorithmName() const override;
    size_t outputBits() const override;

private:
    void processBlock(const uint8_t* block, uint32_t state[8]) const;
    static uint32_t rotateRight(uint32_t value, unsigned int count);
    static const uint32_t K[64];  // SHA-256 round constants
};
