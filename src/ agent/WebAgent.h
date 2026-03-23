
#pragma once
#include "Config.h"
#include "HttpClient.h"
#include "Task.h"
#include "Logger.h"
#include <atomic>
#include <thread>
#include <optional>

class WebAgent {
public:
    explicit WebAgent(const Config& config);
    ~WebAgent();
    
    void start();           // запустить агента
    void stop();            // остановить агента
    bool isRunning() const { return running_; }
    
private:
    Config config_;                         // настройки
    HttpClient http_client_;                // HTTP клиент
    std::unique_ptr<Logger> logger_;        // журнал
    
    std::atomic<bool> running_{false};      // флаг работы
    std::thread worker_thread_;             // поток работы
    
    std::string session_id_;                // идентификатор сессии
    std::chrono::seconds current_interval_; // текущий интервал опроса
    int consecutive_failures_{0};           // количество ошибок подряд
    
    void workerLoop();                      // основной цикл
    bool registerAgent();                   // регистрация на сервере
    std::optional<Task> fetchTask();        // получить задание
    bool sendResult(const Task& task, const ExecutionResult& result); // отправить результат
    void handleNetworkError();              // обработка сетевых ошибок
};