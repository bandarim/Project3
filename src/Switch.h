/**
 * @file Switch.h
 * @brief Defines a Switch for routing requests to multiple load balancers.
 *
 * The Switch acts as a higher-level routing component that examines
 * incoming requests and forwards them to different LoadBalancer
 * instances based on job type.
 */

#pragma once
#include "LoadBalancer.h"

/**
 * @class Switch
 * @brief Routes requests to different load balancers based on job type.
 *
 * This class is used to implement the bonus requirement of the project,
 * allowing streaming and processing jobs to be handled by separate
 * load balancers and server pools.
 */
class Switch {
public:
    /**
     * @brief Constructs a Switch with two load balancers.
     *
     * @param streamingLB Pointer to the load balancer handling streaming jobs.
     * @param processingLB Pointer to the load balancer handling processing jobs.
     * @throws std::runtime_error if either pointer is null.
     */
    Switch(LoadBalancer* streamingLB, LoadBalancer* processingLB);

    /**
     * @brief Routes a request to the appropriate load balancer.
     *
     * Routing is determined by the job type stored in the Request:
     * - 'S' jobs are sent to the streaming load balancer.
     * - 'P' jobs are sent to the processing load balancer.
     * - Unknown job types default to the processing load balancer.
     *
     * @param r The request to route.
     * @param currentCycle Current simulation clock cycle.
     */
    void route(Request r, int currentCycle);

private:
    /** @brief Load balancer dedicated to streaming requests. */
    LoadBalancer* stream;

    /** @brief Load balancer dedicated to processing requests. */
    LoadBalancer* proc;
};