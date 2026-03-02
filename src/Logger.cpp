/**
 * @file Logger.cpp
 * @brief Implements simple file-based logging functionality.
 *
 * This file contains the implementation of the Logger class, which is
 * responsible for writing simulation events and summaries to a log file.
 * Logging is performed in plain text to ensure compatibility with graders
 * and analysis tools.
 */

#include "Logger.h"
#include <stdexcept>

/**
 * @brief Constructs a Logger and opens the specified log file.
 *
 * The log file is opened in output mode. If the file cannot be opened,
 * an exception is thrown to indicate a fatal configuration error.
 *
 * @param filename Path to the log file.
 * @throws std::runtime_error if the file cannot be opened.
 */

Logger::Logger(const std::string& filename) : out(filename) {
    if (!out.is_open()){
        throw std::runtime_error("Logger: failed to open log file: " + filename);
    }
}

/**
 * @brief Writes a single line to the log file.
 *
 * Each log entry is written on its own line and flushed immediately
 * to ensure that output is not lost if the program terminates early.
 *
 * @param line Text line to write to the log file.
 */
void Logger::log(const std::string& line){
    if(out.is_open()){
        out << line << "\n";
        out.flush();
    }
}