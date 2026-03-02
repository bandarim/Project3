/**
 * @file LoadBalancer.h
 * @brief Defines the LoadBalancer class for distributing requests across web servers.
 *
 * The LoadBalancer manages a queue of incoming requests, applies IP-based
 * firewall rules, dispatches requests to available WebServer instances,
 * tracks performance statistics, and dynamically scales the number of
 * servers based on system load.
 */

#pragma once
#include <vector>
#include <string>
#include "RequestQueue.h"
#include "WebServer.h"
#include "IPFirewall.h"
#include "Config.h"
#include "Logger.h"

/**
 * @struct LBStats
 * @brief Stores performance statistics for a load balancer instance.
 *
 * These statistics are used to evaluate system behavior such as throughput,
 * blocking rate, average wait time, and average service time.
 */
struct LBStats {

    /** @brief Number of requests successfully accepted into the system. */
    long long accepted = 0;

    /** @brief Number of requests blocked by the firewall. */
    long long blocked = 0;

    /** @brief Number of requests fully processed by web servers. */
    long long completed = 0;

    /** @brief Total accumulated wait time for completed requests (in cycles). */
    long long totalWaitTime = 0;

    /** @brief Total accumulated service time for completed requests (in cycles). */
    long long totalServiceTime = 0;
};

/**
 * @class LoadBalancer
 * @brief Simulates a load balancer that manages web requests and servers.
 *
 * The LoadBalancer accepts incoming requests, filters them through an
 * IPFirewall, queues accepted requests, assigns them to idle WebServer
 * instances, and dynamically scales server capacity based on queue size
 * thresholds defined in the configuration.
 */
class LoadBalancer {
public:
    /**
     * @brief Constructs a LoadBalancer with an initial number of servers.
     *
     * Firewall rules are initialized from the configuration, and the
     * specified number of WebServer instances are created.
     *
     * @param name Human-readable name for logging and output.
     * @param initialServers Initial number of web servers to create.
     * @param cfg Configuration parameters for simulation and scaling.
     * @param logger Optional logger for writing events to a log file.
     * @throws std::runtime_error if initialServers <= 0.
     */
    LoadBalancer(std::string name, int initialServers, const Config& cfg, Logger* logger);

    /**
     * @brief Destructor that releases dynamically allocated web servers.
     */
    ~LoadBalancer();

    /**
     * @brief Seeds the load balancer with an initial batch of requests.
     *
     * Requests are generated randomly and passed through the normal
     * acceptance logic, including firewall filtering.
     *
     * @param count Number of requests to seed.
     * @param currentCycle Simulation cycle used as the arrival time.
     */
    void seedInitialQueue(int count, int currentCycle);

    /**
     * @brief Accepts a new incoming request.
     *
     * The request is checked against the firewall. Blocked requests are
     * discarded, while accepted requests are added to the request queue.
     *
     * @param r Incoming request.
     * @param currentCycle Current simulation clock cycle.
     */
    void acceptRequest(Request r, int currentCycle);

    /**
     * @brief Advances the simulation by one clock cycle.
     *
     * This function updates server processing, dispatches queued requests,
     * and applies autoscaling decisions when appropriate.
     *
     * @param currentCycle Current simulation clock cycle.
     */
    void tick(int currentCycle);

    /**
     * @brief Returns performance statistics for this load balancer.
     * @return Constant reference to LBStats.
     */
    const LBStats& getStats() const;

    /**
     * @brief Returns the current number of active web servers.
     * @return Number of servers.
     */
    int serverCount() const;

    /**
     * @brief Returns the current size of the request queue.
     * @return Number of queued requests.
     */
    size_t queueSize() const;

    /**
     * @brief Returns the name of this load balancer.
     * @return Constant reference to the load balancer name.
     */
    const std::string& getName() const;

private:
    /**
     * @brief Dispatches queued requests to idle web servers.
     * @param currentCycle Current simulation clock cycle.
     */
    void dispatch(int currentCycle);

    /**
     * @brief Updates server processing and records completed requests.
     * @param currentCycle Current simulation clock cycle.
     */
    void updateServers(int currentCycle);

    /**
     * @brief Applies autoscaling rules to add or remove servers.
     * @param currentCycle Current simulation clock cycle.
     */
    void scaleIfNeeded(int currentCycle);

    /**
     * @brief Generates a randomized request for simulation purposes.
     * @param currentCycle Simulation cycle used as the arrival time.
     * @return Randomly generated Request.
     */
    Request makeRandomRequest(int currentCycle) const;

private:
    /** @brief Name of the load balancer (used for logging and output). */
    std::string name;

    /** @brief Reference to global configuration parameters. */
    const Config& cfg;

    /** @brief Optional logger for writing events to a log file. */
    Logger* logger;

    /** @brief Queue holding accepted requests awaiting processing. */
    RequestQueue rq;

    /** @brief Firewall used to block requests from configured IP ranges. */
    IPFirewall firewall;

    /** @brief Collection of dynamically allocated web servers. */
    std::vector<WebServer*> servers;

    /** @brief Remaining cooldown time before another scaling action is allowed. */
    int coolDownRemaining = 0;

    /** @brief Statistics tracking load balancer performance. */
    LBStats stats;

    /** @brief Counter used to assign unique IDs to web servers. */
    int nextServerID = 1;
};