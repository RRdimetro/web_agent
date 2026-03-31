#include "TaskExecutor.h"
#include "Config.h"
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

TaskExecutor::TaskExecutor(std::shared_ptr<Config> config) : config_(config) {}

ExecutionResult TaskExecutor::executeWithConfig(const Task& task, const std::filesystem::path& results_folder) {
    spdlog::info("Выполнение задания: code={}, session={}", task.task_code, task.session_id);

    switch (task.type) {
        case TaskType::ExecuteCommand:
            return executeCommand(task, results_folder);
        case TaskType::RunProgram:
            return runProgram(task, results_folder);
        case TaskType::TransferFile:
            return transferFiles(task, results_folder);
        case TaskType::ChangeConfig:
            return changeConfig(task);
        case TaskType::ChangeTimeout:
            return changeTimeout(task);
        default:
            return {false, -1, "Неизвестный тип задания", {}};
    }
}

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

ExecutionResult TaskExecutor::changeConfig(const Task& task) {
    // CONF: изменить конфигурацию
    // options содержит JSON {"key":"...","value":"..."}
    // Ключи: poll_interval_sec, task_timeout, max_poll_interval, max_retries, retry_delay
    spdlog::info("Изменение конфигурации: {}", task.options);

    try {
        auto config_json = json::parse(task.options);
        std::string key = config_json.value("key", "");
        std::string value = config_json.value("value", "");

        if (!config_) {
            return {false, -1, "Config недоступен", {}};
        }

        if (key == "poll_interval_sec") {
            int new_value = std::stoi(value);
            config_->poll_interval = std::chrono::seconds(new_value);
            spdlog::info("poll_interval_sec изменён на {}", new_value);
            return {true, 0, "poll_interval_sec = " + std::to_string(new_value), {}};
        } else if (key == "task_timeout") {
            int new_value = std::stoi(value);
            config_->timeout = std::chrono::seconds(new_value);
            spdlog::info("task_timeout изменён на {}", new_value);
            return {true, 0, "task_timeout = " + std::to_string(new_value), {}};
        } else if (key == "max_poll_interval") {
            int new_value = std::stoi(value);
            config_->max_poll_interval = std::chrono::seconds(new_value);
            spdlog::info("max_poll_interval изменён на {}", new_value);
            return {true, 0, "max_poll_interval = " + std::to_string(new_value), {}};
        } else if (key == "max_retries") {
            int new_value = std::stoi(value);
            config_->max_retries = new_value;
            spdlog::info("max_retries изменён на {}", new_value);
            return {true, 0, "max_retries = " + std::to_string(new_value), {}};
        } else if (key == "retry_delay") {
            int new_value = std::stoi(value);
            config_->retry_delay = std::chrono::seconds(new_value);
            spdlog::info("retry_delay изменён на {}", new_value);
            return {true, 0, "retry_delay = " + std::to_string(new_value), {}};
        } else {
            return {false, -1, "Неизвестный ключ конфигурации: " + key, {}};
        }
    } catch (const std::exception& e) {
        return {false, -1, "Ошибка парсинга JSON: " + std::string(e.what()), {}};
    }
}

ExecutionResult TaskExecutor::changeTimeout(const Task& task) {
    // TIMEOUT: изменить интервал опроса сервера (poll interval)
    // options содержит JSON {"key":"poll_interval_sec","value":"30"} или {"interval":"15"}
    spdlog::info("Изменение poll interval: {}", task.options);

    try {
        auto timeout_json = json::parse(task.options);
        
        // Поддержка двух форматов
        if (timeout_json.contains("key") && timeout_json.contains("value")) {
            // Формат 1: {"key":"poll_interval_sec","value":"30"}
            std::string key = timeout_json.value("key", "");
            std::string value = timeout_json.value("value", "");

            if (key == "poll_interval_sec" && config_) {
                int new_interval = std::stoi(value);
                config_->poll_interval = std::chrono::seconds(new_interval);
                spdlog::info("Poll interval изменён на {} секунд", new_interval);
                return {true, 0, "Poll interval изменён на " + std::to_string(new_interval) + " секунд", {}};
            } else {
                return {false, -1, "Неизвестный ключ: " + key, {}};
            }
        } else if (timeout_json.contains("interval")) {
            // Формат 2: {"interval":"15"}
            int new_interval = std::stoi(timeout_json["interval"].get<std::string>());
            if (config_) {
                config_->poll_interval = std::chrono::seconds(new_interval);
                spdlog::info("Poll interval изменён на {} секунд", new_interval);
                return {true, 0, "Poll interval изменён на " + std::to_string(new_interval) + " секунд", {}};
            } else {
                return {false, -1, "Config недоступен", {}};
            }
        } else {
            return {false, -1, "Неверный формат JSON", {}};
        }
    } catch (const std::exception& e) {
        return {false, -1, "Ошибка парсинга JSON: " + std::string(e.what()), {}};
    }
}

std::vector<std::filesystem::path> TaskExecutor::findNewFiles(const std::filesystem::path& folder,
                                                                std::chrono::system_clock::time_point since) {
    std::vector<std::filesystem::path> new_files;

    if (!std::filesystem::exists(folder)) return new_files;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (std::filesystem::is_regular_file(entry)) {
            auto write_time = std::filesystem::last_write_time(entry);
            auto write_time_sys = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                write_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            if (write_time_sys > since) {
                new_files.push_back(entry.path());
            }
        }
    }

    return new_files;
}
