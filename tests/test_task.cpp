
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "src/task/TaskExecutor.h"
#include <filesystem>
#include <fstream>

TEST_CASE("Выполнение команды") {
    Task task;
    task.type = TaskType::ExecuteCommand;
    task.task_id = "test-1";
    task.command = "echo 'test'";
    
    auto result = TaskExecutor::execute(task, "./test_results");
    
    CHECK(result.success);
    CHECK(result.exit_code == 0);
}

TEST_CASE("Ошибка выполнения команды") {
    Task task;
    task.type = TaskType::ExecuteCommand;
    task.task_id = "test-2";
    task.command = "nonexistent_command_12345";
    
    auto result = TaskExecutor::execute(task, "./test_results");
    
    CHECK(!result.success);
    CHECK(result.exit_code != 0);
}

TEST_CASE("Передача файлов") {
    // Создаем тестовый файл
    std::filesystem::create_directories("./test_source");
    std::ofstream("./test_source/test.txt") << "test content";
    
    Task task;
    task.type = TaskType::TransferFile;
    task.task_id = "test-3";
    task.source_files = {"./test_source/test.txt"};
    task.destination_folder = "./test_dest";
    
    auto result = TaskExecutor::execute(task, "./test_results");
    
    CHECK(result.success);
    CHECK(std::filesystem::exists("./test_dest/test.txt"));
    
    // Очистка
    std::filesystem::remove_all("./test_source");
    std::filesystem::remove_all("./test_dest");
}