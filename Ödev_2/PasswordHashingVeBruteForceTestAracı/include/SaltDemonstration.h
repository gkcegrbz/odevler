#pragma once
#include "HashAlgorithm.h"
#include <string>
#include <memory>
#include <vector>

/**
 * @class SaltDemonstration
 * @brief Demonstrates why salted hashing is essential for password storage.
 *
 * Shows side-by-side comparison of unsalted vs salted hashes,
 * and explains rainbow table resistance.
 */
class SaltDemonstration {
public:
    struct SaltedEntry {
        std::string password;
        std::string salt;
        std::string saltedHash;
        std::string unsaltedHash;
    };

    struct DemoResult {
        std::vector<SaltedEntry> entries;
        std::string explanation;
        bool allHashesDifferent;   // Should always be true when salted
    };

    explicit SaltDemonstration(std::shared_ptr<HashAlgorithm> algorithm);

    // Run a full side-by-side demonstration
    DemoResult runDemo(const std::vector<std::string>& passwords) const;

    // Show what happens with two identical passwords
    DemoResult demonstrateIdenticalPasswords(const std::string& password) const;

    // Generate and display a salted storage format
    std::string generateStorageString(const std::string& password) const;
    bool verifyStorageString(const std::string& password,
                              const std::string& storageString) const;

private:
    std::shared_ptr<HashAlgorithm> m_algorithm;
};
