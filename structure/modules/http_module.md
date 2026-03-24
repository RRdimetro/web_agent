# Спецификация HTTP-модуля

## Назначение
Выполнение HTTP/HTTPS запросов к серверу с поддержкой retry, таймаутов и SSL.

## Входные данные
- URL запроса
- Метод HTTP (GET, POST, PUT, DELETE)
- Тело запроса (для POST/PUT)
- Параметры запроса (для GET)
- Заголовки

## Выходные данные
- Структура HttpResponse с кодом статуса, телом ответа и заголовками

## Библиотеки
- **CPR** - C++ HTTP клиент (обертка над libcurl)
- **libcurl** - низкоуровневая реализация

## Класс HttpClient

```cpp
class HttpClient {
public:
    struct HttpResponse {
        int statusCode;
        std::string body;
        std::map<std::string, std::string> headers;
        double durationSec;

        bool isSuccess() const {
            return statusCode >= 200 && statusCode < 300;
        }

        bool isRetryable() const {
            return statusCode >= 500 || statusCode == 408 || statusCode == 429;
        }
    };

    void setConnectionTimeout(int seconds);
    void setRequestTimeout(int seconds);
    void setRetryCount(int count);
    void setRetryDelay(int seconds);
    void setVerifySsl(bool verify);

    HttpResponse post(const std::string& url,
                      const std::string& body,
                      const std::map<std::string, std::string>& headers = {});

    HttpResponse get(const std::string& url,
                     const std::map<std::string, std::string>& params = {},
                     const std::map<std::string, std::string>& headers = {});
};
```

## Retry механизм

| Условие | Действие |
|---------|----------|
| Сетевые ошибки | Повтор с экспоненциальной задержкой |
| 5xx (Server Error) | Повтор с экспоненциальной задержкой |
| 408 (Request Timeout) | Повтор с экспоненциальной задержкой |
| 429 (Too Many Requests) | Повтор с увеличенной задержкой |
| 4xx (Client Error) | Без повтора (кроме 408, 429) |
| 2xx (Success) | Завершение |

## Экспоненциальная задержка

```
delay = base_delay * (2 ^ attempt) + jitter

где:
- base_delay = 1 секунда
- max_delay = 60 секунд
- jitter = случайное значение ±20%
```

## Таймауты

| Тип таймаута | Описание | Значение по умолчанию |
|--------------|----------|----------------------|
| connection_timeout | Ожидание соединения | 30 сек |
| request_timeout | Ожидание ответа | 60 сек |
| total_timeout | Общее время запроса | 120 сек |

## Обработка ошибок

| Код | Описание | Действие |
|-----|----------|----------|
| CURLE_OK | Успех | Возврат ответа |
| CURLE_COULDNT_CONNECT | Не удалось подключиться | Retry |
| CURLE_OPERATION_TIMEDOUT | Таймаут | Retry |
| CURLE_SSL_CERTPROBLEM | Ошибка сертификата | Логирование, без retry |

## Пример использования

```cpp
HttpClient client("https://api.example.com");
client.setConnectionTimeout(10);
client.setRetryCount(3);

auto response = client.post("/register",
    R"({"uid": "agent-001"})",
    {{"Content-Type", "application/json"}});

if (response.isSuccess()) {
    std::cout << "Response: " << response.body << std::endl;
} else {
    std::cerr << "Error: " << response.statusCode << std::endl;
}
```
