#include "../include/MenuSystem.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

#ifdef _WIN32
  #include <windows.h>
  static bool supportsColor() {
      HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
      DWORD mode = 0;
      return GetConsoleMode(h, &mode);
  }
#else
  #include <unistd.h>
  static bool supportsColor() { return isatty(STDOUT_FILENO); }
#endif

MenuSystem::MenuSystem() : m_colorEnabled(supportsColor()) {}

std::string MenuSystem::color(const std::string& code) const {
    return m_colorEnabled ? code : "";
}

void MenuSystem::printBanner() const {
    std::cout << color(CYAN) << color(BOLD);
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════╗\n";
    std::cout << "  ║         DEFENSIVE SECURITY TOOLKIT               ║\n";
    std::cout << "  ║         Educational Cybersecurity Suite          ║\n";
    std::cout << "  ║         C++ OOP Project  |  Safe Mode            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════╝\n";
    std::cout << color(RESET) << "\n";
}

void MenuSystem::printMenu(const std::vector<MenuItem>& items) const {
    std::cout << color(BOLD) << color(WHITE);
    std::cout << "  ┌──────────────────────────────────────────────────┐\n";
    for (const auto& item : items) {
        if (item.id == 0) {
            std::cout << "  │  " << color(DIM) << color(RESET) << color(WHITE) << color(BOLD);
            std::cout << "  " << std::setw(2) << item.id << ".  " << item.label;
            std::cout << color(RESET);
        } else {
            std::cout << "  │  " << color(CYAN) << "  " << std::setw(2) << item.id << ".  "
                      << color(RESET) << color(WHITE) << item.label;
        }
        // Pad to fixed width
        int pad = 40 - (int)item.label.size();
        if (pad > 0) std::cout << std::string(pad, ' ');
        std::cout << color(RESET) << "│\n";
    }
    std::cout << color(BOLD) << color(WHITE);
    std::cout << "  └──────────────────────────────────────────────────┘\n";
    std::cout << color(RESET);
}

void MenuSystem::printSectionHeader(const std::string& title) const {
    std::cout << "\n" << color(CYAN) << color(BOLD);
    std::cout << "  ══════════════════════════════════════════\n";
    std::cout << "   " << title << "\n";
    std::cout << "  ══════════════════════════════════════════\n";
    std::cout << color(RESET) << "\n";
}

void MenuSystem::printResult(const std::string& label, const std::string& value) const {
    std::cout << "  " << color(DIM) << std::setw(22) << std::left << label
              << color(RESET) << ": "
              << color(WHITE) << value << color(RESET) << "\n";
}

void MenuSystem::printSuccess(const std::string& message) const {
    std::cout << "  " << color(GREEN) << color(BOLD) << "[✓] " << color(RESET)
              << color(GREEN) << message << color(RESET) << "\n";
}

void MenuSystem::printError(const std::string& message) const {
    std::cout << "  " << color(RED) << color(BOLD) << "[✗] " << color(RESET)
              << color(RED) << message << color(RESET) << "\n";
}

void MenuSystem::printWarning(const std::string& message) const {
    std::cout << "  " << color(YELLOW) << color(BOLD) << "[!] " << color(RESET)
              << color(YELLOW) << message << color(RESET) << "\n";
}

void MenuSystem::printInfo(const std::string& message) const {
    std::cout << "  " << color(CYAN) << "[i] " << color(RESET) << message << "\n";
}

void MenuSystem::printSeparator(char c, int width) const {
    std::cout << "  " << color(DIM) << std::string(width, c) << color(RESET) << "\n";
}

void MenuSystem::printTable(const std::vector<std::string>& headers,
                             const std::vector<std::vector<std::string>>& rows) const
{
    if (headers.empty()) return;

    // Calculate column widths
    std::vector<size_t> widths(headers.size());
    for (size_t i = 0; i < headers.size(); ++i)
        widths[i] = headers[i].size();
    for (const auto& row : rows)
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i)
            widths[i] = std::max(widths[i], row[i].size());

    auto printRow = [&](const std::vector<std::string>& cells, bool isHeader) {
        std::cout << "  |";
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string cell = (i < cells.size()) ? cells[i] : "";
            if (isHeader) std::cout << color(BOLD) << color(CYAN);
            std::cout << " " << std::left << std::setw(widths[i]) << cell << " ";
            if (isHeader) std::cout << color(RESET);
            std::cout << "|";
        }
        std::cout << "\n";
    };

    auto printDivider = [&]() {
        std::cout << "  +";
        for (size_t i = 0; i < headers.size(); ++i)
            std::cout << std::string(widths[i] + 2, '-') << "+";
        std::cout << "\n";
    };

    printDivider();
    printRow(headers, true);
    printDivider();
    for (const auto& row : rows) printRow(row, false);
    printDivider();
}

std::string MenuSystem::getInput(const std::string& prompt) {
    std::cout << "\n  " << prompt;
    std::string input;
    std::getline(std::cin, input);
    // Trim
    size_t start = input.find_first_not_of(" \t\r\n");
    size_t end   = input.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : input.substr(start, end - start + 1);
}

int MenuSystem::getIntInput(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::string raw = getInput(prompt);
        try {
            int val = std::stoi(raw);
            if (val >= minVal && val <= maxVal) return val;
            std::cout << "  Please enter a number between "
                      << minVal << " and " << maxVal << ".\n";
        } catch (...) {
            std::cout << "  Invalid input. Please enter a number.\n";
        }
    }
}

bool MenuSystem::getConfirmation(const std::string& prompt) {
    std::string input = getInput(prompt + " [y/n]: ");
    return (!input.empty() && (input[0] == 'y' || input[0] == 'Y'));
}
