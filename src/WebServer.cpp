/**
 * @file WebServer.cpp
 * @brief Implements the WebServer class for processing requests.
 *
 * This file contains the implementation of the WebServer class, which
 * simulates a single server capable of processing one request at a time.
 * Each server tracks its busy state, remaining processing time, and
 * the request currently being handled.
 */

#include "WebServer.h"
#include <stdexcept>

/**
 * @brief Constructs a WebServer with a unique identifier.
 * @param id Unique server ID.
 */
WebServer::WebServer(int id) : id(id) {}

/**
 * @brief Checks whether the server is currently idle.
 * @return True if the server is not processing a request.
 */
bool WebServer::isIdle() const {
    return !busy;
}

/**
 * @brief Returns the unique identifier for this server.
 * @return Server ID.
 */
int WebServer::getId() const {
    return id;
}

/**
 * @brief Returns the request currently being processed.
 * @return Constant reference to the active Request.
 */
const Request& WebServer::currentRequest() const { 
    return current;
}

/**
 * @brief Assigns a request to this server for processing.
 *
 * The request's start time is set to the current simulation cycle.
 *
 * @param r Request to assign.
 * @param currentCycle Current simulation clock cycle.
 * @throws std::runtime_error if the server is already busy.
 */
void WebServer::assign(Request r, int currentCycle) {
    if (busy) {
        throw std::runtime_error("Can't assign on a busy server");
    }

    // Mark start time for stats
    r.startTime = currentCycle;

    current = r;
    remainingTime = r.timeRequired;
    busy = true;
}

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
bool WebServer::processCycle(int currentCycle) {
    if (!busy) return false;

    // One clock cycle of work
    remainingTime--;

    // If finished this cycle
    if (remainingTime <= 0) {
        current.finishTime = currentCycle;
        busy = false;
        remainingTime = 0;
        return true;
    }

    return false;
}