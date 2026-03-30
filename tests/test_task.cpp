#include <doctest/doctest.h>
#include "task/TaskExecutor.h"
#include "task/Task.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("Выполнение команды - успех") {
    Task task;
    task.type = TaskType::ExecuteCommand;
    task.task_id = "test-1";
    task.command = "echo 'test'";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(result.success);
    CHECK(result.exit_code == 0);
    CHECK(result.error_message.empty());
}

TEST_CASE("Выполнение команды - неудача") {
    Task task;
    task.type = TaskType::ExecuteCommand;
    task.task_id = "test-2";
    task.command = "nonexistent_command_12345";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(!result.success);
    CHECK(result.exit_code != 0);
    CHECK(!result.error_message.empty());
}

TEST_CASE("Выполнение команды с кодом возврата") {
    Task task;
    task.type = TaskType::ExecuteCommand;
    task.task_id = "test-3";
#ifdef _WIN32
    task.command = "exit /b 1";
#else
    task.command = "exit 1";
#endif

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(!result.success);
    // В Linux exit_code умножается на 256 из-за WEXITSTATUS
    CHECK(result.exit_code != 0);
}

TEST_CASE("Передача файлов - успех") {
    // Создаем тестовый файл
    fs::create_directories("./test_source");
    std::ofstream("./test_source/test.txt") << "test content";

    Task task;
    task.type = TaskType::TransferFile;
    task.task_id = "test-4";
    task.source_files = {"./test_source/test.txt"};
    task.destination_folder = "./test_dest";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(result.success);
    CHECK(result.exit_code == 0);
    CHECK(fs::exists("./test_dest/test.txt"));
    CHECK(result.output_files.size() == 1);

    // Очистка
    fs::remove_all("./test_source");
    fs::remove_all("./test_dest");
}

TEST_CASE("Передача файлов - несуществующий файл") {
    Task task;
    task.type = TaskType::TransferFile;
    task.task_id = "test-5";
    task.source_files = {"./nonexistent_file.txt"};
    task.destination_folder = "./test_dest";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(!result.success);
    CHECK(!result.error_message.empty());
}

TEST_CASE("Передача нескольких файлов") {
    // Создаем тестовые файлы
    fs::create_directories("./test_source");
    std::ofstream("./test_source/file1.txt") << "content 1";
    std::ofstream("./test_source/file2.txt") << "content 2";

    Task task;
    task.type = TaskType::TransferFile;
    task.task_id = "test-6";
    task.source_files = {"./test_source/file1.txt", "./test_source/file2.txt"};
    task.destination_folder = "./test_dest_multi";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(result.success);
    CHECK(result.output_files.size() == 2);
    CHECK(fs::exists("./test_dest_multi/file1.txt"));
    CHECK(fs::exists("./test_dest_multi/file2.txt"));

    // Очистка
    fs::remove_all("./test_source");
    fs::remove_all("./test_dest_multi");
}

TEST_CASE("Задание с неизвестным типом") {
    Task task;
    task.type = TaskType::None;
    task.task_id = "test-7";

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(!result.success);
    CHECK(result.exit_code == -1);
    CHECK(result.error_message == "Неизвестный тип задания");
}

TEST_CASE("Запуск программы - успех") {
    Task task;
    task.type = TaskType::RunProgram;
    task.task_id = "test-8";
#ifdef _WIN32
    task.program_path = "cmd.exe";
    task.arguments = {"/c", "echo", "hello"};
#else
    task.program_path = "/bin/echo";
    task.arguments = {"hello"};
#endif

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(result.success);
    CHECK(result.exit_code == 0);
}

TEST_CASE("Запуск программы - неудача") {
    Task task;
    task.type = TaskType::RunProgram;
    task.task_id = "test-9";
    task.program_path = "/nonexistent_program";
    task.arguments = {"arg1"};

    auto result = TaskExecutor::execute(task, "./test_results");

    CHECK(!result.success);
    CHECK(result.exit_code != 0);
}
