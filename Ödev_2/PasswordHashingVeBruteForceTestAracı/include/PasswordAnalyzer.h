#pragma once
#include <string>
#include <vector>

/**
 * @class PasswordAnalyzer
 * @brief Evaluates password strength and generates security recommendations.
 *
 * Demonstrates ENCAPSULATION: all scoring logic is private; only
 * the result structures are exposed through clean public methods.
 */
class PasswordAnalyzer {
public:
    // Strength category enum
    enum class StrengthCategory {
        Weak,
        Medium,
        Strong,
        VeryStrong
    };

    // Result value object (composition pattern)
    struct AnalysisResult {
        int score;                          // 0–100
        StrengthCategory category;
        std::string categoryLabel;
        std::string estimatedCrackTime;
        std::vector<std::string> recommendations;

        // Score breakdown
        int lengthScore;
        int upperScore;
        int lowerScore;
        int digitScore;
        int symbolScore;
        int penaltyScore;

        bool hasUpper;
        bool hasLower;
        bool hasDigit;
        bool hasSymbol;
        bool hasDictWord;
        bool hasSequential;
        bool hasRepeated;
        int length;
    };

    PasswordAnalyzer();
    explicit PasswordAnalyzer(const std::vector<std::string>& customDictionary);

    AnalysisResult analyze(const std::string& password) const;

    // Static display helper
    static std::string categoryToString(StrengthCategory cat);
    static std::string scoreBar(int score, int width = 30);

private:
    std::vector<std::string> m_dictionary;

    // Scoring sub-methods (private implementation details)
    int scoreLength(const std::string& pw) const;
    int scoreCharacterVariety(const std::string& pw, bool& hasUpper,
                               bool& hasLower, bool& hasDigit, bool& hasSymbol) const;
    int penaltyRepeated(const std::string& pw, bool& hasRepeated) const;
    int penaltySequential(const std::string& pw, bool& hasSequential) const;
    int penaltyDictionary(const std::string& pw, bool& hasDictWord) const;

    std::string estimateCrackTime(int score, const std::string& pw) const;
    std::vector<std::string> generateRecommendations(const AnalysisResult& r) const;

    static std::vector<std::string> defaultDictionary();
};
