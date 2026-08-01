#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>
#include <cstdarg>
#include <iostream>

Logger::Logger(const std::string& filename) {
    try {
        std::vector<spdlog::sink_ptr> sinks;

        // File sink
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        sinks.push_back(file_sink);

        // Console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(console_sink);

        logger = std::make_shared<spdlog::logger>("app", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::err);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

Logger::~Logger() {
    if (logger) {
        logger->flush();
    }
    spdlog::drop_all();
}

void Logger::info(const std::string& message) {
    if (logger) logger->info(message);
}

void Logger::warn(const std::string& message) {
    if (logger) logger->warn(message);
}

void Logger::error(const std::string& message) {
    if (logger) logger->error(message);
}

void Logger::debug(const std::string& message) {
    if (logger) logger->debug(message);
}

void Logger::info(const char* format, ...) {
    if (!logger) return;
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    logger->info(buffer);
}

void Logger::warn(const char* format, ...) {
    if (!logger) return;
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    logger->warn(buffer);
}

void Logger::error(const char* format, ...) {
    if (!logger) return;
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    logger->error(buffer);
}

void Logger::debug(const char* format, ...) {
    if (!logger) return;
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    logger->debug(buffer);
}

void Logger::setLevel(int level) {
    if (logger) {
        logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }
}