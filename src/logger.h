#pragma once
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

class Logger {
private:
    std::shared_ptr<spdlog::logger> logger;

public:
    Logger(const std::string& filename = "app.log");
    ~Logger();

    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void debug(const char* format, ...);

    void setLevel(/* spdlog::level::level_enum */ int level);
};