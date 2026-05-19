#include "../include/ReportGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <stdexcept>

ReportGenerator::ReportGenerator(std::shared_ptr<Logger> logger,
                                  const std::string& outputDir)
    : m_logger(std::move(logger))
    , m_outputDir(outputDir)
{
    std::filesystem::create_directories(outputDir);
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    m_sessionStart = buf;
}

void ReportGenerator::addAnalysisResult(
    const std::string& password,
    const PasswordAnalyzer::AnalysisResult& result)
{
    m_analysisEntries.push_back({password, result});
    if (m_logger) m_logger->info("Analysis result added for password (hidden).");
}

void ReportGenerator::addBenchmarkReport(const BenchmarkManager::BenchmarkReport& report) {
    m_benchmarkReports.push_back(report);
    if (m_logger) m_logger->info("Benchmark report added.");
}

void ReportGenerator::addBruteForceResult(const BruteForceSimulator::SimulationResult& result) {
    m_bruteForceResults.push_back(result);
    if (m_logger) m_logger->info("Brute-force result added.");
}

void ReportGenerator::addDictionaryResult(const DictionaryAttackSimulator::LookupResult& result) {
    m_dictResults.push_back(result);
    if (m_logger) m_logger->info("Dictionary result added.");
}

bool ReportGenerator::exportTXT(const std::string& filename) const {
    std::string path = m_outputDir + (filename.empty()
        ? generateFilename("report", "txt") : filename);
    std::ofstream out(path);
    if (!out.is_open()) {
        if (m_logger) m_logger->error("Failed to open TXT file: " + path);
        return false;
    }
    out << buildTXTReport();
    out.close();
    if (m_logger) m_logger->info("TXT report exported: " + path);
    std::cout << "  [OK] Report saved to: " << path << "\n";
    return true;
}

bool ReportGenerator::exportCSV(const std::string& filename) const {
    std::string path = m_outputDir + (filename.empty()
        ? generateFilename("report", "csv") : filename);
    std::ofstream out(path);
    if (!out.is_open()) {
        if (m_logger) m_logger->error("Failed to open CSV file: " + path);
        return false;
    }
    out << buildCSVReport();
    out.close();
    if (m_logger) m_logger->info("CSV report exported: " + path);
    std::cout << "  [OK] CSV saved to: " << path << "\n";
    return true;
}

void ReportGenerator::clearSession() {
    m_analysisEntries.clear();
    m_benchmarkReports.clear();
    m_bruteForceResults.clear();
    m_dictResults.clear();
}

std::string ReportGenerator::sessionSummary() const {
    std::ostringstream oss;
    oss << "Session started : " << m_sessionStart << "\n";
    oss << "Analysis runs   : " << m_analysisEntries.size() << "\n";
    oss << "Benchmark runs  : " << m_benchmarkReports.size() << "\n";
    oss << "Brute-force runs: " << m_bruteForceResults.size() << "\n";
    oss << "Dictionary runs : " << m_dictResults.size() << "\n";
    return oss.str();
}

std::string ReportGenerator::generateFilename(const std::string& prefix,
                                               const std::string& ext) {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&t));
    return prefix + "_" + buf + "." + ext;
}

std::string ReportGenerator::buildTXTReport() const {
    std::ostringstream oss;
    oss << "============================================================\n";
    oss << "  DEFENSIVE SECURITY TOOLKIT - SESSION REPORT\n";
    oss << "============================================================\n";
    oss << "Session Start: " << m_sessionStart << "\n\n";

    if (!m_analysisEntries.empty()) {
        oss << "---- PASSWORD ANALYSIS RESULTS ----\n";
        for (size_t i = 0; i < m_analysisEntries.size(); ++i) {
            const auto& e = m_analysisEntries[i];
            oss << "\n[" << (i+1) << "] Password : " << std::string(e.password.size(), '*') << "\n";
            oss << "    Length  : " << e.result.length << "\n";
            oss << "    Score   : " << e.result.score << "/100\n";
            oss << "    Category: " << e.result.categoryLabel << "\n";
            oss << "    Crack   : " << e.result.estimatedCrackTime << "\n";
            oss << "    Flags   : ";
            if (e.result.hasUpper)      oss << "Upper ";
            if (e.result.hasLower)      oss << "Lower ";
            if (e.result.hasDigit)      oss << "Digit ";
            if (e.result.hasSymbol)     oss << "Symbol ";
            if (e.result.hasDictWord)   oss << "DictWord! ";
            if (e.result.hasSequential) oss << "Sequential! ";
            if (e.result.hasRepeated)   oss << "Repeated! ";
            oss << "\n    Recommendations:\n";
            for (const auto& r : e.result.recommendations)
                oss << "      - " << r << "\n";
        }
        oss << "\n";
    }

    if (!m_benchmarkReports.empty()) {
        oss << "---- BENCHMARK RESULTS ----\n";
        for (const auto& b : m_benchmarkReports) {
            oss << BenchmarkManager::reportToText(b) << "\n";
        }
    }

    if (!m_bruteForceResults.empty()) {
        oss << "---- BRUTE-FORCE SIMULATION RESULTS ----\n";
        for (const auto& bf : m_bruteForceResults) {
            oss << "  Algorithm   : " << bf.algorithmUsed << "\n";
            oss << "  Charset     : " << bf.charsetUsed << "\n";
            oss << "  Max Length  : " << bf.maxLengthTested << "\n";
            oss << "  Found       : " << (bf.found ? "YES -> " + bf.foundPassword : "No") << "\n";
            oss << "  Attempts    : " << BruteForceSimulator::formatNumber(bf.totalAttempts) << "\n";
            oss << "  Elapsed     : " << std::fixed << std::setprecision(3)
                << bf.elapsedSeconds << "s\n";
            oss << "  Speed       : " << (long long)bf.attemptsPerSecond << " /sec\n\n";
        }
    }

    if (!m_dictResults.empty()) {
        oss << "---- DICTIONARY ATTACK RESULTS ----\n";
        for (const auto& d : m_dictResults) {
            oss << "  Found       : " << (d.found ? "YES -> " + d.matchedWord : "No") << "\n";
            oss << "  Words Checked: " << d.wordsChecked << " / " << d.dictionarySize << "\n";
            oss << "  Elapsed     : " << std::fixed << std::setprecision(6)
                << d.elapsedSeconds << "s\n\n";
        }
    }

    oss << "============================================================\n";
    oss << "  END OF REPORT\n";
    oss << "============================================================\n";
    return oss.str();
}

std::string ReportGenerator::buildCSVReport() const {
    std::ostringstream oss;

    // Analysis section
    oss << "TYPE,FIELD,VALUE\n";
    for (size_t i = 0; i < m_analysisEntries.size(); ++i) {
        const auto& e = m_analysisEntries[i];
        std::string prefix = "analysis_" + std::to_string(i+1);
        oss << prefix << ",score,"    << e.result.score << "\n";
        oss << prefix << ",category," << e.result.categoryLabel << "\n";
        oss << prefix << ",length,"   << e.result.length << "\n";
        oss << prefix << ",crack_time," << e.result.estimatedCrackTime << "\n";
    }

    // Benchmark section
    for (const auto& b : m_benchmarkReports) {
        oss << BenchmarkManager::reportToCSV(b);
    }

    // Brute force section
    if (!m_bruteForceResults.empty()) {
        oss << "\nbrute_force,algorithm,attempts,found,elapsed_s,speed\n";
        for (const auto& bf : m_bruteForceResults) {
            oss << "brute_force," << bf.algorithmUsed << ","
                << bf.totalAttempts << ","
                << (bf.found ? bf.foundPassword : "not_found") << ","
                << std::fixed << std::setprecision(4) << bf.elapsedSeconds << ","
                << (long long)bf.attemptsPerSecond << "\n";
        }
    }
    return oss.str();
}
