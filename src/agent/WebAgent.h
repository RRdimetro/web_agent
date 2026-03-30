#pragma once
#include "Config.h"
#include "HttpClient.h"
#include "Task.h"
#include "TaskExecutor.h"
#include "Logger.h"
#include <atomic>
#include <thread>
#include <optional>

class WebAgent {
public:
    explicit WebAgent(const Config& config);
    ~WebAgent();

    void start();
    void stop();
    bool isRunning() const { return running_; }

private:
    Config config_;
    HttpClient http_client_;
    std::unique_ptr<Logger> logger_;

    std::atomic<bool> running_{false};
    std::thread worker_thread_;

    std::string session_id_;
    std::chrono::seconds current_interval_;
    int consecutive_failures_{0};

    void workerLoop();
    bool registerAgent();
    std::optional<Task> fetchTask();
    bool sendResult(const Task& task, const ExecutionResult& result);
    void handleNetworkError();
};
