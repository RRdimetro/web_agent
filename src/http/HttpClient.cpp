#include "HttpClient.h"
#include <spdlog/spdlog.h>

HttpClient::HttpClient(std::chrono::seconds timeout) : timeout_(timeout) {}

HttpResponse HttpClient::get(const std::string& url, const std::vector<cpr::Pair>& params) {
    try {
        auto response = cpr::Get(
            cpr::Url{url},
            cpr::Parameters{params},
            cpr::Timeout{timeout_},
            getSslOptions()
        );
        return {
            .success = response.status_code >= 200 && response.status_code < 300,
            .status_code = response.status_code,
            .body = response.text,
            .error = response.error.message
        };
    } catch (const std::exception& e) {
        spdlog::error("HTTP GET исключение: {}", e.what());
        return {false, 0, "", e.what()};
    }
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body, const std::string& content_type) {
    try {
        auto response = cpr::Post(
            cpr::Url{url},
            cpr::Header{{"Content-Type", content_type}},
            cpr::Body{body},
            cpr::Timeout{timeout_},
            getSslOptions()
        );
        return {
            .success = response.status_code >= 200 && response.status_code < 300,
            .status_code = response.status_code,
            .body = response.text,
            .error = response.error.message
        };
    } catch (const std::exception& e) {
        spdlog::error("HTTP POST исключение: {}", e.what());
        return {false, 0, "", e.what()};
    }
}

HttpResponse HttpClient::postFiles(const std::string& url, const std::vector<cpr::Pair>& fields, const std::vector<cpr::File>& files) {
    try {
        cpr::Multipart multipart;
        for (const auto& field : fields) {
            multipart.parts.emplace_back(field.key, field.value);
        }
        for (const auto& file : files) {
            multipart.parts.emplace_back(file.key, cpr::File{file.value});
        }
        
        auto response = cpr::Post(
            cpr::Url{url},
            multipart,
            cpr::Timeout{timeout_},
            getSslOptions()
        );
        return {
            .success = response.status_code >= 200 && response.status_code < 300,
            .status_code = response.status_code,
            .body = response.text,
            .error = response.error.message
        };
    } catch (const std::exception& e) {
        spdlog::error("HTTP POST файлы исключение: {}", e.what());
        return {false, 0, "", e.what()};
    }
}

cpr::SslOptions HttpClient::getSslOptions() const {
    if (verify_ssl_) {
        return cpr::Ssl(cpr::ssl::VerifyPeer{true}, cpr::ssl::VerifyHost{true});
    }
    return cpr::Ssl(cpr::ssl::VerifyPeer{false}, cpr::ssl::VerifyHost{false});
}