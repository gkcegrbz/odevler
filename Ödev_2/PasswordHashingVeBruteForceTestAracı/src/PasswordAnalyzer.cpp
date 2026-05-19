#include "../include/PasswordAnalyzer.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cmath>
#include <stdexcept>

PasswordAnalyzer::PasswordAnalyzer()
    : m_dictionary(defaultDictionary()) {}

PasswordAnalyzer::PasswordAnalyzer(const std::vector<std::string>& customDictionary)
    : m_dictionary(customDictionary) {}

PasswordAnalyzer::AnalysisResult PasswordAnalyzer::analyze(const std::string& password) const {
    if (password.empty())
        throw std::invalid_argument("Password cannot be empty.");

    AnalysisResult r{};
    r.length = (int)password.size();

    // Score components
    r.lengthScore   = scoreLength(password);
    r.upperScore    = 0; r.lowerScore = 0; r.digitScore = 0; r.symbolScore = 0;
    r.upperScore    = scoreCharacterVariety(password, r.hasUpper, r.hasLower,
                                             r.hasDigit, r.hasSymbol);

    // Penalty components
    int repPenalty = penaltyRepeated(password, r.hasRepeated);
    int seqPenalty = penaltySequential(password, r.hasSequential);
    int dictPenalty= penaltyDictionary(password, r.hasDictWord);
    r.penaltyScore  = repPenalty + seqPenalty + dictPenalty;

    // Character variety breakdown (re-score individually)
    int variety = 0;
    if (r.hasUpper)   variety += 10;
    if (r.hasLower)   variety += 10;
    if (r.hasDigit)   variety += 10;
    if (r.hasSymbol)  variety += 15;
    r.upperScore  = r.hasUpper  ? 10 : 0;
    r.lowerScore  = r.hasLower  ? 10 : 0;
    r.digitScore  = r.hasDigit  ? 10 : 0;
    r.symbolScore = r.hasSymbol ? 15 : 0;

    r.score = std::max(0, std::min(100,
        r.lengthScore + variety - r.penaltyScore));

    // Category
    if      (r.score < 25)  r.category = StrengthCategory::Weak;
    else if (r.score < 50)  r.category = StrengthCategory::Medium;
    else if (r.score < 75)  r.category = StrengthCategory::Strong;
    else                    r.category = StrengthCategory::VeryStrong;
    r.categoryLabel = categoryToString(r.category);

    r.estimatedCrackTime = estimateCrackTime(r.score, password);
    r.recommendations    = generateRecommendations(r);
    return r;
}

int PasswordAnalyzer::scoreLength(const std::string& pw) const {
    int len = (int)pw.size();
    if (len < 6)  return 5;
    if (len < 8)  return 15;
    if (len < 10) return 25;
    if (len < 12) return 35;
    if (len < 16) return 45;
    return 55;
}

int PasswordAnalyzer::scoreCharacterVariety(const std::string& pw,
    bool& hasUpper, bool& hasLower, bool& hasDigit, bool& hasSymbol) const
{
    hasUpper = hasLower = hasDigit = hasSymbol = false;
    for (char c : pw) {
        if (std::isupper((unsigned char)c)) hasUpper = true;
        if (std::islower((unsigned char)c)) hasLower = true;
        if (std::isdigit((unsigned char)c)) hasDigit = true;
        if (std::ispunct((unsigned char)c)) hasSymbol = true;
    }
    int score = 0;
    if (hasUpper)  score += 10;
    if (hasLower)  score += 10;
    if (hasDigit)  score += 10;
    if (hasSymbol) score += 15;
    return score;
}

int PasswordAnalyzer::penaltyRepeated(const std::string& pw, bool& hasRepeated) const {
    hasRepeated = false;
    for (size_t i = 2; i < pw.size(); ++i) {
        if (pw[i] == pw[i-1] && pw[i] == pw[i-2]) {
            hasRepeated = true;
            return 15;
        }
    }
    return 0;
}

int PasswordAnalyzer::penaltySequential(const std::string& pw, bool& hasSeq) const {
    hasSeq = false;
    std::string lower = pw;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (size_t i = 2; i < lower.size(); ++i) {
        if ((lower[i] - lower[i-1] == 1) && (lower[i-1] - lower[i-2] == 1)) {
            hasSeq = true;
            return 15;
        }
        if ((lower[i-2] - lower[i-1] == 1) && (lower[i-1] - lower[i] == 1)) {
            hasSeq = true;
            return 15;
        }
    }
    return 0;
}

int PasswordAnalyzer::penaltyDictionary(const std::string& pw, bool& hasDictWord) const {
    hasDictWord = false;
    std::string lower = pw;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (const auto& word : m_dictionary) {
        if (lower.find(word) != std::string::npos) {
            hasDictWord = true;
            return 20;
        }
    }
    return 0;
}

std::string PasswordAnalyzer::estimateCrackTime(int score, const std::string& pw) const {
    // Rough charset size estimation
    size_t charsetSize = 0;
    bool u=false,l=false,d=false,s=false;
    for (char c : pw) {
        if (std::isupper((unsigned char)c)) u=true;
        if (std::islower((unsigned char)c)) l=true;
        if (std::isdigit((unsigned char)c)) d=true;
        if (std::ispunct((unsigned char)c)) s=true;
    }
    if (u) charsetSize += 26;
    if (l) charsetSize += 26;
    if (d) charsetSize += 10;
    if (s) charsetSize += 32;
    if (charsetSize == 0) charsetSize = 26;

    // Combinations = charsetSize^length
    double combinations = std::pow((double)charsetSize, (double)pw.size());
    // Assume 1 billion hashes/sec (modern GPU)
    double seconds = combinations / 1e9;

    if (seconds < 1)         return "Instantly";
    if (seconds < 60)        return "< 1 minute";
    if (seconds < 3600)      return std::to_string((int)(seconds/60)) + " minutes";
    if (seconds < 86400)     return std::to_string((int)(seconds/3600)) + " hours";
    if (seconds < 2592000)   return std::to_string((int)(seconds/86400)) + " days";
    if (seconds < 31536000)  return std::to_string((int)(seconds/2592000)) + " months";
    double years = seconds / 31536000.0;
    if (years < 1e6)         return std::to_string((long long)years) + " years";
    return "Millions of years";
}

std::vector<std::string> PasswordAnalyzer::generateRecommendations(
    const AnalysisResult& r) const
{
    std::vector<std::string> recs;
    if (r.length < 12)
        recs.push_back("Increase length to at least 12 characters.");
    if (!r.hasUpper)
        recs.push_back("Add uppercase letters (A-Z).");
    if (!r.hasLower)
        recs.push_back("Add lowercase letters (a-z).");
    if (!r.hasDigit)
        recs.push_back("Include numbers (0-9).");
    if (!r.hasSymbol)
        recs.push_back("Add special characters (e.g. !@#$%^&*).");
    if (r.hasRepeated)
        recs.push_back("Avoid repeated characters (e.g. 'aaa').");
    if (r.hasSequential)
        recs.push_back("Avoid sequential patterns (e.g. 'abc', '123').");
    if (r.hasDictWord)
        recs.push_back("Avoid common dictionary words.");
    if (recs.empty())
        recs.push_back("Great password! Consider using a password manager.");
    return recs;
}

std::string PasswordAnalyzer::categoryToString(StrengthCategory cat) {
    switch (cat) {
        case StrengthCategory::Weak:      return "Weak";
        case StrengthCategory::Medium:    return "Medium";
        case StrengthCategory::Strong:    return "Strong";
        case StrengthCategory::VeryStrong:return "Very Strong";
        default: return "Unknown";
    }
}

std::string PasswordAnalyzer::scoreBar(int score, int width) {
    int filled = (int)((score / 100.0) * width);
    std::string bar = "[";
    for (int i = 0; i < width; ++i)
        bar += (i < filled) ? '#' : '-';
    bar += "] " + std::to_string(score) + "/100";
    return bar;
}

std::vector<std::string> PasswordAnalyzer::defaultDictionary() {
    return {
        "password","123456","qwerty","admin","letmein","welcome","monkey",
        "dragon","master","iloveyou","sunshine","princess","football","shadow",
        "superman","michael","login","hello","trustno1","pass","abc123",
        "111111","654321","123123","passwd","1234","12345678","1234567890",
        "baseball","soccer","batman","ashley","bailey","access","mustang",
        "computer","charlie","donald","harley","jordan","ranger","hunter",
        "thomas","robert","daniel","andrew","jessica","george","samsung"
    };
}
