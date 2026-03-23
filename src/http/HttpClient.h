#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <cpr/cpr.h>

struct HttpResponse {
    bool success;           // успех
    int status_code;        // код статуса
    std::string body;       // тело ответа
    std::string error;      // сообщение об ошибке
};

class HttpClient {
public:
    explicit HttpClient(std::chrono::seconds timeout = std::chrono::seconds(30));
    
    // GET запрос с параметрами
    HttpResponse get(const std::string& url, const std::vector<cpr::Pair>& params = {});
    
    // POST запрос с телом
    HttpResponse post(const std::string& url, const std::string& body, const std::string& content_type = "application/json");
    
    // POST запрос с файлами (multipart/form-data)
    HttpResponse postFiles(const std::string& url, const std::vector<cpr::Pair>& fields, const std::vector<cpr::File>& files);
    
    void setTimeout(std::chrono::seconds timeout) { timeout_ = timeout; }
    void setVerifySsl(bool verify) { verify_ssl_ = verify; }
    
private:
    std::chrono::seconds timeout_;      // таймаут
    bool verify_ssl_{true};             // проверка SSL сертификатов
    
    cpr::SslOptions getSslOptions() const;
};