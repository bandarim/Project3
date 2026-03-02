/**
 * @file RequestQueue.h
 * @brief Defines a FIFO queue for storing incoming requests.
 *
 * The RequestQueue class provides a simple abstraction over a standard
 * FIFO queue to manage Request objects waiting to be processed by
 * the load balancer.
 */

#pragma once
#include <queue>
#include "Request.h"

/**
 * @class RequestQueue
 * @brief Manages a first-in, first-out queue of Request objects.
 *
 * This class is used by the LoadBalancer to store accepted requests
 * until they can be dispatched to an available WebServer.
 */
class RequestQueue {
public:
    /**
     * @brief Adds a request to the back of the queue.
     * @param r The Request to enqueue.
     */
    void enqueue(const Request& r);

    /**
     * @brief Removes and returns the request at the front of the queue.
     *
     * Requests are returned in FIFO order.
     *
     * @return The next Request in the queue.
     * @throws std::runtime_error if the queue is empty.
     */
    Request dequeue();

    /**
     * @brief Checks whether the request queue is empty.
     * @return True if the queue contains no requests; false otherwise.
     */
    bool empty() const;

    /**
     * @brief Returns the number of requests currently in the queue.
     * @return Queue size.
     */
    size_t size() const;

private:
    /** @brief Internal FIFO queue storing Request objects. */
    std::queue<Request> q;
};