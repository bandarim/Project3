/**
 * @file IPRange.h
 * @brief Defines a simple structure representing a range of IP addresses.
 *
 * This structure is used by the IPFirewall to store blocked IP address
 * ranges in numeric (32-bit) form for efficient comparison.
 */

#pragma once
#include <cstdint>

/**
 * @struct IPRange
 * @brief Represents an inclusive range of IPv4 addresses.
 *
 * IP addresses are stored as 32-bit unsigned integers, allowing
 * fast range checks when determining whether an incoming request
 * should be blocked by the firewall.
 */
struct IPRange {

    /** @brief Numeric value of the starting IP address in the range. */
    uint32_t start = 0;

    /** @brief Numeric value of the ending IP address in the range. */
    uint32_t end = 0;
};