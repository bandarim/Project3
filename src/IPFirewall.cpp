/**
 * @file IPFirewall.cpp
 * @brief Implements IP-based firewall functionality.
 *
 * This file provides the implementation of the IPFirewall class, including
 * conversion of IPv4 addresses to numeric form, storage of blocked IP ranges,
 * and logic for determining whether a given IP address should be blocked.
 *
 * IP addresses are validated and converted to 32-bit integers to allow
 * efficient range comparisons.
 */

#include "IPFirewall.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

/**
 * @brief Removes leading and trailing whitespace from a string.
 * @param s Input string.
 * @return A trimmed copy of the input string.
 */
static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

/**
 * @brief Converts a dotted IPv4 address string to a 32-bit unsigned integer.
 *
 * The input string must contain exactly four numeric octets separated by dots,
 * with each octet in the range 0–255. Any malformed input will result in an
 * exception being thrown.
 *
 * @param ip IPv4 address in dotted-decimal notation.
 * @return 32-bit unsigned integer representation of the IP address.
 * @throws std::runtime_error if the IP address format is invalid.
 */
uint32_t IPFirewall::ipToUint(const std::string& ip) {
    std::istringstream iss(ip);
    std::string part;
    uint32_t result = 0;

    for (int i = 0; i < 4; i++) {
        if (!std::getline(iss, part, '.')) {
            throw std::runtime_error("Invalid IP (missing octets): " + ip);
        }
        part = trim(part);
        if (part.empty()) {
            throw std::runtime_error("Invalid IP (empty octet): " + ip);
        }

        // Ensure numeric
        for (char c : part) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                throw std::runtime_error("Invalid IP (non-numeric octet): " + ip);
            }
        }

        int octet = std::stoi(part);
        if (octet < 0 || octet > 255) {
            throw std::runtime_error("Invalid IP (octet out of range): " + ip);
        }

        result = (result << 8) | static_cast<uint32_t>(octet);
    }

    // There should be no extra data
    if (std::getline(iss, part)) {
        // If there's anything non-whitespace left, reject
        if (!trim(part).empty()) {
            throw std::runtime_error("Invalid IP (extra characters): " + ip);
        }
    }

    return result;
}

/**
 * @brief Adds a blocked IP range to the firewall.
 *
 * The start and end IP addresses are converted to numeric form. If the
 * addresses are provided out of order, they are normalized so that the
 * start value is always less than or equal to the end value.
 *
 * @param startIP Starting IP address of the blocked range.
 * @param endIP Ending IP address of the blocked range.
 * @throws std::runtime_error if either IP address is invalid.
 */
void IPFirewall::addBlockedRange(const std::string& startIP, const std::string& endIP) {
    uint32_t s = ipToUint(startIP);
    uint32_t e = ipToUint(endIP);

    IPRange r;
    r.start = std::min(s, e);
    r.end   = std::max(s, e);

    blockedRanges.push_back(r);
}

/**
 * @brief Checks whether a given IP address is blocked by the firewall.
 *
 * The IP address is converted to a numeric value and compared against
 * all configured blocked ranges.
 *
 * @param ip Source IP address to check.
 * @return True if the IP address falls within any blocked range; false otherwise.
 * @throws std::runtime_error if the IP address format is invalid.
 */
bool IPFirewall::isBlocked(const std::string& ip) const {
    uint32_t val = ipToUint(ip);
    for (const auto& r : blockedRanges) {
        if (val >= r.start && val <= r.end) return true;
    }
    return false;
}