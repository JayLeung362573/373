#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
#include <filesystem>
/**
 * spdlog::basic_logger_mt("file_logger", "logs/test_log.txt")
 *   - Create a thread-safe file logger named "file_logger" that writes to logs/test_log.txt.
 * 
 * spdlog::set_default_logger(file_logger)
 *   - Make the created logger the default so free functions like spdlog::info() use it.
 * 
 * spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v")
 *   - Set the global log message format (timestamp, level, message).
 * 
 * spdlog::info("...") / spdlog::warn("...") / spdlog::error("...")
 *   - Log messages at info, warning and error severity via the default logger.
 * 
 * file_logger->info("...")
 *   - Log directly through the specific file_logger instance (bypasses default lookup).
 * 
 * file_logger->flush()
 *   - Force any buffered log messages to be written to disk immediately.
 * 
 * spdlog::shutdown()
 *   - Flush and close all registered loggers and release spdlog resources.
 * 
 * spdlog::spdlog_ex
 *   - Exception type thrown by spdlog on errors; caught below to report init failures.
 */

int main() {
    try {
        // Ensure logs directory exists relative to current working directory
        std::filesystem::create_directories("logs");

        // Create a file logger and set it as the default logger so spdlog::info() also writes to file
        auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/test_log.txt");
        spdlog::set_default_logger(file_logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

        spdlog::info("This is an info message");
        spdlog::warn("This is a warning message");
        spdlog::error("This is an error message");

        // Write explicitly with the file logger too
        file_logger->info("This log goes into a file!");

        // Flush and shutdown so file is closed and visible immediately
        file_logger->flush();
        spdlog::shutdown();

        std::cout << "Wrote logs/test_log.txt" << std::endl;
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception &ex) {
        std::cerr << "Unexpected error: " << ex.what() << std::endl;
        return 2;
    }
    return 0;
}
