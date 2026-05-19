#include "../include/HashAlgorithm.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <stdexcept>

std::string HashAlgorithm::hashWithSalt(const std::string& input,
                                         const std::string& salt) const {
    if (input.empty()) {
        throw std::invalid_argument("Password cannot be empty.");
    }
    return hash(salt + input);
}

std::string HashAlgorithm::bytesToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t b : bytes) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

std::string HashAlgorithm::generateRandomSalt(size_t length) {
    static const char charset[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string salt;
    salt.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dist(gen)];
    }
    return salt;
}

std::vector<uint8_t> HashAlgorithm::stringToBytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}
