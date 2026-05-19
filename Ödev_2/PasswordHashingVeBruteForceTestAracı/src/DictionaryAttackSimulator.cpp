#include "../include/DictionaryAttackSimulator.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>

DictionaryAttackSimulator::DictionaryAttackSimulator(std::shared_ptr<HashAlgorithm> algorithm)
    : m_algorithm(std::move(algorithm))
    , m_wordlist(buildDefaultWordlist())
{
    if (!m_algorithm)
        throw std::invalid_argument("HashAlgorithm cannot be null.");
}

DictionaryAttackSimulator::LookupResult
DictionaryAttackSimulator::attack(const std::string& targetHash) const
{
    if (targetHash.empty())
        throw std::invalid_argument("Target hash cannot be empty.");

    LookupResult result{};
    result.dictionarySize = m_wordlist.size();
    result.found = false;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < m_wordlist.size(); ++i) {
        ++result.wordsChecked;
        if (m_algorithm->hash(m_wordlist[i]) == targetHash) {
            result.found       = true;
            result.matchedWord = m_wordlist[i];
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.elapsedSeconds = std::chrono::duration<double>(end - start).count();
    return result;
}

bool DictionaryAttackSimulator::isCommonPassword(const std::string& password) const {
    std::string lower = password;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (const auto& w : m_wordlist) {
        if (lower == w) return true;
    }
    return false;
}

size_t DictionaryAttackSimulator::dictionarySize() const {
    return m_wordlist.size();
}

const std::vector<std::string>& DictionaryAttackSimulator::dictionary() const {
    return m_wordlist;
}

void DictionaryAttackSimulator::setCustomDictionary(const std::vector<std::string>& words) {
    m_wordlist = words;
}

std::vector<std::string> DictionaryAttackSimulator::buildDefaultWordlist() {
    return {
        "password","123456","password1","abc123","qwerty","letmein",
        "monkey","1234567","dragon","master","123123","baseball",
        "iloveyou","trustno1","sunshine","princess","welcome","shadow",
        "superman","michael","football","admin","test","root","pass",
        "hello","computer","login","12345678","1234567890","passw0rd",
        "batman","access","ashley","mustang","soccer","tiger","charlie",
        "robert","thomas","andrew","george","joshua","daniel","jessica",
        "samsung","cookie","cheese","butter","flower","purple","orange",
        "donald","harley","ranger","hunter","jordan","lucky","killer",
        "hockey","yankees","cowboys","starwars","minecraft","roblox",
        "school","college","student","teacher","office","windows","linux",
        "secret","mypass","guitar","coffee","summer","winter","spring",
        "autumn","monday","friday","birthday","holiday","chicken","matrix"
    };
}
