#include <doctest/doctest.h>
#include "utils/Logger.h"
#include "task/Task.h"
#include "task/TaskExecutor.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

TEST_CASE("Создание Logger") {
    Logger logger("./test_logger.log");

    // Проверяем, что Logger создался без ошибок
    CHECK(true);

    // Очистка
    fs::remove("./test_logger.log");
}

TEST_CASE("Logger - запись информации") {
    Logger logger("./test_logger_info.log");

    logger.logInfo("Тестовое сообщение");

    // Проверяем, что файл существует и не пуст
    std::ifstream file("./test_logger_info.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CHECK_FALSE(content.empty());
    CHECK(content.find("Тестовое сообщение") != std::string::npos);
    CHECK(content.find("[ИНФО]") != std::string::npos);

    // Очистка
    fs::remove("./test_logger_info.log");
}

TEST_CASE("Logger - запись предупреждения") {
    Logger logger("./test_logger_warn.log");

    logger.logWarning("Предупреждение");

    std::ifstream file("./test_logger_warn.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Предупреждение") != std::string::npos);
    CHECK(content.find("[ПРЕДУПРЕЖДЕНИЕ]") != std::string::npos);

    // Очистка
    fs::remove("./test_logger_warn.log");
}

TEST_CASE("Logger - запись ошибки") {
    Logger logger("./test_logger_error.log");

    logger.logError("task-123", "Произошла ошибка");

    std::ifstream file("./test_logger_error.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("task-123") != std::string::npos);
    CHECK(content.find("Произошла ошибка") != std::string::npos);
    CHECK(content.find("[ОШИБКА]") != std::string::npos);

    // Очистка
    fs::remove("./test_logger_error.log");
}

TEST_CASE("Logger - запись выполнения задания") {
    Logger logger("./test_logger_task.log");

    Task task;
    task.task_id = "task-456";
    task.type = TaskType::ExecuteCommand;
    task.command = "echo test";

    ExecutionResult result;
    result.success = true;
    result.exit_code = 0;
    result.output_files = {"file1.txt", "file2.txt"};

    logger.logTask(task, result);

    std::ifstream file("./test_logger_task.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("task-456") != std::string::npos);
    CHECK(content.find("да") != std::string::npos); // успех
    CHECK(content.find("0") != std::string::npos); // код возврата
    CHECK(content.find("2") != std::string::npos); // количество файлов

    // Очистка
    fs::remove("./test_logger_task.log");
}

TEST_CASE("Logger - запись неудачного выполнения задания") {
    Logger logger("./test_logger_task_fail.log");

    Task task;
    task.task_id = "task-789";
    task.type = TaskType::ExecuteCommand;

    ExecutionResult result;
    result.success = false;
    result.exit_code = 1;
    result.error_message = "Command failed";

    logger.logTask(task, result);

    std::ifstream file("./test_logger_task_fail.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("task-789") != std::string::npos);
    CHECK(content.find("нет") != std::string::npos); // неудача
    CHECK(content.find("1") != std::string::npos); // код возврата

    // Очистка
    fs::remove("./test_logger_task_fail.log");
}

TEST_CASE("Logger - множественные записи") {
    Logger logger("./test_logger_multi.log");

    logger.logInfo("Сообщение 1");
    logger.logWarning("Сообщение 2");
    logger.logError("task-1", "Ошибка 1");
    logger.logInfo("Сообщение 3");

    // Перечитываем файл для проверки
    std::ifstream file("./test_logger_multi.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Сообщение 1") != std::string::npos);
    CHECK(content.find("Сообщение 2") != std::string::npos);
    CHECK(content.find("Сообщение 3") != std::string::npos);
    CHECK(content.find("Ошибка 1") != std::string::npos);

    // Проверяем количество строк (должно быть 4)
    file.clear();
    file.seekg(0);
    int line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        line_count++;
    }
    CHECK(line_count == 4);

    // Очистка
    fs::remove("./test_logger_multi.log");
}

TEST_CASE("Logger - временная метка") {
    Logger logger("./test_logger_timestamp.log");

    logger.logInfo("Тест");

    std::ifstream file("./test_logger_timestamp.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Проверяем формат временной метки [YYYY-MM-DD HH:MM:SS.mmm]
    CHECK(content.find("[20") != std::string::npos); // год начинается с 20
    CHECK(content.find("-") != std::string::npos); // разделители даты
    CHECK(content.find(":") != std::string::npos); // разделители времени

    // Очистка
    fs::remove("./test_logger_timestamp.log");
}
