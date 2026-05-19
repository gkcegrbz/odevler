#pragma once
#include <string>
#include <vector>
#include <functional>

/**
 * @class MenuSystem
 * @brief Reusable terminal menu renderer.
 *
 * Demonstrates ENCAPSULATION: all ANSI color codes and layout
 * logic are hidden behind a clean interface.
 */
class MenuSystem {
public:
    struct MenuItem {
        int id;
        std::string label;
        std::string description;
    };

    MenuSystem();

    void printBanner() const;
    void printMenu(const std::vector<MenuItem>& items) const;
    void printSectionHeader(const std::string& title) const;
    void printResult(const std::string& label, const std::string& value) const;
    void printSuccess(const std::string& message) const;
    void printError(const std::string& message) const;
    void printWarning(const std::string& message) const;
    void printInfo(const std::string& message) const;
    void printSeparator(char c = '=', int width = 50) const;
    void printTable(const std::vector<std::string>& headers,
                    const std::vector<std::vector<std::string>>& rows) const;

    // Input helpers
    static std::string getInput(const std::string& prompt);
    static int getIntInput(const std::string& prompt, int min, int max);
    static bool getConfirmation(const std::string& prompt);

private:
    bool m_colorEnabled;

    // ANSI codes (disabled if terminal doesn't support them)
    std::string color(const std::string& code) const;

    static constexpr const char* RESET  = "\033[0m";
    static constexpr const char* BOLD   = "\033[1m";
    static constexpr const char* GREEN  = "\033[32m";
    static constexpr const char* CYAN   = "\033[36m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* RED    = "\033[31m";
    static constexpr const char* WHITE  = "\033[97m";
    static constexpr const char* DIM    = "\033[2m";
};
