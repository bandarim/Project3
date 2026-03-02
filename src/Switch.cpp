/**
 * @file Switch.cpp
 * @brief Implements the Switch class for routing requests by job type.
 *
 * The Switch acts as a higher-level routing component that examines
 * incoming requests and forwards them to the appropriate LoadBalancer
 * based on job type (Streaming or Processing).
 */

#include "Switch.h"
#include <stdexcept>

/**
 * @brief Constructs a Switch that routes requests to two load balancers.
 *
 * The Switch requires one load balancer dedicated to streaming jobs
 * and another dedicated to processing jobs.
 *
 * @param streamingLB Pointer to the load balancer handling streaming requests.
 * @param processingLB Pointer to the load balancer handling processing requests.
 * @throws std::runtime_error if either load balancer pointer is null.
 */
Switch::Switch(LoadBalancer* streamingLB, LoadBalancer* processingLB)
    : stream(streamingLB), proc(processingLB) {

    if (!stream || !proc) {
        throw std::runtime_error("Switch can't have null LBs");
    }
}

/**
 * @brief Routes a request to the appropriate load balancer.
 *
 * Routing rules:
 * - 'S' (Streaming) requests are sent to the streaming load balancer.
 * - 'P' (Processing) requests are sent to the processing load balancer.
 * - Any unknown job type defaults to the processing load balancer.
 *
 * @param r The request to route.
 * @param currentCycle Current simulation clock cycle.
 */
void Switch::route(Request r, int currentCycle) {
    if (r.jobType == 'S') {
        stream->acceptRequest(r, currentCycle);
    } else if (r.jobType == 'P') {
        proc->acceptRequest(r, currentCycle);
    } else {
        proc->acceptRequest(r, currentCycle);
    }
}