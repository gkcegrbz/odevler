#include "../include/SaltDemonstration.h"
#include <stdexcept>
#include <sstream>

SaltDemonstration::SaltDemonstration(std::shared_ptr<HashAlgorithm> algorithm)
    : m_algorithm(std::move(algorithm))
{
    if (!m_algorithm)
        throw std::invalid_argument("HashAlgorithm cannot be null.");
}

SaltDemonstration::DemoResult
SaltDemonstration::runDemo(const std::vector<std::string>& passwords) const
{
    if (passwords.empty())
        throw std::invalid_argument("Password list cannot be empty.");

    DemoResult result{};
    std::vector<std::string> saltedHashes;
    bool allDifferent = true;

    for (const auto& pw : passwords) {
        SaltedEntry entry;
        entry.password     = pw;
        entry.salt         = HashAlgorithm::generateRandomSalt(16);
        entry.unsaltedHash = m_algorithm->hash(pw);
        entry.saltedHash   = m_algorithm->hashWithSalt(pw, entry.salt);
        result.entries.push_back(entry);

        // Check for hash collisions (should never happen with salt)
        for (const auto& prev : saltedHashes) {
            if (prev == entry.saltedHash) { allDifferent = false; }
        }
        saltedHashes.push_back(entry.saltedHash);
    }

    result.allHashesDifferent = allDifferent;
    result.explanation =
        "Without salt: identical passwords produce IDENTICAL hashes.\n"
        "  -> An attacker with a rainbow table can crack all at once.\n\n"
        "With salt: each password gets a unique random prefix before hashing.\n"
        "  -> Even identical passwords produce DIFFERENT hashes.\n"
        "  -> Rainbow table attacks become infeasible.\n"
        "  -> Each password must be attacked individually.";
    return result;
}

SaltDemonstration::DemoResult
SaltDemonstration::demonstrateIdenticalPasswords(const std::string& password) const
{
    if (password.empty())
        throw std::invalid_argument("Password cannot be empty.");

    DemoResult result{};

    // Same password hashed twice with different salts
    for (int i = 0; i < 2; ++i) {
        SaltedEntry entry;
        entry.password     = password;
        entry.salt         = HashAlgorithm::generateRandomSalt(16);
        entry.unsaltedHash = m_algorithm->hash(password);
        entry.saltedHash   = m_algorithm->hashWithSalt(password, entry.salt);
        result.entries.push_back(entry);
    }

    bool hashesMatch = (result.entries[0].saltedHash == result.entries[1].saltedHash);
    result.allHashesDifferent = !hashesMatch;
    result.explanation =
        "Two instances of the SAME password hashed with different salts.\n"
        "  Unsalted hashes: IDENTICAL (attacker can spot reuse)\n"
        "  Salted hashes:   DIFFERENT (each is unique)";

    return result;
}

std::string SaltDemonstration::generateStorageString(const std::string& password) const {
    if (password.empty())
        throw std::invalid_argument("Password cannot be empty.");
    std::string salt = HashAlgorithm::generateRandomSalt(16);
    std::string hash = m_algorithm->hashWithSalt(password, salt);
    // Format: algo$salt$hash  (similar to Unix crypt)
    return m_algorithm->algorithmName() + "$" + salt + "$" + hash;
}

bool SaltDemonstration::verifyStorageString(const std::string& password,
                                             const std::string& storageString) const
{
    // Parse format: algo$salt$hash
    size_t first = storageString.find('$');
    size_t second = storageString.find('$', first + 1);
    if (first == std::string::npos || second == std::string::npos)
        throw std::invalid_argument("Invalid storage string format.");

    std::string salt     = storageString.substr(first + 1, second - first - 1);
    std::string expected = storageString.substr(second + 1);
    std::string computed = m_algorithm->hashWithSalt(password, salt);
    return computed == expected;
}
