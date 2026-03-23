#pragma once
#include <string>
#include <filesystem>
#include <chrono>

struct Config {
    std::string uid;                         // уникальный идентификатор агента
    std::string server_url;                  // адрес сервера
    std::filesystem::path tasks_folder;      // папка для хранения задач
    std::filesystem::path results_folder;    // папка для результатов
    std::filesystem::path log_file;          // файл журнала
    std::chrono::seconds poll_interval{5};   // интервал опроса
    std::chrono::seconds max_poll_interval{300}; // максимальный интервал
    std::chrono::seconds timeout{30};        // таймаут запросов
    int max_retries{3};                      // максимум попыток
    std::chrono::seconds retry_delay{5};     // задержка между попытками
    
    static Config load(const std::string& path);   // загрузить из файла
    bool validate() const;                         // проверить настройки
    
private:
    void setDefaults();                            // установить значения по умолчанию
};