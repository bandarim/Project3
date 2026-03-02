/**
 * @file Config.cpp
 * @brief Implements parsing/loading of simulation configuration from a text file.
 *
 * The configuration file uses a simple key=value format. Blank lines are ignored
 * and comments begin with '#'. The loader supports repeated keys for firewall
 * ranges using the form:
 *
 *   blockedRange=startIP-endIP
 *
 * These ranges are stored in Config::blockedRanges and later applied to the
 * IPFirewall during LoadBalancer initialization.
 */

#include "Config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

/**
 * @brief Trims leading and trailing whitespace from a string.
 * @param s Input string.
 * @return A copy of the string with surrounding whitespace removed.
 */

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

/**
 * @brief Parses a firewall blocked range value of the form "startIP-endIP".
 * @param value Range string from the config file (e.g., "10.0.0.0-10.255.255.255").
 * @return Pair {startIP, endIP} as strings.
 * @throws std::runtime_error if the value is malformed.
 */

static std::pair<std::string,std::string> parseBlockedRangeValue(const std::string& value) {
    // expected: "startIP-endIP"
    size_t dash = value.find('-');
    if (dash == std::string::npos) {
        throw std::runtime_error("Config: blockedRange must be startIP-endIP, got: " + value);
    }
    std::string startIP = trim(value.substr(0, dash));
    std::string endIP   = trim(value.substr(dash + 1));
    if (startIP.empty() || endIP.empty()) {
        throw std::runtime_error("Config: blockedRange start/end cannot be empty: " + value);
    }
    return {startIP, endIP};
}

/**
 * @brief Loads configuration values from a key=value text file.
 *
 * Supported keys include:
 * - arrivalChance (double, 0..1)
 * - arrivalsPerCycleMax (int)
 * - minJobTime (int)
 * - maxJobTime (int)
 * - scaleLowFactor (int)
 * - scaleHighFactor (int)
 * - cooldownN (int)
 * - logEvery (int)
 * - logFile (string)
 * - blockedRange (repeatable, "startIP-endIP")
 *
 * Any keys not present will leave Config defaults unchanged.
 *
 * @param path Path to the configuration file (e.g., "config.txt").
 * @return A populated Config object.
 * @throws std::runtime_error if the file cannot be opened or values are invalid.
 */

Config Config::loadFromFile(const std::string& path) {
    Config cfg;

    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Config: failed to open config file: " + path);
    }

    std::string line;
    int lineNo = 0;

    while (std::getline(in, line)) {
        lineNo++;

        // strip comments (# ...)
        size_t hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);

        line = trim(line);
        if (line.empty()) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            throw std::runtime_error("Config: invalid line (missing '=') at line " + std::to_string(lineNo));
        }

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        if (key.empty()) {
            throw std::runtime_error("Config: empty key at line " + std::to_string(lineNo));
        }

        // blockedRange can appear multiple times
        if (key == "blockedRange") {
            cfg.blockedRanges.push_back(parseBlockedRangeValue(val));
            continue;
        }

        try {
            if (key == "arrivalChance") {
                cfg.arrivalChance = std::stod(val);
            } else if (key == "arrivalsPerCycleMax") {
                cfg.arrivalsPerCycleMax = std::stoi(val);
            } else if (key == "minJobTime") {
                cfg.minJobTime = std::stoi(val);
            } else if (key == "maxJobTime") {
                cfg.maxJobTime = std::stoi(val);
            } else if (key == "scaleLowFactor") {
                cfg.scaleLowFactor = std::stoi(val);
            } else if (key == "scaleHighFactor") {
                cfg.scaleHighFactor = std::stoi(val);
            } else if (key == "cooldownN") {
                cfg.coolDown = std::stoi(val);
            } else if (key == "logEvery") {
                cfg.logIncrement = std::stoi(val);
            } else if (key == "logFile") {
                cfg.logFile = val;
            } else {}
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "Config: parse error for key '" + key + "' at line " + std::to_string(lineNo) +
                " with value '" + val + "'"
            );
        }
    }

    // Light validation
    if (cfg.arrivalChance < 0.0 || cfg.arrivalChance > 1.0) {
        throw std::runtime_error("Config: arrivalChance must be between 0 and 1");
    }
    if (cfg.minJobTime <= 0 || cfg.maxJobTime < cfg.minJobTime) {
        throw std::runtime_error("Config: invalid minJobTime/maxJobTime");
    }
    if (cfg.coolDown < 0) {
        throw std::runtime_error("Config: cooldownN must be >= 0");
    }
    if (cfg.scaleLowFactor <= 0 || cfg.scaleHighFactor <= 0) {
        throw std::runtime_error("Config: scaleLowFactor and scaleHighFactor must be > 0");
    }

    return cfg;
}