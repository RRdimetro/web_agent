
#include "Task.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool Task::isValid() const {
    if (type == TaskType::None || session_id.empty() || task_id.empty()) return false;
    
    switch (type) {
        case TaskType::ExecuteCommand:
            return !command.empty();
        case TaskType::RunProgram:
            return !program_path.empty();
        case TaskType::TransferFile:
            return !source_files.empty() && !destination_folder.empty();
        default:
            return false;
    }
}

Task Task::fromJson(const std::string& json_str) {
    Task task;
    auto j = json::parse(json_str);
    
    task.session_id = j.value("session_id", "");
    task.task_id = j.value("task_id", "");
    
    std::string type_str = j.value("type", "none");
    if (type_str == "execute_command") {
        task.type = TaskType::ExecuteCommand;
        task.command = j.value("command", "");
        task.timeout = std::chrono::seconds(j.value("timeout", 300));
    } else if (type_str == "run_program") {
        task.type = TaskType::RunProgram;
        task.program_path = j.value("program", "");
        if (j.contains("arguments")) {
            task.arguments = j["arguments"].get<std::vector<std::string>>();
        }
        task.timeout = std::chrono::seconds(j.value("timeout", 300));
    } else if (type_str == "transfer_file") {
        task.type = TaskType::TransferFile;
        task.source_files = j.value("source_files", std::vector<std::string>{});
        task.destination_folder = j.value("destination", "");
    }
    
    return task;
}