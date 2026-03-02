/**
 * @file RequestQueue.cpp
 * @brief Implements a FIFO queue for managing incoming requests.
 *
 * This file contains the implementation of the RequestQueue class, which
 * provides a simple wrapper around a standard queue to store Request
 * objects awaiting processing by the load balancer.
 */

#include "RequestQueue.h"
#include <stdexcept>

/**
 * @brief Adds a request to the back of the queue.
 * @param r The Request to enqueue.
 */
void RequestQueue::enqueue(const Request& r) {
    q.push(r);
}

/**
 * @brief Removes and returns the request at the front of the queue.
 *
 * Requests are returned in first-in, first-out (FIFO) order.
 *
 * @return The next Request in the queue.
 * @throws std::runtime_error if the queue is empty.
 */
Request RequestQueue::dequeue() {
    if (q.empty()) {
        throw std::runtime_error("The queue is empty");
    }
    Request front = q.front();
    q.pop();
    return front;
}

/**
 * @brief Checks whether the request queue is empty.
 * @return True if the queue contains no requests; false otherwise.
 */
bool RequestQueue::empty() const {
    return q.empty();
}

/**
 * @brief Returns the number of requests currently in the queue.
 * @return Queue size.
 */
size_t RequestQueue::size() const {
    return q.size();
}