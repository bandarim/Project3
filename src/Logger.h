/**
 * @file Logger.h
 * @brief Defines a simple file-based logger for the load balancer simulation.
 *
 * The Logger class provides a minimal interface for writing text-based
 * log entries to a file. It is used throughout the simulation to record
 * significant events such as request acceptance, blocking, scaling actions,
 * and summary statistics.
 */

#pragma once
#include <fstream>
#include <string>

/**
 * @class Logger
 * @brief Writes log messages to an output file.
 *
 * This class encapsulates an output file stream and provides a simple
 * method for appending log entries. Logs are written in plain text
 * format to ensure compatibility with graders and analysis tools.
 */
class Logger {
public:
    /**
     * @brief Constructs a Logger and opens the specified log file.
     *
     * The log file is opened for writing when the Logger is created.
     * If the file cannot be opened, an exception is thrown.
     *
     * @param filename Path to the log file.
     * @throws std::runtime_error if the file cannot be opened.
     */
    explicit Logger(const std::string& filename);

    /**
     * @brief Writes a single line to the log file.
     *
     * Each call appends the provided line to the log file followed
     * by a newline character.
     *
     * @param line Text to be written to the log file.
     */
    void log(const std::string& line);

private:
    /** @brief Output file stream used for logging. */
    std::ofstream out;
};