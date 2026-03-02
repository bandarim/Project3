/**
 * @file IPFirewall.h
 * @brief Defines the IPFirewall class for blocking requests based on IP ranges.
 *
 * The IPFirewall is used by the LoadBalancer to reject incoming requests
 * whose source IP addresses fall within configured blocked ranges. These
 * ranges are typically loaded from a configuration file and represent
 * firewall rules or denial-of-service prevention mechanisms.
 */

#pragma once
#include <string>
#include <vector>
#include "IPRange.h"

/**
 * @class IPFirewall
 * @brief Implements an IP-based firewall for request filtering.
 *
 * This class maintains a collection of blocked IP address ranges and
 * provides functionality to check whether a given IP address should
 * be rejected. IP addresses are internally converted to 32-bit integers
 * to allow efficient range comparisons.
 */
class IPFirewall {
public:
    /**
     * @brief Adds a blocked IP address range to the firewall.
     *
     * Requests with source IP addresses that fall within this range
     * (inclusive) will be blocked.
     *
     * @param startIP Starting IP address of the blocked range.
     * @param endIP Ending IP address of the blocked range.
     */
    void addBlockedRange(const std::string& startIP, const std::string& endIP);

    /**
     * @brief Determines whether a given IP address is blocked.
     *
     * The IP address is converted to a numeric form and compared against
     * all configured blocked ranges.
     *
     * @param ip Source IP address to check.
     * @return True if the IP address is blocked; false otherwise.
     */
    bool isBlocked(const std::string& ip) const;

    /**
     * @brief Converts a dotted IPv4 address string to a 32-bit integer.
     *
     * This utility function allows IP addresses to be compared efficiently
     * using numeric range comparisons.
     *
     * @param ip IPv4 address in dotted-decimal notation (e.g., "192.168.1.1").
     * @return 32-bit unsigned integer representation of the IP address.
     */
    static uint32_t ipToUint(const std::string& ip);

private:
    /**
     * @brief Collection of blocked IP address ranges.
     *
     * Each range represents a contiguous set of IP addresses that should
     * be rejected by the firewall.
     */
    std::vector<IPRange> blockedRanges;
};