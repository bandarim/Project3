/**
 * @file Colors.h
 * @brief Defines ANSI color escape codes for colored terminal output.
 *
 * This header provides a small collection of ANSI escape sequences
 * that can be used to colorize console output. These colors are used
 * to visually distinguish different events in the load balancer
 * simulation (such as accepted requests, blocked requests, scaling
 * actions, and completed jobs).
 *
 * Color output is intended for terminal display only and should not
 * be used in log files.
 */

#pragma once

/**
 * @namespace Color
 * @brief ANSI color codes for terminal output.
 *
 * This namespace contains string constants representing ANSI escape
 * sequences. Each constant can be inserted into output streams
 * (e.g., std::cout) to change the color of subsequent text.
 * The RESET code should always be used after colored output to
 * restore the terminal to its default color.
 */
namespace Color {

    /** @brief Reset terminal color to default. */
    constexpr const char* RESET   = "\033[0m";

    /** @brief Red text (used for blocked or error events). */
    constexpr const char* RED     = "\033[31m";

    /** @brief Green text (used for accepted or successful events). */
    constexpr const char* GREEN   = "\033[32m";

    /** @brief Yellow text (used for scale-up or warning events). */
    constexpr const char* YELLOW  = "\033[33m";

    /** @brief Blue text (used for request assignment or processing events). */
    constexpr const char* BLUE    = "\033[34m";

    /** @brief Magenta text (used for completed request events). */
    constexpr const char* MAGENTA = "\033[35m";

    /** @brief Cyan text (used for informational or summary output). */
    constexpr const char* CYAN    = "\033[36m";
}