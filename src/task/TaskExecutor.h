
#pragma once
#include "Task.h"
#include <filesystem>
#include <vector>

struct ExecutionResult {
    bool success;                               // успех выполнения
    int exit_code;                              // код возврата
    std::string error_message;                  // сообщение об ошибке
    std::vector<std::filesystem::path> output_files; // выходные файлы
};

class TaskExecutor {
public:
    // Выполнить задание
    static ExecutionResult execute(const Task& task, const std::filesystem::path& results_folder);
    
private:
    static ExecutionResult executeCommand(const Task& task, const std::filesystem::path& results_folder);
    static ExecutionResult runProgram(const Task& task, const std::filesystem::path& results_folder);
    static ExecutionResult transferFiles(const Task& task, const std::filesystem::path& results_folder);
    
    static std::vector<std::filesystem::path> findNewFiles(const std::filesystem::path& folder, 
                                                            std::chrono::system_clock::time_point since);
};