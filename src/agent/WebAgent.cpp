#include "WebAgent.h"
#include "TaskExecutor.h"
#include "HttpClient.h"
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

    // Создаём копию конфига для TaskExecutor
    executor_config_ = std::make_shared<Config>(config);
    task_executor_ = std::make_unique<TaskExecutor>(executor_config_);

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
    // Если есть access_code — используем его, иначе регистрируемся
    if (config_.access_code.empty()) {
        if (!registerAgent()) {
            spdlog::error("Не удалось зарегистрировать агент");
            handleNetworkError();
        }
    } else {
        spdlog::info("Используем существующий access_code");
        session_id_ = config_.access_code;
    }

    while (running_) {
        try {
            auto task_opt = fetchTask();

            if (task_opt.has_value()) {
                Task task = task_opt.value();
                spdlog::info("Получено задание: {}", task.task_code);

                ExecutionResult result = task_executor_->executeWithConfig(task, config_.results_folder);

                if (!sendResult(task, result)) {
                    spdlog::error("Не удалось отправить результаты для задания {}", task.task_id);
                }

                if (result.success) {
                    spdlog::info("Задание {} выполнено успешно", task.task_code);
                    logger_->logTask(task, result);
                } else {
                    spdlog::error("Задание {} не выполнено: {}", task.task_code, result.error_message);
                    logger_->logError(task.task_id, result.error_message);
                }

                current_interval_ = config_.poll_interval;
                consecutive_failures_ = 0;
            } else {
                spdlog::info("Нет заданий, следующий опрос через {} секунд", current_interval_.count());
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
    request["UID"] = config_.uid;
    request["descr"] = "web-agent";

    std::string url = config_.server_url;
    if (url.back() != '/') url += '/';
    url += "wa_reg/";
    
    auto response = http_client_.post(url, request.dump());

    if (!response.success) {
        spdlog::error("Ошибка регистрации: {}", response.error);
        return false;
    }

    try {
        auto response_json = json::parse(response.body);
        int code = response_json.value("code_response", -1);
        
        if (code == 0) {
            session_id_ = response_json.value("access_code", "");
            if (!session_id_.empty()) {
                spdlog::info("Регистрация успешна, access_code: {}", session_id_);
                return true;
            }
        }
        
        std::string msg = response_json.value("msg", "unknown error");
        spdlog::error("Ошибка регистрации: {} (code {})", msg, code);
        return false;
    } catch (const json::exception& e) {
        spdlog::error("Не удалось разобрать ответ регистрации: {}", e.what());
        return false;
    }
}

std::optional<Task> WebAgent::fetchTask() {
    if (session_id_.empty()) {
        spdlog::error("Нет access_code, невозможно получить задание");
        return std::nullopt;
    }

    json request;
    request["UID"] = config_.uid;
    request["descr"] = "web-agent";
    request["access_code"] = session_id_;

    std::string url = config_.server_url;
    if (url.back() != '/') url += '/';
    url += "wa_task/";
    
    auto response = http_client_.post(url, request.dump());

    if (!response.success) {
        throw std::runtime_error("Не удалось получить задание: " + response.error);
    }

    try {
        auto resp_json = json::parse(response.body);
        
        // Обрабатываем опечатку в имени поля (code_responce вместо code_response)
        int code = -1;
        if (resp_json.contains("code_response")) {
            auto val = resp_json["code_response"];
            if (val.is_string()) code = std::stoi(val.get<std::string>());
            else code = val.get<int>();
        } else if (resp_json.contains("code_responce")) {
            auto val = resp_json["code_responce"];
            if (val.is_string()) code = std::stoi(val.get<std::string>());
            else code = val.get<int>();
        }
        
        if (code == 0) {
            // Нет заданий
            return std::nullopt;
        } else if (code == 1) {
            // Есть задание - парсим его
            std::string task_code = resp_json.value("task_code", "");
            std::string options = resp_json.value("options", "");
            std::string session = resp_json.value("session_id", "");

            Task task;
            task.task_id = task_code;
            task.session_id = session;
            
            // Обработка разных типов задач
            if (task_code == "FILE") {
                // FILE: options содержит JSON {"filename":"..."}
                // Нужно найти файл и отправить его
                task.type = TaskType::TransferFile;
                
                // Парсим JSON для получения имени файла
                try {
                    auto file_json = json::parse(options);
                    std::string filename = file_json.value("filename", "");
                    task.source_files = {filename};
                    task.destination_folder = config_.results_folder;
                    task.command = ""; // Не выполняем как команду
                } catch (...) {
                    // Если не JSON, используем как есть
                    task.source_files = {options};
                    task.destination_folder = config_.results_folder;
                }
            } else if (task_code == "CONF") {
                // CONF: изменить конфигурацию
                // options содержит JSON {"key":"...","value":"..."}
                task.type = TaskType::ChangeConfig;
                task.options = options;
            } else if (task_code == "TIMEOUT") {
                // TIMEOUT: изменить интервал опроса сервера (poll interval)
                // options содержит JSON {"key":"poll_interval_sec","value":"30"}
                task.type = TaskType::ChangeTimeout;
                task.options = options;
            } else if (task_code == "TASK") {
                // TASK: выполнить команду
                // options содержит JSON {"command":"..."}
                task.type = TaskType::ExecuteCommand;
                try {
                    auto task_json = json::parse(options);
                    task.command = task_json.value("command", "");
                } catch (...) {
                    task.command = options;
                }
            } else {
                // По умолчанию - выполнить команду
                task.type = TaskType::ExecuteCommand;
                task.command = options;
            }

            spdlog::info("Получена задача: {}", task_code);
            return task;
        } else {
            std::string msg = resp_json.value("msg", "unknown error");
            spdlog::warn("Код задачи: {}, сообщение: {}", code, msg);
            return std::nullopt;
        }
    } catch (const json::exception& e) {
        spdlog::error("Не удалось разобрать задание: {}", e.what());
        return std::nullopt;
    }
}

bool WebAgent::sendResult(const Task& task, const ExecutionResult& result) {
    std::string url = config_.server_url;
    if (url.back() != '/') url += '/';
    url += "wa_result/";

    json result_json;
    result_json["UID"] = config_.uid;
    result_json["access_code"] = session_id_;
    result_json["message"] = result.error_message;
    result_json["files"] = result.output_files.size();
    result_json["session_id"] = task.session_id;

    std::vector<cpr::Pair> fields = {
        {"result_code", std::to_string(result.exit_code)},
        {"result", result_json.dump()}
    };

    std::vector<FileData> files;
    for (size_t i = 0; i < result.output_files.size(); ++i) {
        std::string field_name = "file" + std::to_string(i + 1);
        files.push_back({field_name, result.output_files[i].string()});
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

    auto backoff = config_.poll_interval * (1 << std::min(consecutive_failures_, 5));
    if (backoff > config_.max_poll_interval) {
        backoff = config_.max_poll_interval;
    }

    current_interval_ = backoff;
    spdlog::warn("Сетевая ошибка, следующий опрос через {} секунд", current_interval_.count());

    std::this_thread::sleep_for(current_interval_);

    // Пробуем перерегистрироваться только если нет access_code
    if (config_.access_code.empty()) {
        registerAgent();
    }
}
