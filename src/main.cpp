/**
 * @mainpage Loadbalancer Documentation
 * @file main.cpp
 * @brief Entry point for the Load Balancer simulation.
 * * This file coordinates the simulation by initializing configuration,
 * setting up the streaming and processing load balancers, and running
 * the main clock-cycle loop to process requests.
 */

#include <iostream>
#include <random>
#include <string>
#include <stdexcept>

#include "Config.h"
#include "Logger.h"
#include "LoadBalancer.h"
#include "Switch.h"
#include "Request.h"

/**
 * @brief Generates a random IPv4 address string.
 * @param rng Reference to the random number generator.
 * @return A string in dotted-decimal format (e.g., "192.168.1.1").
 */
static std::string randomIPv4(std::mt19937& rng) {
    std::uniform_int_distribution<int> octet(0, 255);
    return std::to_string(octet(rng)) + "." +
           std::to_string(octet(rng)) + "." +
           std::to_string(octet(rng)) + "." +
           std::to_string(octet(rng));
}

/**
 * @brief Randomly selects a job type for a new request.
 * @param rng Reference to the random number generator.
 * @return 'P' for Processing or 'S' for Streaming.
 */
static char randomJobType(std::mt19937& rng) {
    std::uniform_int_distribution<int> pick(0, 1);
    return pick(rng) == 0 ? 'P' : 'S';
}

/**
 * @brief Prints a status update to the standard output.
 * @param t Current simulation cycle.
 * @param lb Reference to the LoadBalancer being reported.
 */
static void printSummaryLine(int t, const LoadBalancer& lb) {
    const LBStats& st = lb.getStats();
    std::cout << "[t=" << t << "] "
              << lb.getName()
              << " servers=" << lb.serverCount()
              << " queue=" << lb.queueSize()
              << " accepted=" << st.accepted
              << " blocked=" << st.blocked
              << " completed=" << st.completed
              << "\n";
}

/**
 * @brief Records a status update into the provided Logger.
 * @param logger Reference to the Logger instance.
 * @param t Current simulation cycle.
 * @param lb Reference to the LoadBalancer being reported.
 */
static void logSummaryLine(Logger& logger, int t, const LoadBalancer& lb) {
    const LBStats& st = lb.getStats();
    logger.log("SUMMARY t=" + std::to_string(t) + " " + lb.getName() +
               " servers=" + std::to_string(lb.serverCount()) +
               " queue=" + std::to_string(lb.queueSize()) +
               " accepted=" + std::to_string(st.accepted) +
               " blocked=" + std::to_string(st.blocked) +
               " completed=" + std::to_string(st.completed));
}

/**
 * @brief Displays the final simulation results for a load balancer.
 * @param lb Reference to the LoadBalancer to summarize.
 */
static void printFinalStats(const LoadBalancer& lb) {
    const LBStats& st = lb.getStats();
    std::cout << "Final Stats: " << lb.getName() << " ===\n";
    std::cout << "Servers:   " << lb.serverCount() << "\n";
    std::cout << "Queue:     " << lb.queueSize() << "\n";
    std::cout << "Accepted:  " << st.accepted << "\n";
    std::cout << "Blocked:   " << st.blocked << "\n";
    std::cout << "Completed: " << st.completed << "\n";

    if (st.completed > 0) {
        double avgWait = static_cast<double>(st.totalWaitTime) / static_cast<double>(st.completed);
        double avgSvc  = static_cast<double>(st.totalServiceTime) / static_cast<double>(st.completed);
        std::cout << "Avg Wait Time:    " << avgWait << " cycles\n";
        std::cout << "Avg Service Time: " << avgSvc  << " cycles\n";
    }
    std::cout << "\n";
}

/**
 * @brief Main execution function for the simulation.
 * * Performs the following steps:
 * 1. Loads configuration from "config.txt".
 * 2. Collects user input for server count and simulation duration.
 * 3. Initializes LoadBalancers for 'Streaming' and 'Processing' traffic.
 * 4. Seeds the initial request queues.
 * 5. Runs the simulation loop, generating random requests and ticking the system.
 * 6. Outputs final performance metrics.
 * * @return 0 on success, 1 on error.
 */
int main() {
    try {
        // Load configuration parameters
        Config cfg = Config::loadFromFile("config.txt");

        int totalServers = 0;
        int totalCycles = 0;

        std::cout << "Enter total number of servers (e.g., 10): ";
        std::cin >> totalServers;
        std::cout << "Enter total number of clock cycles (e.g., 10000): ";
        std::cin >> totalCycles;

        if (totalServers <= 0 || totalCycles <= 0) {
            throw std::runtime_error("totalServers and totalCycles must be > 0");
        }

        // Divide resources between specific traffic types
        int streamServers = totalServers / 2;
        int procServers = totalServers - streamServers;

        if (streamServers == 0) streamServers = 1;
        if (procServers == 0) procServers = 1;

        // Initialize logging
        Logger logger(cfg.logFile);
        logger.log("START");
        logger.log("TotalServers=" + std::to_string(totalServers) +
                   " StreamServers=" + std::to_string(streamServers) +
                   " ProcServers=" + std::to_string(procServers) +
                   " TotalCycles=" + std::to_string(totalCycles));

        // Create LoadBalancers and the Switch for routing
        LoadBalancer streamingLB("StreamingLB", streamServers, cfg, &logger);
        LoadBalancer processingLB("ProcessingLB", procServers, cfg, &logger);
        Switch sw(&streamingLB, &processingLB);

        // Populate initial queues to simulate a pre-existing load
        streamingLB.seedInitialQueue(streamServers * 100, 0);
        processingLB.seedInitialQueue(procServers * 100, 0);

        // Setup Random Number Generation
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        std::uniform_int_distribution<int> arrivalsDist(0, cfg.arrivalsPerCycleMax);
        std::uniform_int_distribution<int> timeDist(cfg.minJobTime, cfg.maxJobTime);

        // --- Main Simulation Loop ---
        for (int t = 1; t <= totalCycles; t++) {

            // Handle new request arrivals based on arrivalChance
            if (prob(rng) < cfg.arrivalChance) {
                int arrivals = arrivalsDist(rng);
                for (int i = 0; i < arrivals; i++) {
                    Request r;
                    r.ipIn = randomIPv4(rng);
                    r.ipOut = randomIPv4(rng);
                    r.timeRequired = timeDist(rng);
                    r.jobType = randomJobType(rng);
                    r.arrivalTime = t;

                    // Pass request to the switch to route to the correct LB
                    sw.route(r, t);
                }
            }

            // Update internal state of the load balancers (server work and scaling)
            streamingLB.tick(t);
            processingLB.tick(t);

            // Periodic summary reporting
            if (cfg.logIncrement > 0 && (t % cfg.logIncrement == 0)) {
                printSummaryLine(t, streamingLB);
                printSummaryLine(t, processingLB);

                logSummaryLine(logger, t, streamingLB);
                logSummaryLine(logger, t, processingLB);
            }
        }

        // Finalize logs and output statistics
        logger.log("END");
        printFinalStats(streamingLB);
        printFinalStats(processingLB);

        // Final detailed stats log entry
        logger.log("FINAL " + streamingLB.getName() +
                   " accepted=" + std::to_string(streamingLB.getStats().accepted) +
                   " blocked=" + std::to_string(streamingLB.getStats().blocked) +
                   " completed=" + std::to_string(streamingLB.getStats().completed));

        logger.log("FINAL " + processingLB.getName() +
                   " accepted=" + std::to_string(processingLB.getStats().accepted) +
                   " blocked=" + std::to_string(processingLB.getStats().blocked) +
                   " completed=" + std::to_string(processingLB.getStats().completed));

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}