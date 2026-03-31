#include <doctest/doctest.h>
#include "task/Task.h"
#include <nlohmann/json.hpp>

TEST_CASE("Парсинг задания команды") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "execute_command",
        "command": "echo hello",
        "timeout": 60
    })";

    Task task = Task::fromJson(json);

    CHECK(task.session_id == "sess-123");
    CHECK(task.task_id == "task-456");
    CHECK(task.type == TaskType::ExecuteCommand);
    CHECK(task.command == "echo hello");
    CHECK(task.timeout == std::chrono::seconds(60));
    CHECK(task.isValid());
}

TEST_CASE("Парсинг задания команды с таймаутом по умолчанию") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "execute_command",
        "command": "echo hello"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::ExecuteCommand);
    CHECK(task.command == "echo hello");
    CHECK(task.timeout == std::chrono::seconds(300)); // значение по умолчанию
    CHECK(task.isValid());
}

TEST_CASE("Парсинг задания программы") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "run_program",
        "program": "/usr/bin/python3",
        "arguments": ["script.py", "--verbose"],
        "timeout": 120
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::RunProgram);
    CHECK(task.program_path == "/usr/bin/python3");
    CHECK(task.arguments.size() == 2);
    CHECK(task.arguments[0] == "script.py");
    CHECK(task.arguments[1] == "--verbose");
    CHECK(task.isValid());
}

TEST_CASE("Парсинг задания программы без аргументов") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "run_program",
        "program": "/usr/bin/python3",
        "timeout": 120
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::RunProgram);
    CHECK(task.program_path == "/usr/bin/python3");
    CHECK(task.arguments.empty());
    CHECK(task.isValid());
}

TEST_CASE("Парсинг задания передачи файлов") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "transfer_file",
        "source_files": ["file1.txt", "file2.txt"],
        "destination": "/tmp/output"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::TransferFile);
    CHECK(task.source_files.size() == 2);
    CHECK(task.source_files[0] == "file1.txt");
    CHECK(task.source_files[1] == "file2.txt");
    CHECK(task.destination_folder == "/tmp/output");
    CHECK(task.isValid());
}

TEST_CASE("Парсинг задания передачи одного файла") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "transfer_file",
        "source_files": ["file1.txt"],
        "destination": "/tmp/output"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::TransferFile);
    CHECK(task.source_files.size() == 1);
    CHECK(task.isValid());
}

TEST_CASE("Пустое задание") {
    std::string json = R"({
        "session_id": "",
        "task_id": "null"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::None);
    CHECK(!task.isValid());
}

TEST_CASE("Задание с пустым session_id") {
    std::string json = R"({
        "session_id": "",
        "task_id": "task-456",
        "type": "execute_command",
        "command": "echo hello"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::ExecuteCommand);
    CHECK(!task.isValid()); // session_id пустой
}

TEST_CASE("Задание с пустым task_id") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "",
        "type": "execute_command",
        "command": "echo hello"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::ExecuteCommand);
    CHECK(!task.isValid()); // task_id пустой
}

TEST_CASE("Задание команды с пустой командой") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "execute_command",
        "command": ""
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::ExecuteCommand);
    CHECK(!task.isValid()); // команда пустая
}

TEST_CASE("Задание программы с пустым путем") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "run_program",
        "program": ""
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::RunProgram);
    CHECK(!task.isValid()); // путь пустой
}

TEST_CASE("Задание передачи файлов с пустым destination") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "transfer_file",
        "source_files": ["file1.txt"],
        "destination": ""
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::TransferFile);
    CHECK(!task.isValid()); // destination пустой
}

TEST_CASE("Задание передачи файлов с пустым списком файлов") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "transfer_file",
        "source_files": [],
        "destination": "/tmp/output"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::TransferFile);
    CHECK(!task.isValid()); // список файлов пустой
}

TEST_CASE("Задание с неизвестным типом") {
    std::string json = R"({
        "session_id": "sess-123",
        "task_id": "task-456",
        "type": "unknown_type"
    })";

    Task task = Task::fromJson(json);

    CHECK(task.type == TaskType::None);
    CHECK(!task.isValid());
}

TEST_CASE("Задание с некорректным JSON") {
    std::string json = R"({ invalid json })";

    CHECK_THROWS_AS(Task::fromJson(json), nlohmann::json::exception);
}
