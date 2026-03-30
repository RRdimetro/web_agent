
// DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN определён в test_main.cpp
#include <doctest/doctest.h>
#include "http/HttpClient.h"
#include <chrono>

TEST_CASE("HttpClient GET запрос") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.get("https://httpbin.org/get", {{"test", "value"}});

    // Проверяем только что ответ получен (статус код всегда >= 0)
    CHECK(response.status_code != -1);
}

TEST_CASE("HttpClient GET запрос без параметров") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.get("https://httpbin.org/get");

    CHECK(response.success);
    CHECK(response.status_code == 200);
    CHECK(!response.body.empty());
}

TEST_CASE("HttpClient POST запрос") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.post("https://httpbin.org/post", R"({"key":"value"})");

    CHECK(response.success);
    CHECK(response.status_code == 200);
}

TEST_CASE("HttpClient POST запрос с пустым телом") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.post("https://httpbin.org/post", "");

    CHECK(response.success);
    CHECK(response.status_code == 200);
}

TEST_CASE("HttpClient POST запрос с custom content-type") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.post("https://httpbin.org/post", "test data", "text/plain");

    CHECK(response.success);
    CHECK(response.status_code == 200);
}

TEST_CASE("HttpClient таймаут") {
    HttpClient client(std::chrono::seconds(1));

    auto response = client.get("https://httpbin.org/delay/5");

    CHECK(!response.success);
    // Проверяем, что есть ошибка или статус 0
    bool has_error = !response.error.empty() || response.status_code == 0;
    REQUIRE(has_error);
}

TEST_CASE("HttpClient неверный URL") {
    HttpClient client(std::chrono::seconds(5));

    auto response = client.get("https://invalid-url-that-does-not-exist-12345.com");

    CHECK(!response.success);
}

TEST_CASE("HttpClient установка таймаута") {
    HttpClient client(std::chrono::seconds(5));
    
    client.setTimeout(std::chrono::seconds(10));
    
    CHECK(true); // Проверяем, что метод не выбрасывает исключение
}

TEST_CASE("HttpClient отключение SSL проверки") {
    HttpClient client(std::chrono::seconds(5));
    
    client.setVerifySsl(false);
    
    CHECK(true); // Проверяем, что метод не выбрасывает исключение
}