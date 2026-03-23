
#include "WebAgent.h"
#include "TaskExecutor.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebAgent::WebAgent(const Config& config) 
    : config_(config)
    , http_client_(config.timeout)
    , current_interval_(config.poll_interval) {
    
    logger_ = std::make_unique<Logger>(config.log_file);
    http_client_.setVerifySsl(config.server_url.find("https") == 0);
    
    spdlog::info("WebAgent инициализирован с UID: {}", config_.uid);
}

WebAgent::~WebAgent() {
    stop();
}

void WebAgent::start() {
    if (running_) return;
    running_ = true;
    worker_thread_ = std::thread(&WebAgent::workerLoop, this);
    spdlog::info("WebAgent запущен");
}

void WebAgent::stop() {
    if (!running_) return;
    running_ = false;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    spdlog::info("WebAgent остановлен");
}

void WebAgent::workerLoop() {
    // Регистрация агента
    if (!registerAgent()) {
        spdlog::error("Не удалось зарегистрировать агент");
        handleNetworkError();
    }
    
    while (running_) {
        try {
            auto task_opt = fetchTask();
            
            if (task_opt.has_value()) {
                Task task = task_opt.value();
                spdlog::info("Получено задание: {}", task.task_id);
                
                ExecutionResult result = TaskExecutor::execute(task, config_.results_folder);
                
                if (!sendResult(task, result)) {
                    spdlog::error("Не удалось отправить результаты для задания {}", task.task_id);
                }
                
                if (result.success) {
                    spdlog::info("Задание {} выполнено успешно", task.task_id);
                    logger_->logTask(task, result);
                } else {
                    spdlog::error("Задание {} не выполнено: {}", task.task_id, result.error_message);
                    logger_->logError(task.task_id, result.error_message);
                }
                
                // Сброс интервала при успехе
                current_interval_ = config_.poll_interval;
                consecutive_failures_ = 0;
            } else {
                spdlog::debug("Нет заданий, ожидание {} секунд", current_interval_.count());
                std::this_thread::sleep_for(current_interval_);
            }
            
        } catch (const std::exception& e) {
            spdlog::error("Исключение в цикле работы: {}", e.what());
            handleNetworkError();
        }
    }
}

bool WebAgent::registerAgent() {
    spdlog::info("Регистрация агента с UID: {}", config_.uid);
    
    json request;
    request["uid"] = config_.uid;
    request["version"] = "1.0.0";
    
    std::string url = config_.server_url + "/api/register";
    auto response = http_client_.post(url, request.dump());
    
    if (!response.success) {
        spdlog::error("Ошибка регистрации: {}", response.error);
        return false;
    }
    
    try {
        auto response_json = json::parse(response.body);
        if (response_json.contains("session_id")) {
            session_id_ = response_json["session_id"];
            spdlog::info("Регистрация успешна, ID сессии: {}", session_id_);
            return true;
        } else {
            spdlog::error("Неверный ответ регистрации");
            return false;
        }
    } catch (const json::exception& e) {
        spdlog::error("Не удалось разобрать ответ регистрации: {}", e.what());
        return false;
    }
}

std::optional<Task> WebAgent::fetchTask() {
    if (session_id_.empty()) {
        spdlog::error("Нет ID сессии, невозможно получить задание");
        return std::nullopt;
    }
    
    std::string url = config_.server_url + "/api/task";
    auto response = http_client_.get(url, {{"session_id", session_id_}});
    
    if (!response.success) {
        if (response.status_code == 404) {
            // Нет заданий
            return std::nullopt;
        }
        throw std::runtime_error("Не удалось получить задание: " + response.error);
    }
    
    try {
        auto task_json = json::parse(response.body);
        if (task_json.contains("task_id") && task_json["task_id"] != "null") {
            return Task::fromJson(response.body);
        }
        return std::nullopt;
    } catch (const json::exception& e) {
        spdlog::error("Не удалось разобрать задание: {}", e.what());
        return std::nullopt;
    }
}

bool WebAgent::sendResult(const Task& task, const ExecutionResult& result) {
    std::string url = config_.server_url + "/api/result";
    
    std::vector<cpr::Pair> fields = {
        {"session_id", session_id_},
        {"task_id", task.task_id},
        {"success", result.success ? "true" : "false"},
        {"exit_code", std::to_string(result.exit_code)},
        {"error_message", result.error_message}
    };
    
    std::vector<cpr::File> files;
    for (const auto& file : result.output_files) {
        files.emplace_back("files", file.string());
    }
    
    auto response = http_client_.postFiles(url, fields, files);
    
    if (!response.success) {
        spdlog::error("Не удалось отправить результаты: {}", response.error);
        return false;
    }
    
    spdlog::info("Результаты отправлены для задания {}", task.task_id);
    return true;
}

void WebAgent::handleNetworkError() {
    consecutive_failures_++;
    
    // Экспоненциальная задержка
    auto backoff = config_.poll_interval * (1 << std::min(consecutive_failures_, 5));
    if (backoff > config_.max_poll_interval) {
        backoff = config_.max_poll_interval;
    }
    
    current_interval_ = backoff;
    spdlog::warn("Сетевая ошибка, увеличение интервала до {} секунд", current_interval_.count());
    
    std::this_thread::sleep_for(current_interval_);
    
    // Повторная регистрация
    registerAgent();
}