
#pragma once
#include "Task.h"
#include "TaskExecutor.h"
#include <string>
#include <filesystem>
#include <mutex>

class Logger {
public:
    explicit Logger(const std::filesystem::path& log_file);
    
    void logTask(const Task& task, const ExecutionResult& result);   // записать выполнение задания
    void logError(const std::string& task_id, const std::string& error); // записать ошибку
    void logInfo(const std::string& message);                        // записать информацию
    void logWarning(const std::string& message);                     // записать предупреждение
    
private:
    std::filesystem::path log_file_;    // путь к файлу журнала
    std::mutex mutex_;                  // мьютекс для потокобезопасности
    
    void write(const std::string& level, const std::string& message); // записать в журнал
    std::string getTimestamp();                                        // получить временную метку
};