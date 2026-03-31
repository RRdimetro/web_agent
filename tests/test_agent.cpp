#include <doctest/doctest.h>
#include "agent/WebAgent.h"
#include "config/Config.h"
#include <thread>
#include <chrono>

TEST_CASE("Создание WebAgent") {
    Config config;
    config.uid = "test-agent";
    config.server_url = "https://test-server.com";
    config.log_file = "test_agent.log";

    WebAgent agent(config);

    CHECK(!agent.isRunning());

    // Очистка
    std::remove("test_agent.log");
}

TEST_CASE("WebAgent - запуск и остановка") {
    Config config;
    config.uid = "test-agent-2";
    config.server_url = "https://test-server.com";
    config.log_file = "test_agent2.log";
    config.poll_interval = std::chrono::seconds(1);

    WebAgent agent(config);

    agent.start();
    CHECK(agent.isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    agent.stop();
    CHECK(!agent.isRunning());

    // Очистка
    std::remove("test_agent2.log");
}

TEST_CASE("WebAgent - повторный запуск") {
    Config config;
    config.uid = "test-agent-3";
    config.server_url = "https://test-server.com";
    config.log_file = "test_agent3.log";

    WebAgent agent(config);

    agent.start();
    CHECK(agent.isRunning());

    // Повторный запуск не должен вызывать ошибок
    agent.start();
    CHECK(agent.isRunning());

    agent.stop();

    // Очистка
    std::remove("test_agent3.log");
}

TEST_CASE("WebAgent - повторная остановка") {
    Config config;
    config.uid = "test-agent-4";
    config.server_url = "https://test-server.com";
    config.log_file = "test_agent4.log";

    WebAgent agent(config);

    agent.start();
    agent.stop();

    // Повторная остановка не должна вызывать ошибок
    agent.stop();
    CHECK(!agent.isRunning());

    // Очистка
    std::remove("test_agent4.log");
}

TEST_CASE("WebAgent - запуск без остановки") {
    Config config;
    config.uid = "test-agent-5";
    config.server_url = "https://test-server.com";
    config.log_file = "test_agent5.log";
    config.poll_interval = std::chrono::seconds(1);

    WebAgent agent(config);

    agent.start();
    CHECK(agent.isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Деструктор должен корректно остановить агент
    // (тест проходит, если нет зависания или падения)
}
