#pragma once
#include "Task.h"
#include <string>
#include <filesystem>
#include <mutex>

struct ExecutionResult;

class Logger {
public:
    explicit Logger(const std::filesystem::path& log_file);

    void logTask(const Task& task, const ExecutionResult& result);
    void logError(const std::string& task_id, const std::string& error);
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);

private:
    std::filesystem::path log_file_;
    std::mutex mutex_;

    void write(const std::string& level, const std::string& message);
    std::string getTimestamp();
};
