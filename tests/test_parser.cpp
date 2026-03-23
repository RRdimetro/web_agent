
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "../src/Task.h"

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
    CHECK(task.destination_folder == "/tmp/output");
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