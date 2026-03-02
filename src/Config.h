/**
 * @file Config.h
 * @brief Defines the Config structure used to configure the load balancer simulation.
 *
 * The Config structure stores all tunable parameters for the simulation, including
 * request arrival behavior, autoscaling thresholds, logging options, and firewall
 * IP blocking rules. Values may be loaded from an external configuration file
 * to allow easy modification without recompiling the program.
 */

#pragma once
#include <string>
#include <vector>
#include <utility>

/**
 * @struct Config
 * @brief Holds all configurable parameters for the load balancer simulation.
 *
 * This structure provides default values for all configuration options.
 * Any values specified in the configuration file will override these defaults.
 */
struct Config {

    /** @name Simulation Parameters */
    ///@{

    /** @brief Probability (0.0–1.0) that new requests arrive during a clock cycle. */
    double arrivalChance = 0.3;

    /** @brief Maximum number of new requests that may arrive in a single cycle. */
    int arrivalsPerCycleMax = 3;

    /** @brief Minimum processing time (in cycles) for a request. */
    int minJobTime = 2;

    /** @brief Maximum processing time (in cycles) for a request. */
    int maxJobTime = 25;

    ///@}

    /** @name Autoscaling Parameters */
    ///@{

    /** @brief Lower queue threshold factor for scaling down servers. */
    int scaleLowFactor = 50;

    /** @brief Upper queue threshold factor for scaling up servers. */
    int scaleHighFactor = 80;

    /** @brief Cooldown period (in cycles) between scaling actions. */
    int coolDown = 25;

    ///@}

    /** @name Logging Parameters */
    ///@{

    /** @brief Number of cycles between summary log entries. */
    int logIncrement = 100;

    /** @brief Path to the output log file. */
    std::string logFile = "log.txt";

    ///@}

    /** @name Firewall Parameters */
    ///@{

    /**
     * @brief List of blocked IP address ranges.
     *
     * Each entry is a pair of strings representing the start and end IP
     * addresses of a blocked range. Requests with incoming IPs that fall
     * within any of these ranges will be rejected by the firewall.
     */
    std::vector<std::pair<std::string, std::string>> blockedRanges;

    ///@}

    /**
     * @brief Loads configuration values from a text file.
     *
     * The configuration file uses a key=value format. Any values not specified
     * in the file will retain their default values.
     *
     * @param path Path to the configuration file.
     * @return A Config object populated with values from the file.
     * @throws std::runtime_error if the file cannot be opened or contains invalid values.
     */
    static Config loadFromFile(const std::string& path);
};