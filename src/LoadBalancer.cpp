#include <iostream>
#include "LoadBalancer.h"
#include <random>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "Colors.h"

/**
 * @file LoadBalancer.cpp
 * @brief Implements the LoadBalancer simulation logic.
 *
 * This file contains the implementation of the LoadBalancer class, which:
 * - Maintains a queue of incoming Request objects.
 * - Uses an IPFirewall to block requests from specified IP ranges.
 * - Dispatches queued requests to idle WebServer instances.
 * - Tracks per-load-balancer statistics (accepted/blocked/completed, timing).
 * - Automatically scales the number of servers based on queue thresholds and cooldown.
 *
 * Colored terminal output is used to visually distinguish key events such as
 * ACCEPT, BLOCK, ASSIGN, DONE, SCALE_UP, and SCALE_DOWN actions.
 */

 /**
 * @brief Generates a random IPv4 address in dotted-decimal format.
 * @param rng Random number engine used to generate octets.
 * @return Random IPv4 address string (e.g., "123.45.67.89").
 */
static std::string randomIPv4(std::mt19937& rng) {
    std::uniform_int_distribution<int> octet(0, 255);
    std::ostringstream oss;
    oss << octet(rng) << "."
        << octet(rng) << "."
        << octet(rng) << "."
        << octet(rng);
    return oss.str();
}

/**
 * @brief Randomly selects a job type for a request.
 * @param rng Random number engine used to select the type.
 * @return 'P' for processing or 'S' for streaming.
 */

static char randomJobType(std::mt19937& rng) {
    std::uniform_int_distribution<int> pick(0, 1);
    return pick(rng) == 0 ? 'P' : 'S';
}

/**
 * @brief Constructs a LoadBalancer with an initial server pool and firewall rules.
 *
 * The firewall is initialized from Config::blockedRanges. The load balancer then
 * creates an initial set of WebServer objects and prepares the internal request queue.
 *
 * @param name Human-readable name for logging and output.
 * @param initialServers Initial number of WebServer instances to allocate.
 * @param cfg Simulation configuration (arrivals, scaling thresholds, logging, firewall rules).
 * @param logger Optional logger for writing events to a log file (may be nullptr).
 * @throws std::runtime_error if initialServers <= 0.
 */

LoadBalancer::LoadBalancer(std::string name, int initialServers, const Config& cfg, Logger* logger)
    : name(std::move(name)), cfg(cfg), logger(logger) {

    if (initialServers <= 0) {
        throw std::runtime_error("LoadBalancer: initialServers must be > 0");
    }

    // Build firewall from config blocked ranges
    for (const auto& pr : cfg.blockedRanges) {
        firewall.addBlockedRange(pr.first, pr.second);
    }

    // Create initial server pool
    servers.reserve(static_cast<size_t>(initialServers));
    for (int i = 0; i < initialServers; i++) {
        servers.push_back(new WebServer(nextServerID++));
    }

    if (logger) {
        logger->log("INIT " + this->name + ": servers=" + std::to_string(initialServers));
    }
}

/**
 * @brief Destructor that releases dynamically allocated WebServer objects.
 *
 * Web servers are created using new and stored as raw pointers, so they must
 * be deleted to avoid memory leaks.
 */

LoadBalancer::~LoadBalancer() {
    for (WebServer* s : servers) {
        delete s;
    }
    servers.clear();
}

/**
 * @brief Returns a reference to the current statistics for this load balancer.
 * @return Constant reference to LBStats.
 */
const LBStats& LoadBalancer::getStats() const {
    return stats;
}

/**
 * @brief Returns the current number of allocated web servers.
 * @return Number of servers managed by this load balancer.
 */

int LoadBalancer::serverCount() const {
    return static_cast<int>(servers.size());
}

/**
 * @brief Returns the number of requests currently waiting in the queue.
 * @return Current request queue size.
 */

size_t LoadBalancer::queueSize() const {
    return rq.size();
}

/**
 * @brief Returns the name of this load balancer.
 * @return Constant reference to the load balancer name.
 */
const std::string& LoadBalancer::getName() const {
    return name;
}

/**
 * @brief Creates a randomly generated Request.
 *
 * The request includes random incoming/outgoing IP addresses, a randomized
 * service time (in cycles), and a randomly selected job type ('P' or 'S').
 *
 * @param currentCycle The current simulation clock cycle.
 * @return A randomized Request with arrivalTime set to currentCycle.
 */

Request LoadBalancer::makeRandomRequest(int currentCycle) const {
    static thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<int> tdist(cfg.minJobTime, cfg.maxJobTime);

    Request r;
    r.ipIn = randomIPv4(rng);
    r.ipOut = randomIPv4(rng);
    r.timeRequired = tdist(rng);
    r.jobType = randomJobType(rng);

    r.arrivalTime = currentCycle;
    r.startTime = -1;
    r.finishTime = -1;

    return r;
}

/**
 * @brief Seeds the load balancer with an initial batch of requests.
 *
 * Requests are generated and then passed through acceptRequest(), meaning
 * they are subject to firewall filtering and will be counted in stats.
 *
 * @param count Number of requests to seed into the system.
 * @param currentCycle The cycle used as the arrival time for seeded requests.
 */

void LoadBalancer::seedInitialQueue(int count, int currentCycle) {
    for (int i = 0; i < count; i++) {
        Request r = makeRandomRequest(currentCycle);
        acceptRequest(r, currentCycle);
    }

    if (logger) {
        logger->log("SEED " + name + ": added=" + std::to_string(count) +
                    " queue=" + std::to_string(rq.size()));
    }
}

/**
 * @brief Accepts a new incoming request into the load balancer.
 *
 * The request is first checked against the firewall. If blocked, it is dropped
 * and counted in blocked stats. Otherwise, it is enqueued and counted as accepted.
 *
 * @param r The incoming Request.
 * @param currentCycle Current simulation clock cycle (used to set arrivalTime).
 */

void LoadBalancer::acceptRequest(Request r, int currentCycle) {
    // ensure arrivalTime is set for stats
    r.arrivalTime = currentCycle;

    if (firewall.isBlocked(r.ipIn)) {
        stats.blocked++;
        if (logger) {
            logger->log("BLOCK " + name + " t=" + std::to_string(currentCycle) +
                        " ipIn=" + r.ipIn);
        }
        std::cout << Color::RED
          << "[BLOCK] " << name
          << " t=" << currentCycle
          << " ip=" << r.ipIn
          << Color::RESET << "\n";
        return;
    }

    rq.enqueue(r);
    stats.accepted++;

    if (logger) {
        logger->log("ACCEPT " + name + " t=" + std::to_string(currentCycle) +
                    " ipIn=" + r.ipIn + " ipOut=" + r.ipOut +
                    " type=" + std::string(1, r.jobType) +
                    " time=" + std::to_string(r.timeRequired) +
                    " queue=" + std::to_string(rq.size()));
    }
    std::cout << Color::GREEN
        << "[ACCEPT] " << name
        << " t=" << currentCycle
        << " queue=" << rq.size()
        << Color::RESET << "\n";
}

/**
 * @brief Advances the load balancer simulation by one clock cycle.
 *
 * Performs the following actions:
 * 1) Decrements cooldown (if active).
 * 2) Updates servers (processing/completion).
 * 3) Dispatches queued requests to idle servers.
 * 4) Applies autoscaling decisions (scale up/down) when cooldown allows.
 *
 * @param currentCycle Current simulation clock cycle.
 */

void LoadBalancer::tick(int currentCycle) {
    if (coolDownRemaining > 0) {
        coolDownRemaining--;
    }

    updateServers(currentCycle);
    dispatch(currentCycle);
    scaleIfNeeded(currentCycle);
}

/**
 * @brief Advances processing on each WebServer by one cycle and records completions.
 *
 * When a request completes on a server, statistics are updated (completed count,
 * wait time, service time) and a completion event is logged.
 *
 * @param currentCycle Current simulation clock cycle.
 */
void LoadBalancer::updateServers(int currentCycle) {
    for (WebServer* s : servers) {
        if (!s) continue;

        // If a job finishes this cycle, update stats/logs
        if (!s->isIdle()) {
            bool finished = s->processCycle(currentCycle);
            if (finished) {
                const Request& done = s->currentRequest();
                stats.completed++;

                // wait time = start - arrival (startTime set on assign)
                if (done.startTime >= 0) {
                    stats.totalWaitTime += (done.startTime - done.arrivalTime);
                }
                stats.totalServiceTime += done.timeRequired;

                if (logger) {
                    logger->log("DONE " + name +
                                " t=" + std::to_string(currentCycle) +
                                " server=" + std::to_string(s->getId()) +
                                " type=" + std::string(1, done.jobType) +
                                " wait=" + (done.startTime >= 0 ? std::to_string(done.startTime - done.arrivalTime) : std::string("NA")) +
                                " service=" + std::to_string(done.timeRequired));
                }
                std::cout << Color::MAGENTA
                    << "[DONE] " << name
                    << " server=" << s->getId()
                    << Color::RESET << "\n";
            }
        }
    }
}

/**
 * @brief Assigns queued requests to available (idle) WebServer instances.
 *
 * This function iterates over servers and assigns the next queued request
 * to each idle server until either the queue is empty or no servers are idle.
 *
 * @param currentCycle Current simulation clock cycle (used to set request start time).
 */

void LoadBalancer::dispatch(int currentCycle) {
    for (WebServer* s : servers) {
        if (!s) continue;
        if (rq.empty()) break;

        if (s->isIdle()) {
            Request r = rq.dequeue();
            s->assign(r, currentCycle);

            if (logger) {
                logger->log("ASSIGN " + name +
                            " t=" + std::to_string(currentCycle) +
                            " server=" + std::to_string(s->getId()) +
                            " type=" + std::string(1, r.jobType) +
                            " time=" + std::to_string(r.timeRequired) +
                            " queue=" + std::to_string(rq.size()));
            }
            std::cout << Color::BLUE
                << "[ASSIGN] " << name
                << " server=" << s->getId()
                << Color::RESET << "\n";
        }
    }
}

/**
 * @brief Adjusts the number of web servers based on queue size and scaling thresholds.
 *
 * Scaling rules:
 * - If queueSize > (scaleHighFactor * servers), scale up by 1 server.
 * - If queueSize < (scaleLowFactor * servers), scale down by 1 server (if an idle server exists).
 * - Scaling actions are limited by a cooldown period to prevent oscillation.
 *
 * @param currentCycle Current simulation clock cycle.
 */

void LoadBalancer::scaleIfNeeded(int currentCycle) {
    // Respect cooldown
    if (coolDownRemaining > 0) return;

    int S = serverCount();
    size_t Q = queueSize();

    // Avoid division/threshold weirdness if S hits 0 (should not happen if you prevent it)
    if (S <= 0) {
        // If we somehow have no servers, add one if there is any work.
        servers.push_back(new WebServer(nextServerID++));
        coolDownRemaining = cfg.coolDown;
        if (logger) {
            logger->log("SCALE_UP " + name + " t=" + std::to_string(currentCycle) +
                        " reason=no_servers newServers=" + std::to_string(serverCount()) +
                        " queue=" + std::to_string(Q));
        }

        std::cout << Color::YELLOW
          << "[SCALE UP] " << name
          << " servers=" << serverCount()
          << Color::RESET << "\n";
        
        return;
    }

    // Scale up
    if (Q > static_cast<size_t>(cfg.scaleHighFactor * S)) {
        servers.push_back(new WebServer(nextServerID++));
        coolDownRemaining = cfg.coolDown;

        if (logger) {
            logger->log("SCALE_UP " + name + " t=" + std::to_string(currentCycle) +
                        " newServers=" + std::to_string(serverCount()) +
                        " queue=" + std::to_string(Q));
        }
        std::cout << Color::YELLOW
          << "[SCALE UP] " << name
          << " servers=" << serverCount()
          << Color::RESET << "\n";

        return;
    }

    // Scale down (don't go below 1 server)
    if (S > 1 && Q < static_cast<size_t>(cfg.scaleLowFactor * S)) {
        // Prefer removing an idle server
        auto it = std::find_if(servers.begin(), servers.end(),
                               [](WebServer* s) { return s && s->isIdle(); });

        if (it != servers.end()) {
            delete *it;
            servers.erase(it);
            coolDownRemaining = cfg.coolDown;

            if (logger) {
                logger->log("SCALE_DOWN " + name + " t=" + std::to_string(currentCycle) +
                            " newServers=" + std::to_string(serverCount()) +
                            " queue=" + std::to_string(Q));
            }
        } else {
            // No idle servers: skip scale down safely
            if (logger) {
                logger->log("SCALE_DOWN_SKIP " + name + " t=" + std::to_string(currentCycle) +
                            " reason=no_idle_servers servers=" + std::to_string(S) +
                            " queue=" + std::to_string(Q));
            }
        }
        std::cout << "\033[38;5;208m"  // orange
          << "[SCALE DOWN] " << name
          << " servers=" << serverCount()
          << "\033[0m\n";
    }
}