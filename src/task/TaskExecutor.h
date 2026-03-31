#pragma once
#include "Task.h"
#include <filesystem>
#include <vector>
#include <memory>

struct Config;

struct ExecutionResult {
    bool success;
    int exit_code;
    std::string error_message;
    std::vector<std::filesystem::path> output_files;
};

class TaskExecutor {
public:
    TaskExecutor(std::shared_ptr<Config> config);
    
    static ExecutionResult execute(const Task& task, const std::filesystem::path& results_folder);
    ExecutionResult executeWithConfig(const Task& task, const std::filesystem::path& results_folder);

private:
    std::shared_ptr<Config> config_;
    
    static ExecutionResult executeCommand(const Task& task, const std::filesystem::path& results_folder);
    static ExecutionResult runProgram(const Task& task, const std::filesystem::path& results_folder);
    static ExecutionResult transferFiles(const Task& task, const std::filesystem::path& results_folder);
    ExecutionResult changeConfig(const Task& task);
    ExecutionResult changeTimeout(const Task& task);

    static std::vector<std::filesystem::path> findNewFiles(const std::filesystem::path& folder,
                                                            std::chrono::system_clock::time_point since);
};
