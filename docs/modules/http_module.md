# HTTP-модуль (HttpClient)

## Назначение

Выполнение HTTPS запросов к серверу с поддержкой retry, таймаутов и SSL.

---

## Библиотеки

- **CPR** — C++ HTTP клиент (обёртка над libcurl)
- **libcurl** — низкоуровневая реализация
- **OpenSSL** — SSL/TLS шифрование

---

## Структура HttpResponse

```cpp
struct HttpResponse {
    bool success;           // true если статус 2xx
    int status_code;        // HTTP статус код
    std::string body;       // Тело ответа
    std::string error;      // Сообщение об ошибке
};
```

---

## Класс HttpClient

```cpp
class HttpClient {
public:
    // Конструктор
    explicit HttpClient(std::chrono::seconds timeout = 30s);
    
    // GET запрос с параметрами
    HttpResponse get(const std::string& url, 
                     const std::vector<cpr::Pair>& params = {});
    
    // POST запрос с телом
    HttpResponse post(const std::string& url, 
                      const std::string& body,
                      const std::string& content_type = "application/json");
    
    // POST запрос с файлами (multipart/form-data)
    HttpResponse postFiles(const std::string& url,
                           const std::vector<cpr::Pair>& fields,
                           const std::vector<FileData>& files);
    
    // Установка таймаута
    void setTimeout(std::chrono::seconds timeout);
    
    // Проверка SSL сертификатов
    void setVerifySsl(bool verify);

private:
    std::chrono::seconds timeout_;
    bool verify_ssl_{true};
    
    cpr::SslOptions getSslOptions() const;
};
```

---

## Структура FileData

```cpp
struct FileData {
    std::string key;        // Имя поля (например, "file1")
    std::string filepath;   // Путь к файлу
};
```

---

## Примеры использования

### GET запрос

```cpp
HttpClient client(30s);  // таймаут 30 секунд

auto response = client.get(
    "https://server.com/api/task",
    {{"session_id", "abc123"}}
);

if (response.success) {
    spdlog::info("Ответ: {}", response.body);
} else {
    spdlog::error("Ошибка: {}", response.error);
}
```

### POST запрос

```cpp
nlohmann::json request;
request["UID"] = "agent-001";
request["descr"] = "web-agent";

auto response = client.post(
    "https://server.com/wa_reg/",
    request.dump(),
    "application/json"
);
```

### POST с файлами (multipart)

```cpp
std::vector<cpr::Pair> fields = {
    {"result_code", "0"},
    {"result", result_json.dump()}
};

std::vector<FileData> files = {
    {"file1", "./results/output.txt"}
};

auto response = client.postFiles(
    "https://server.com/wa_result/",
    fields,
    files
);
```

---

## Обработка ошибок

### Таймауты

| Тип | Значение |
|-----|----------|
| connection_timeout | 30 сек |
| request_timeout | 60 сек |

### SSL

```cpp
// Отключить проверку сертификатов (для self-signed)
client.setVerifySsl(false);

// Включить проверку (по умолчанию)
client.setVerifySsl(true);
```

### Retry механизм

Реализован в `WebAgent::handleNetworkError()`:

```cpp
void WebAgent::handleNetworkError() {
    consecutive_failures_++;
    
    // Экспоненциальная задержка
    auto backoff = config_.poll_interval * (1 << std::min(consecutive_failures_, 5));
    if (backoff > config_.max_poll_interval) {
        backoff = config_.max_poll_interval;
    }
    
    current_interval_ = backoff;
    spdlog::warn("Сетевая ошибка, следующий опрос через {} секунд", current_interval_.count());
    
    std::this_thread::sleep_for(current_interval_);
}
```

---

## Коды ответов сервера

| Код | Описание | Действие |
|-----|----------|----------|
| 200 | OK | Обработка ответа |
| 404 | Нет задач | Возврат в режим ожидания |
| 401/403 | Неверный access_code | Очистка кода, повторная регистрация |
| 500/503 | Ошибка сервера | Retry с задержкой |
| 0 | Нет соединения | Retry с экспоненциальной задержкой |

---

## Безопасность

- Все запросы по HTTPS
- Проверка SSL сертификата (опционально)
- Валидация URL перед запросом
- Очистка чувствительных данных из логов
