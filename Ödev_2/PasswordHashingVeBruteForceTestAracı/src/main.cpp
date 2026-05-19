#include <iostream>
#include <memory>
#include <stdexcept>
#include <iomanip>

#include "../include/HashAlgorithm.h"
#include "../include/MD5Hash.h"
#include "../include/SHA1Hash.h"
#include "../include/SHA256Hash.h"
#include "../include/PasswordAnalyzer.h"
#include "../include/BruteForceSimulator.h"
#include "../include/DictionaryAttackSimulator.h"
#include "../include/SaltDemonstration.h"
#include "../include/BenchmarkManager.h"
#include "../include/Logger.h"
#include "../include/ReportGenerator.h"
#include "../include/MenuSystem.h"

// ─────────────────────────────────────────────────────────────
//  Helper: pick algorithm at runtime
// ─────────────────────────────────────────────────────────────
static std::shared_ptr<HashAlgorithm> pickAlgorithm(MenuSystem& ui) {
    ui.printInfo("Select hashing algorithm:");
    std::cout << "    1. MD5    (128-bit  – educational only, broken)\n";
    std::cout << "    2. SHA1   (160-bit  – deprecated, educational)\n";
    std::cout << "    3. SHA256 (256-bit  – recommended)\n";
    int choice = MenuSystem::getIntInput("  Algorithm [1-3]: ", 1, 3);
    switch (choice) {
        case 1: return std::make_shared<MD5Hash>();
        case 2: return std::make_shared<SHA1Hash>();
        default: return std::make_shared<SHA256Hash>();
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 1 – Generate Hash
// ─────────────────────────────────────────────────────────────
static void featureGenerateHash(MenuSystem& ui, ReportGenerator& report,
                                 Logger& logger)
{
    ui.printSectionHeader("GENERATE HASH");
    auto algo = pickAlgorithm(ui);
    std::string pw = MenuSystem::getInput("  Enter password/text: ");
    if (pw.empty()) { ui.printError("Input cannot be empty."); return; }

    try {
        std::string h = algo->hash(pw);
        ui.printSeparator();
        ui.printResult("Algorithm",  algo->algorithmName());
        ui.printResult("Input",      std::string(pw.size(), '*'));
        ui.printResult("Hash",       h);
        ui.printResult("Output Bits",std::to_string(algo->outputBits()));
        logger.info("Hash generated using " + algo->algorithmName());

        if (algo->algorithmName() == "MD5")
            ui.printWarning("MD5 is cryptographically broken. Use SHA256 for real applications.");
        if (algo->algorithmName() == "SHA1")
            ui.printWarning("SHA1 is deprecated. Use SHA256 for real applications.");
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Hash generation failed: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 2 – Verify Password
// ─────────────────────────────────────────────────────────────
static void featureVerifyPassword(MenuSystem& ui, Logger& logger)
{
    ui.printSectionHeader("VERIFY PASSWORD");
    auto algo = pickAlgorithm(ui);
    std::string pw   = MenuSystem::getInput("  Enter password: ");
    std::string hash = MenuSystem::getInput("  Enter expected hash: ");

    if (pw.empty() || hash.empty()) { ui.printError("Fields cannot be empty."); return; }

    try {
        bool ok = algo->verify(pw, hash);
        ui.printSeparator();
        if (ok) ui.printSuccess("Password MATCHES the hash.");
        else    ui.printError("Password does NOT match the hash.");
        logger.info("Verify attempt: " + std::string(ok ? "match" : "no match"));
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Verify failed: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 3 – Password Strength Analysis
// ─────────────────────────────────────────────────────────────
static void featureStrengthAnalysis(MenuSystem& ui, ReportGenerator& report,
                                     Logger& logger)
{
    ui.printSectionHeader("PASSWORD STRENGTH ANALYSIS");
    std::string pw = MenuSystem::getInput("  Enter password to analyze: ");
    if (pw.empty()) { ui.printError("Password cannot be empty."); return; }

    try {
        PasswordAnalyzer analyzer;
        auto r = analyzer.analyze(pw);

        ui.printSeparator();
        ui.printResult("Password",      std::string(pw.size(), '*'));
        ui.printResult("Length",        std::to_string(r.length));
        ui.printResult("Score",         PasswordAnalyzer::scoreBar(r.score));
        ui.printResult("Category",      r.categoryLabel);
        ui.printResult("Est. Crack Time", r.estimatedCrackTime);
        ui.printSeparator('-');

        std::cout << "\n  Character Composition:\n";
        ui.printResult("  Uppercase",  r.hasUpper      ? "Yes (+10)" : "No");
        ui.printResult("  Lowercase",  r.hasLower      ? "Yes (+10)" : "No");
        ui.printResult("  Digits",     r.hasDigit      ? "Yes (+10)" : "No");
        ui.printResult("  Symbols",    r.hasSymbol     ? "Yes (+15)" : "No");
        ui.printResult("  Dict. Word", r.hasDictWord   ? "FOUND (-20)" : "Clean");
        ui.printResult("  Sequential", r.hasSequential ? "FOUND (-15)" : "None");
        ui.printResult("  Repeated",   r.hasRepeated   ? "FOUND (-15)" : "None");

        std::cout << "\n  Recommendations:\n";
        for (const auto& rec : r.recommendations)
            std::cout << "    → " << rec << "\n";

        report.addAnalysisResult(pw, r);
        logger.info("Password analysis completed. Score: " + std::to_string(r.score));
    } catch (const std::exception& e) {
        ui.printError(e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 4 – Brute Force Simulation
// ─────────────────────────────────────────────────────────────
static void featureBruteForce(MenuSystem& ui, ReportGenerator& report,
                               Logger& logger)
{
    ui.printSectionHeader("BRUTE FORCE SIMULATION (SAFE/EDUCATIONAL)");
    ui.printWarning("This is a strictly limited educational simulation.");
    ui.printInfo("Max password length: " +
        std::to_string(BruteForceSimulator::MAX_PASSWORD_LENGTH) + " chars.");
    ui.printInfo("Max attempts: " +
        BruteForceSimulator::formatNumber(BruteForceSimulator::MAX_ATTEMPTS));

    auto algo = pickAlgorithm(ui);

    std::cout << "\n  Attack Profile:\n";
    std::cout << "    1. Numeric Only       (0-9, 10 chars)\n";
    std::cout << "    2. Lowercase Only     (a-z, 26 chars)\n";
    std::cout << "    3. Lowercase + Digits (a-z + 0-9, 36 chars)\n";
    int profileChoice = MenuSystem::getIntInput("  Profile [1-3]: ", 1, 3);
    BruteForceSimulator::AttackProfile profile;
    switch (profileChoice) {
        case 1: profile = BruteForceSimulator::AttackProfile::NumericOnly; break;
        case 2: profile = BruteForceSimulator::AttackProfile::LowercaseOnly; break;
        default: profile = BruteForceSimulator::AttackProfile::LowercaseAndDigits;
    }

    int maxLen = MenuSystem::getIntInput(
        "  Max password length [1-" +
        std::to_string(BruteForceSimulator::MAX_PASSWORD_LENGTH) + "]: ",
        1, BruteForceSimulator::MAX_PASSWORD_LENGTH);

    std::string targetPassword = MenuSystem::getInput(
        "  Enter password to crack (must be simple!): ");
    if (targetPassword.empty()) { ui.printError("Target cannot be empty."); return; }

    try {
        std::string targetHash = algo->hash(targetPassword);
        ui.printResult("Target Hash", targetHash);
        ui.printInfo("Starting simulation...\n");

        BruteForceSimulator sim(algo);
        auto result = sim.simulate(
            targetHash, profile, maxLen,
            [&](long long attempts, double elapsed) {
                std::cout << "\r  Attempts: " << std::setw(10)
                          << BruteForceSimulator::formatNumber(attempts)
                          << "  Elapsed: " << std::fixed << std::setprecision(1)
                          << elapsed << "s  " << std::flush;
            }
        );
        std::cout << "\n";

        ui.printSeparator();
        if (result.found) {
            ui.printSuccess("Password FOUND: " + result.foundPassword);
        } else {
            ui.printWarning("Password NOT found within limits.");
        }
        ui.printResult("Attempts",    BruteForceSimulator::formatNumber(result.totalAttempts));
        ui.printResult("Total Combs.",BruteForceSimulator::formatNumber(result.totalCombinations));
        ui.printResult("Elapsed",     std::to_string(result.elapsedSeconds).substr(0,6) + "s");
        ui.printResult("Speed",       BruteForceSimulator::formatNumber((long long)result.attemptsPerSecond) + " /sec");
        ui.printResult("Est. Full",   result.estimatedFullTime);

        // Complexity table
        std::cout << "\n  Complexity Growth (charset=" << result.charsetUsed.size() << "):\n";
        std::vector<std::string> headers = {"Length", "Combinations"};
        std::vector<std::vector<std::string>> rows;
        for (int l = 1; l <= BruteForceSimulator::MAX_PASSWORD_LENGTH; ++l) {
            long long c = BruteForceSimulator::calculateCombinations(result.charsetUsed.size(), l);
            rows.push_back({std::to_string(l), BruteForceSimulator::formatNumber(c)});
        }
        ui.printTable(headers, rows);

        report.addBruteForceResult(result);
        logger.info("Brute-force simulation completed. Found: " +
            std::string(result.found ? "YES" : "NO"));
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Brute-force error: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 5 – Dictionary Attack Simulation
// ─────────────────────────────────────────────────────────────
static void featureDictionaryAttack(MenuSystem& ui, ReportGenerator& report,
                                     Logger& logger)
{
    ui.printSectionHeader("DICTIONARY ATTACK SIMULATION");
    ui.printInfo("Using built-in educational wordlist.");

    auto algo = pickAlgorithm(ui);
    DictionaryAttackSimulator dictSim(algo);

    ui.printResult("Dictionary size", std::to_string(dictSim.dictionarySize()) + " words");

    std::string targetPassword = MenuSystem::getInput(
        "  Enter password to test (try a common one!): ");
    if (targetPassword.empty()) { ui.printError("Input cannot be empty."); return; }

    try {
        std::string targetHash = algo->hash(targetPassword);
        ui.printResult("Hash to crack", targetHash);
        ui.printInfo("Running dictionary attack...\n");

        auto result = dictSim.attack(targetHash);

        ui.printSeparator();
        if (result.found) {
            ui.printSuccess("Password FOUND in dictionary: \"" + result.matchedWord + "\"");
            ui.printWarning("This password is extremely weak and should NEVER be used!");
        } else {
            ui.printSuccess("Password NOT found in dictionary.");
            ui.printInfo("Not in wordlist, but may still be weak. Run strength analysis.");
        }
        ui.printResult("Words checked", std::to_string(result.wordsChecked)
                        + " / " + std::to_string(result.dictionarySize));
        ui.printResult("Time elapsed",
            std::to_string(result.elapsedSeconds * 1000).substr(0,8) + " ms");

        report.addDictionaryResult(result);
        logger.info("Dictionary attack completed. Found: " +
            std::string(result.found ? "YES" : "NO"));
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Dictionary attack error: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 6 – Benchmark Algorithms
// ─────────────────────────────────────────────────────────────
static void featureBenchmark(MenuSystem& ui, ReportGenerator& report,
                              Logger& logger)
{
    ui.printSectionHeader("ALGORITHM BENCHMARK");
    int iters = MenuSystem::getIntInput("  Iterations per algorithm [100-5000]: ", 100, 5000);

    ui.printInfo("Benchmarking MD5, SHA1, SHA256...\n");

    BenchmarkManager bm;
    bm.addAlgorithm(std::make_shared<MD5Hash>());
    bm.addAlgorithm(std::make_shared<SHA1Hash>());
    bm.addAlgorithm(std::make_shared<SHA256Hash>());

    try {
        auto bmReport = bm.runBenchmark(iters);
        BenchmarkManager::printReport(bmReport);
        report.addBenchmarkReport(bmReport);
        logger.info("Benchmark completed with " + std::to_string(iters) + " iterations.");
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Benchmark error: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 7 – Salt Demonstration
// ─────────────────────────────────────────────────────────────
static void featureSaltDemo(MenuSystem& ui, Logger& logger)
{
    ui.printSectionHeader("SALT DEMONSTRATION");
    auto algo = pickAlgorithm(ui);
    SaltDemonstration saltDemo(algo);

    ui.printInfo("Demonstrating identical password with two different salts:\n");

    std::string pw = MenuSystem::getInput("  Enter a password: ");
    if (pw.empty()) { ui.printError("Password cannot be empty."); return; }

    try {
        auto result = saltDemo.demonstrateIdenticalPasswords(pw);

        ui.printSeparator();
        for (size_t i = 0; i < result.entries.size(); ++i) {
            const auto& e = result.entries[i];
            std::cout << "\n  Instance " << (i+1) << ":\n";
            ui.printResult("  Salt",         e.salt);
            ui.printResult("  Unsalted Hash",e.unsaltedHash);
            ui.printResult("  Salted Hash",  e.saltedHash);
        }

        ui.printSeparator();
        if (result.allHashesDifferent)
            ui.printSuccess("Salted hashes are DIFFERENT (as expected).");
        else
            ui.printWarning("Hash collision detected (should not happen).");

        ui.printInfo("Unsalted hashes are IDENTICAL — attacker can spot password reuse.");

        std::cout << "\n  Explanation:\n";
        for (char c : result.explanation) std::cout << c;
        std::cout << "\n";

        // Simulate storage format
        std::cout << "\n  Storage Format Demo:\n";
        std::string stored = saltDemo.generateStorageString(pw);
        ui.printResult("  Stored String", stored);
        bool verified = saltDemo.verifyStorageString(pw, stored);
        if (verified) ui.printSuccess("Storage string verified successfully.");

        logger.info("Salt demonstration completed.");
    } catch (const std::exception& e) {
        ui.printError(e.what());
        logger.error(std::string("Salt demo error: ") + e.what());
    }
}

// ─────────────────────────────────────────────────────────────
//  Feature 8 – Export Report
// ─────────────────────────────────────────────────────────────
static void featureExportReport(MenuSystem& ui, ReportGenerator& report,
                                 Logger& logger)
{
    ui.printSectionHeader("EXPORT REPORT");
    std::cout << report.sessionSummary() << "\n";

    std::cout << "  Export format:\n";
    std::cout << "    1. TXT only\n";
    std::cout << "    2. CSV only\n";
    std::cout << "    3. Both TXT and CSV\n";
    int choice = MenuSystem::getIntInput("  Choice [1-3]: ", 1, 3);

    if (choice == 1 || choice == 3) report.exportTXT();
    if (choice == 2 || choice == 3) report.exportCSV();

    logger.info("Report exported.");
}

// ─────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────
int main() {
    // Infrastructure
    auto logger = std::make_shared<Logger>("logs/toolkit.log");
    auto reportGen = std::make_shared<ReportGenerator>(logger, "reports/");
    MenuSystem ui;

    ui.printBanner();
    logger->info("Defensive Security Toolkit started.");

    const std::vector<MenuSystem::MenuItem> menuItems = {
        {1, "Generate Hash",             "Hash any text with MD5/SHA1/SHA256"},
        {2, "Verify Password",           "Check password against a stored hash"},
        {3, "Password Strength Analysis","Score and analyse a password"},
        {4, "Brute Force Simulation",    "Educational brute-force demo (limited)"},
        {5, "Dictionary Attack Sim.",    "Test against common password list"},
        {6, "Benchmark Algorithms",      "Compare hash speeds"},
        {7, "Salt Demonstration",        "See why salting matters"},
        {8, "Export Report",             "Save session results to file"},
        {0, "Exit",                      ""}
    };

    bool running = true;
    while (running) {
        ui.printMenu(menuItems);
        int choice = MenuSystem::getIntInput("  Select option: ", 0, 8);
        std::cout << "\n";

        try {
            switch (choice) {
                case 1: featureGenerateHash(ui, *reportGen, *logger);       break;
                case 2: featureVerifyPassword(ui, *logger);                 break;
                case 3: featureStrengthAnalysis(ui, *reportGen, *logger);   break;
                case 4: featureBruteForce(ui, *reportGen, *logger);         break;
                case 5: featureDictionaryAttack(ui, *reportGen, *logger);   break;
                case 6: featureBenchmark(ui, *reportGen, *logger);          break;
                case 7: featureSaltDemo(ui, *logger);                       break;
                case 8: featureExportReport(ui, *reportGen, *logger);       break;
                case 0:
                    ui.printInfo("Exiting Defensive Security Toolkit. Stay safe!");
                    logger->info("Toolkit exited gracefully.");
                    running = false;
                    break;
            }
        } catch (const std::exception& e) {
            ui.printError(std::string("Unexpected error: ") + e.what());
            logger->error(std::string("Unhandled exception: ") + e.what());
        }

        if (running) {
            std::cout << "\n";
            ui.printSeparator();
        }
    }

    return 0;
}
