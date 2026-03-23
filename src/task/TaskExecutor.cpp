
#include "TaskExecutor.h"
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

ExecutionResult TaskExecutor::execute(const Task& task, const std::filesystem::path& results_folder) {
    spdlog::info("Выполнение задания {} типа {}", task.task_id, static_cast<int>(task.type));
    
    switch (task.type) {
        case TaskType::ExecuteCommand:
            return executeCommand(task, results_folder);
        case TaskType::RunProgram:
            return runProgram(task, results_folder);
        case TaskType::TransferFile:
            return transferFiles(task, results_folder);
        default:
            return {false, -1, "Неизвестный тип задания", {}};
    }
}

ExecutionResult TaskExecutor::executeCommand(const Task& task, const std::filesystem::path& results_folder) {
    spdlog::info("Выполнение команды: {}", task.command);
    
    int exit_code = std::system(task.command.c_str());
    bool success = (exit_code == 0);
    std::string error = success ? "" : "Команда завершилась с кодом: " + std::to_string(exit_code);
    
    auto new_files = findNewFiles(results_folder, std::chrono::system_clock::now() - task.timeout);
    
    return {success, exit_code, error, new_files};
}

ExecutionResult TaskExecutor::runProgram(const Task& task, const std::filesystem::path& results_folder) {
    spdlog::info("Запуск программы: {} с {} аргументами", task.program_path, task.arguments.size());
    
    int exit_code = -1;
    std::string error;
    auto start_time = std::chrono::system_clock::now();
    
#ifdef _WIN32
    std::string cmd = "\"" + task.program_path + "\"";
    for (const auto& arg : task.arguments) {
        cmd += " \"" + arg + "\"";
    }
    exit_code = std::system(cmd.c_str());
#else
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> args;
        args.push_back(const_cast<char*>(task.program_path.c_str()));
        for (const auto& arg : task.arguments) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);
        execvp(task.program_path.c_str(), args.data());
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        } else {
            error = "Процесс завершился некорректно";
        }
    } else {
        error = "Ошибка создания процесса";
    }
#endif
    
    bool success = (exit_code == 0);
    auto new_files = findNewFiles(results_folder, start_time);
    
    return {success, exit_code, error, new_files};
}

ExecutionResult TaskExecutor::transferFiles(const Task& task, const std::filesystem::path& results_folder) {
    spdlog::info("Передача {} файлов в {}", task.source_files.size(), task.destination_folder);
    
    std::filesystem::create_directories(task.destination_folder);
    std::vector<std::filesystem::path> transferred;
    
    for (const auto& source : task.source_files) {
        std::filesystem::path src(source);
        std::filesystem::path dest = task.destination_folder / src.filename();
        
        try {
            std::filesystem::copy(src, dest, std::filesystem::copy_options::overwrite_existing);
            transferred.push_back(dest);
            spdlog::info("Скопирован {} в {}", source, dest.string());
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Не удалось скопировать {}: {}", source, e.what());
            return {false, -1, e.what(), {}};
        }
    }
    
    return {true, 0, "", transferred};
}

std::vector<std::filesystem::path> TaskExecutor::findNewFiles(const std::filesystem::path& folder, 
                                                                std::chrono::system_clock::time_point since) {
    std::vector<std::filesystem::path> new_files;
    
    if (!std::filesystem::exists(folder)) return new_files;
    
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (std::filesystem::is_regular_file(entry)) {
            auto write_time = std::filesystem::last_write_time(entry);
            if (write_time > since) {
                new_files.push_back(entry.path());
            }
        }
    }
    
    return new_files;
}