#include "../include/SHA1Hash.h"
#include <cstring>
#include <stdexcept>

uint32_t SHA1Hash::rotateLeft(uint32_t value, unsigned int count) {
    return (value << count) | (value >> (32 - count));
}

void SHA1Hash::processBlock(const uint8_t* block, uint32_t state[5]) const {
    uint32_t W[80];
    for (int i = 0; i < 16; ++i) {
        W[i] = ((uint32_t)block[i * 4] << 24)
              | ((uint32_t)block[i * 4 + 1] << 16)
              | ((uint32_t)block[i * 4 + 2] << 8)
              |  (uint32_t)block[i * 4 + 3];
    }
    for (int i = 16; i < 80; ++i) {
        W[i] = rotateLeft(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16], 1);
    }

    uint32_t a = state[0], b = state[1], c = state[2],
             d = state[3], e = state[4];

    for (int i = 0; i < 80; ++i) {
        uint32_t f, k;
        if (i < 20)      { f = (b & c) | (~b & d); k = 0x5A827999; }
        else if (i < 40) { f = b ^ c ^ d;           k = 0x6ED9EBA1; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else             { f = b ^ c ^ d;           k = 0xCA62C1D6; }

        uint32_t tmp = rotateLeft(a, 5) + f + e + k + W[i];
        e = d; d = c; c = rotateLeft(b, 30); b = a; a = tmp;
    }

    state[0] += a; state[1] += b; state[2] += c;
    state[3] += d; state[4] += e;
}

std::string SHA1Hash::hash(const std::string& input) const {
    if (input.empty())
        throw std::invalid_argument("Input to SHA1 cannot be empty.");

    uint32_t state[5] = {0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};

    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t origBitLen = (uint64_t)msg.size() * 8;

    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0x00);
    for (int i = 7; i >= 0; --i)
        msg.push_back((uint8_t)(origBitLen >> (8 * i)));

    for (size_t off = 0; off < msg.size(); off += 64)
        processBlock(msg.data() + off, state);

    std::vector<uint8_t> digest(20);
    for (int i = 0; i < 5; ++i) {
        digest[i*4+0] = (state[i] >> 24) & 0xFF;
        digest[i*4+1] = (state[i] >> 16) & 0xFF;
        digest[i*4+2] = (state[i] >> 8)  & 0xFF;
        digest[i*4+3] = (state[i] >> 0)  & 0xFF;
    }
    return bytesToHex(digest);
}

bool SHA1Hash::verify(const std::string& input, const std::string& expected) const {
    return hash(input) == expected;
}
std::string SHA1Hash::algorithmName() const { return "SHA1"; }
size_t SHA1Hash::outputBits() const { return 160; }
