//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_LOGGING_H
#define SERVER_LOGGING_H


#include <string_view>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

constexpr char LOGGER_NAME[] = "logger";

#endif //SERVER_LOGGING_H
