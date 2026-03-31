#include <doctest/doctest.h>
#include "config/Config.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

TEST_CASE("Загрузка корректного конфига") {
    // Создаем временный файл конфигурации
    std::string config_content = R"({
        "uid": "test-agent-001",
        "server_url": "https://test-server.com:8443",
        "tasks_folder": "./test_tasks",
        "results_folder": "./test_results",
        "log_file": "./test_agent.log",
        "poll_interval": 10,
        "max_poll_interval": 600,
        "timeout": 60,
        "max_retries": 5,
        "retry_delay": 10
    })";

    std::ofstream config_file("test_config.json");
    config_file << config_content;
    config_file.close();

    Config cfg = Config::load("test_config.json");

    CHECK(cfg.uid == "test-agent-001");
    CHECK(cfg.server_url == "https://test-server.com:8443");
    CHECK(cfg.tasks_folder == "./test_tasks");
    CHECK(cfg.results_folder == "./test_results");
    CHECK(cfg.poll_interval == std::chrono::seconds(10));
    CHECK(cfg.max_poll_interval == std::chrono::seconds(600));
    CHECK(cfg.timeout == std::chrono::seconds(60));
    CHECK(cfg.max_retries == 5);
    CHECK(cfg.retry_delay == std::chrono::seconds(10));

    // Очистка
    fs::remove("test_config.json");
    fs::remove_all("./test_tasks");
    fs::remove_all("./test_results");
    fs::remove("./test_agent.log");
}

TEST_CASE("Загрузка конфига с минимальными полями") {
    std::string config_content = R"({
        "uid": "minimal-agent",
        "server_url": "https://minimal.com"
    })";

    std::ofstream config_file("test_min_config.json");
    config_file << config_content;
    config_file.close();

    Config cfg = Config::load("test_min_config.json");

    CHECK(cfg.uid == "minimal-agent");
    CHECK(cfg.server_url == "https://minimal.com");
    // Проверяем значения по умолчанию
    CHECK(cfg.poll_interval == std::chrono::seconds(5));
    CHECK(cfg.max_poll_interval == std::chrono::seconds(300));
    CHECK(cfg.timeout == std::chrono::seconds(30));
    CHECK(cfg.max_retries == 3);
    CHECK(cfg.retry_delay == std::chrono::seconds(5));

    // Очистка
    fs::remove("test_min_config.json");
    fs::remove_all(cfg.tasks_folder);
    fs::remove_all(cfg.results_folder);
}

TEST_CASE("Загрузка несуществующего конфига") {
    CHECK_THROWS_AS(Config::load("nonexistent_config.json"), std::runtime_error);
}

TEST_CASE("Загрузка конфига с некорректным JSON") {
    std::ofstream config_file("test_invalid_config.json");
    config_file << "{ invalid json }";
    config_file.close();

    CHECK_THROWS_AS(Config::load("test_invalid_config.json"), std::runtime_error);

    // Очистка
    fs::remove("test_invalid_config.json");
}

TEST_CASE("Загрузка конфига с пустым UID") {
    std::string config_content = R"({
        "uid": "",
        "server_url": "https://test.com"
    })";

    std::ofstream config_file("test_empty_uid.json");
    config_file << config_content;
    config_file.close();

    CHECK_THROWS_AS(Config::load("test_empty_uid.json"), std::runtime_error);

    // Очистка
    fs::remove("test_empty_uid.json");
}

TEST_CASE("Загрузка конфига с пустым server_url") {
    std::string config_content = R"({
        "uid": "test-agent",
        "server_url": ""
    })";

    std::ofstream config_file("test_empty_url.json");
    config_file << config_content;
    config_file.close();

    CHECK_THROWS_AS(Config::load("test_empty_url.json"), std::runtime_error);

    // Очистка
    fs::remove("test_empty_url.json");
}

TEST_CASE("Валидация конфига - успех") {
    Config cfg;
    cfg.uid = "valid-agent";
    cfg.server_url = "https://valid.com";
    cfg.tasks_folder = "./valid_tasks";
    cfg.results_folder = "./valid_results";

    CHECK(cfg.validate());

    // Очистка
    fs::remove_all("./valid_tasks");
    fs::remove_all("./valid_results");
}

TEST_CASE("Валидация конфига - пустой UID") {
    Config cfg;
    cfg.uid = "";
    cfg.server_url = "https://valid.com";

    CHECK(!cfg.validate());
}

TEST_CASE("Валидация конфига - пустой server_url") {
    Config cfg;
    cfg.uid = "test-agent";
    cfg.server_url = "";

    CHECK(!cfg.validate());
}

TEST_CASE("Конфиг - значения по умолчанию") {
    Config cfg;
    cfg.setDefaults();

    CHECK_FALSE(cfg.uid.empty());
    CHECK(cfg.tasks_folder == fs::current_path() / "tasks");
    CHECK(cfg.results_folder == fs::current_path() / "results");
    CHECK(cfg.log_file == fs::current_path() / "agent.log");
}
