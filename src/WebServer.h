/**
 * @file WebServer.h
 * @brief Defines the WebServer class used to process requests.
 *
 * The WebServer class simulates a single web server that can process
 * one Request at a time. Each server tracks its busy state, remaining
 * processing time, and the request currently being handled.
 */

#pragma once
#include "Request.h"

/**
 * @class WebServer
 * @brief Simulates a single web server in the load balancer system.
 *
 * A WebServer processes requests assigned by a LoadBalancer. It can
 * handle only one request at a time and operates on a per-clock-cycle
 * basis within the simulation.
 */
class WebServer {
public:
    /**
     * @brief Constructs a WebServer with a unique identifier.
     *
     * @param id Unique identifier for this server.
     */
    explicit WebServer(int id);

    /**
     * @brief Checks whether the server is currently idle.
     * @return True if the server is not processing a request.
     */
    bool isIdle() const;

    /**
     * @brief Assigns a request to this server for processing.
     *
     * The request's start time is set to the current simulation cycle.
     *
     * @param r Request to assign.
     * @param currentCycle Current simulation clock cycle.
     * @throws std::runtime_error if the server is already busy.
     */
    void assign(Request r, int currentCycle);

    /**
     * @brief Processes one simulation clock cycle of work.
     *
     * Decrements the remaining processing time. If the request finishes
     * during this cycle, the server becomes idle and the request's
     * finish time is recorded.
     *
     * @param currentCycle Current simulation clock cycle.
     * @return True if the request finished during this cycle; false otherwise.
     */
    bool processCycle(int currentCycle);

    /**
     * @brief Returns the unique identifier of this server.
     * @return Server ID.
     */
    int getId() const;

    /**
     * @brief Returns the request currently being processed.
     * @return Constant reference to the active Request.
     */
    const Request& currentRequest() const;

private:
    /** @brief Unique identifier for the server. */
    int id;

    /** @brief Indicates whether the server is currently busy. */
    bool busy = false;

    /** @brief Remaining processing time for the current request (in cycles). */
    int remainingTime = 0;

    /** @brief Request currently being processed by the server. */
    Request current;
};