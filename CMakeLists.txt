cmake_minimum_required(VERSION 3.14)
project(WebAgent VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Сборка тестов" OFF)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

set(SOURCES
    src/config/Config.cpp
    src/http/HttpClient.cpp
    src/agent/WebAgent.cpp
    src/task/Task.cpp
    src/task/TaskExecutor.cpp
    src/utils/Logger.cpp
)

# Библиотека с основным кодом для линковки в тесты
add_library(web-agent-lib STATIC ${SOURCES})

target_include_directories(web-agent-lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/http
    ${CMAKE_SOURCE_DIR}/src/agent
    ${CMAKE_SOURCE_DIR}/src/task
    ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(web-agent-lib
    cpr::cpr
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Threads::Threads
    ${CURL_LIBRARIES}
)

# Исполняемый файл агента
add_executable(web-agent src/main.cpp)
target_link_libraries(web-agent web-agent-lib)

if(BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)

    add_executable(tests
        tests/test_main.cpp
        tests/test_http.cpp
        tests/test_parser.cpp
        tests/test_task.cpp
        tests/test_config.cpp
        tests/test_logger.cpp
        tests/test_agent.cpp
    )
    target_link_libraries(tests
        web-agent-lib
        doctest::doctest
    )
    add_test(NAME web-agent-tests COMMAND tests)
endif()