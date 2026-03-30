#pragma once
#include <string>
#include <vector>
#include <chrono>

enum class TaskType {
    None,
    ExecuteCommand,
    RunProgram,
    TransferFile
};

struct Task {
    TaskType type = TaskType::None;
    std::string session_id;
    std::string task_id;
    std::string command;
    std::string program_path;
    std::vector<std::string> arguments;
    std::vector<std::string> source_files;
    std::string destination_folder;
    std::chrono::seconds timeout{300};

    bool isValid() const;
    static Task fromJson(const std::string& json);
};
