#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <vector>

/**
 * @class Logger
 * @brief Thread-safe event logger for the toolkit.
 *
 * Demonstrates ENCAPSULATION: internal file handle and mutex are fully private.
 * Follows the Single Responsibility Principle — logging only.
 */
class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR,
        DEBUG
    };

    explicit Logger(const std::string& logFilePath = "logs/toolkit.log");
    ~Logger();

    // Disable copy; logger is a singleton-like resource
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(Level level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);

    // Separator line for readability in log files
    void separator(char c = '-', int width = 60);

    const std::vector<std::string>& sessionLog() const;

    static std::string timestamp();
    static std::string levelToString(Level level);

private:
    std::ofstream m_file;
    std::mutex m_mutex;
    std::vector<std::string> m_sessionLog;  // In-memory buffer
    std::string m_filePath;

    void writeEntry(const std::string& entry);
};
