#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * @class HashAlgorithm
 * @brief Abstract base class for all hashing algorithms.
 *
 * Demonstrates ABSTRACTION and POLYMORPHISM.
 * All concrete hashing classes must implement the pure virtual interface.
 */
class HashAlgorithm {
public:
    virtual ~HashAlgorithm() = default;

    // Pure virtual interface (abstraction)
    virtual std::string hash(const std::string& input) const = 0;
    virtual bool verify(const std::string& input, const std::string& expectedHash) const = 0;
    virtual std::string algorithmName() const = 0;
    virtual size_t outputBits() const = 0;

    // Concrete utility method (non-virtual)
    std::string hashWithSalt(const std::string& input, const std::string& salt) const;

    // Static utility
    static std::string bytesToHex(const std::vector<uint8_t>& bytes);
    static std::string generateRandomSalt(size_t length = 16);

protected:
    // Protected helper for subclasses
    static std::vector<uint8_t> stringToBytes(const std::string& s);
};
