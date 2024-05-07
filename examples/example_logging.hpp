#pragma once

#include <dv_utilities.hpp>

#define LIBNETWRK_INFO(component, frmt, ...)        \
    dvsku::dv_util_log::log_message(dvsku::dv_util_log::level::informational, component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_WARNING(component, frmt, ...)     \
    dvsku::dv_util_log::log_message(dvsku::dv_util_log::level::warning,       component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_ERROR(component, frmt, ...)       \
    dvsku::dv_util_log::log_message(dvsku::dv_util_log::level::error,         component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_DEBUG(component, frmt, ...)       \
    dvsku::dv_util_log::log_message(dvsku::dv_util_log::level::debug,         component, frmt, ##__VA_ARGS__)

#define LIBNETWRK_VERBOSE(component, frmt, ...)     \
    dvsku::dv_util_log::log_message(dvsku::dv_util_log::level::verbose,       component, frmt, ##__VA_ARGS__)

#include <libnetwrk.hpp>
