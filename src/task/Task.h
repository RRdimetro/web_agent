
#pragma once
#include <string>
#include <vector>
#include <chrono>

enum class TaskType {
    None,           // нет задания
    ExecuteCommand, // выполнить команду
    RunProgram,     // запустить программу
    TransferFile    // передать файл
};

struct Task {
    TaskType type = TaskType::None;              // тип задания
    std::string session_id;                      // идентификатор сессии
    std::string task_id;                         // идентификатор задания
    std::string command;                         // команда для выполнения
    std::string program_path;                    // путь к программе
    std::vector<std::string> arguments;          // аргументы программы
    std::vector<std::string> source_files;       // исходные файлы для передачи
    std::string destination_folder;              // папка назначения
    std::chrono::seconds timeout{300};           // таймаут выполнения
    
    bool isValid() const;                        // проверить корректность
    static Task fromJson(const std::string& json); // создать из JSON
};