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
    GIT_TAG 1.10.0
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
    src/main.cpp
    src/config/Config.cpp
    src/http/HttpClient.cpp
    src/agent/WebAgent.cpp
    src/task/Task.cpp
    src/task/TaskExecutor.cpp
    src/utils/Logger.cpp
)

add_executable(web-agent ${SOURCES})

target_include_directories(web-agent PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/http
    ${CMAKE_SOURCE_DIR}/src/agent
    ${CMAKE_SOURCE_DIR}/src/task
    ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(web-agent
    cpr::cpr
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Threads::Threads
    ${CURL_LIBRARIES}
)

if(BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)
    
    add_executable(tests 
        tests/test_http.cpp 
        tests/test_parser.cpp 
        tests/test_task.cpp
    )
    target_link_libraries(tests 
        web-agent
        doctest::doctest
    )
    add_test(NAME web-agent-tests COMMAND tests)
endif()