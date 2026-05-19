#include "../include/Logger.h"
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <filesystem>

Logger::Logger(const std::string& logFilePath)
    : m_filePath(logFilePath)
{
    // Ensure logs directory exists
    std::filesystem::path p(logFilePath);
    if (p.has_parent_path())
        std::filesystem::create_directories(p.parent_path());

    m_file.open(logFilePath, std::ios::app);
    if (!m_file.is_open())
        throw std::runtime_error("Cannot open log file: " + logFilePath);

    separator('=');
    info("Logger initialized. Session started.");
    separator('=');
}

Logger::~Logger() {
    info("Session ended.");
    if (m_file.is_open()) m_file.close();
}

void Logger::log(Level level, const std::string& message) {
    std::string entry = "[" + timestamp() + "] [" + levelToString(level) + "] " + message;
    writeEntry(entry);
}

void Logger::info(const std::string& msg)    { log(Level::INFO,    msg); }
void Logger::warning(const std::string& msg) { log(Level::WARNING, msg); }
void Logger::error(const std::string& msg)   { log(Level::ERROR,   msg); }
void Logger::debug(const std::string& msg)   { log(Level::DEBUG,   msg); }

void Logger::separator(char c, int width) {
    std::string line(width, c);
    writeEntry(line);
}

const std::vector<std::string>& Logger::sessionLog() const {
    return m_sessionLog;
}

void Logger::writeEntry(const std::string& entry) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessionLog.push_back(entry);
    if (m_file.is_open()) {
        m_file << entry << "\n";
        m_file.flush();
    }
}

std::string Logger::timestamp() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::INFO:    return "INFO ";
        case Level::WARNING: return "WARN ";
        case Level::ERROR:   return "ERROR";
        case Level::DEBUG:   return "DEBUG";
        default:             return "?????";
    }
}
