# Модуль конфигурации (Config)

## Назначение

Загрузка, парсинг, валидация и предоставление доступа к настройкам агента.

---

## Входные данные

- Путь к JSON файлу конфигурации (по умолчанию: `config/agent_config.json`)

---

## Выходные данные

- Структура `Config` с параметрами, доступными через публичные поля

---

## Формат файла

**JSON** — причины выбора:
- Читаемость для человека
- Поддержка в C++ (nlohmann/json)
- Нативная поддержка в REST API

---

## Структура

```json
{
    "uid": "agent-001",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "access_code": "",
    "tasks_folder": "./tasks",
    "results_folder": "./results",
    "log_file": "./agent.log",
    "poll_interval": 5,
    "max_poll_interval": 300,
    "timeout": 30,
    "max_retries": 3,
    "retry_delay": 5
}
```

---

## Поля структуры Config

```cpp
struct Config {
    std::string uid;                         // UID агента
    std::string server_url;                  // URL сервера
    std::string access_code;                 // Код доступа
    std::filesystem::path tasks_folder;      // Папка задач
    std::filesystem::path results_folder;    // Папка результатов
    std::filesystem::path log_file;          // Файл логов
    std::chrono::seconds poll_interval{5};   // Интервал опроса
    std::chrono::seconds max_poll_interval{300}; // Макс. интервал
    std::chrono::seconds timeout{30};        // Таймаут запросов
    int max_retries{3};                      // Макс. попыток
    std::chrono::seconds retry_delay{5};     // Задержка между попытками
};
```

---

## Методы

```cpp
class Config {
public:
    // Загрузить конфигурацию из файла
    static Config load(const std::string& path);
    
    // Проверить корректность конфигурации
    bool validate() const;
    
    // Установить значения по умолчанию
    void setDefaults();
};
```

---

## Пример использования

```cpp
// Загрузка конфигурации
Config config = Config::load("config/agent_config.json");

// Проверка
if (!config.validate()) {
    spdlog::error("Некорректная конфигурация");
    return 1;
}

// Использование
spdlog::info("UID: {}, Сервер: {}", config.uid, config.server_url);
spdlog::info("Интервал опроса: {} сек", config.poll_interval.count());
```

---

## Обработка ошибок

| Ошибка | Действие |
|--------|----------|
| Файл не найден | `std::runtime_error`, логирование ERROR |
| Некорректный JSON | `std::runtime_error`, логирование ошибки парсинга |
| Отсутствуют обязательные поля | Логирование, выброс исключения |
| Пути недоступны для записи | Попытка создания директорий |

---

## Валидация

### Обязательные поля:
- `uid` — не пустая строка
- `server_url` — не пустая строка, начинается с `https://`

### Проверка диапазонов:
- `poll_interval` > 0
- `max_poll_interval` >= `poll_interval`
- `timeout` > 0
- `max_retries` >= 0
- `retry_delay` > 0

---

## Изменение конфигурации

Конфигурация может быть изменена динамически через задачу **CONF**:

```cpp
// В TaskExecutor::changeConfig()
if (key == "poll_interval_sec") {
    config_->poll_interval = std::chrono::seconds(new_value);
}
```
