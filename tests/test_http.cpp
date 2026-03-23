
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "../src/HttpClient.h"
#include <chrono>

TEST_CASE("HttpClient GET запрос") {
    HttpClient client(std::chrono::seconds(5));
    
    // Тест с мок-сервером
    auto response = client.get("https://httpbin.org/get", {{"test", "value"}});
    
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

TEST_CASE("HttpClient таймаут") {
    HttpClient client(std::chrono::seconds(1));
    
    auto response = client.get("https://httpbin.org/delay/5");
    
    CHECK(!response.success);
    CHECK(response.error.find("timeout") != std::string::npos);
}