#pragma once
#include "Logger.h"
#include "BenchmarkManager.h"
#include "PasswordAnalyzer.h"
#include "BruteForceSimulator.h"
#include "DictionaryAttackSimulator.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class ReportGenerator
 * @brief Generates TXT and CSV reports from session results.
 *
 * Demonstrates COMPOSITION: aggregates result types from multiple modules.
 */
class ReportGenerator {
public:
    explicit ReportGenerator(std::shared_ptr<Logger> logger,
                              const std::string& outputDir = "reports/");

    // Add results to current report session
    void addAnalysisResult(const std::string& password,
                            const PasswordAnalyzer::AnalysisResult& result);

    void addBenchmarkReport(const BenchmarkManager::BenchmarkReport& report);

    void addBruteForceResult(const BruteForceSimulator::SimulationResult& result);

    void addDictionaryResult(const DictionaryAttackSimulator::LookupResult& result);

    // Export functions
    bool exportTXT(const std::string& filename = "") const;
    bool exportCSV(const std::string& filename = "") const;

    void clearSession();

    std::string sessionSummary() const;

private:
    std::shared_ptr<Logger> m_logger;
    std::string m_outputDir;
    std::string m_sessionStart;

    struct AnalysisEntry {
        std::string password;
        PasswordAnalyzer::AnalysisResult result;
    };

    std::vector<AnalysisEntry> m_analysisEntries;
    std::vector<BenchmarkManager::BenchmarkReport> m_benchmarkReports;
    std::vector<BruteForceSimulator::SimulationResult> m_bruteForceResults;
    std::vector<DictionaryAttackSimulator::LookupResult> m_dictResults;

    static std::string generateFilename(const std::string& prefix,
                                         const std::string& ext);
    std::string buildTXTReport() const;
    std::string buildCSVReport() const;
};
