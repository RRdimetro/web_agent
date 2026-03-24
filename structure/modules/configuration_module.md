# Спецификация модуля конфигурации

## Назначение
Загрузка, парсинг, валидация и предоставление доступа к настройкам агента.

## Входные данные
- Путь к JSON файлу конфигурации (по умолчанию: `./config.json`)

## Выходные данные
- Структура с параметрами конфигурации, доступная через геттеры

## Формат файла
**JSON** - выбран по следующим причинам:
- Читаемость для человека
- Широкая поддержка в C++ (nlohmann/json)
- Нативная поддержка в REST API

## Структура конфигурационного файла

```json
{
    "agent": {
        "uid": "agent-001",
        "name": "Workstation-01"
    },
    "server": {
        "url": "https://command-server.example.com",
        "connection_timeout_sec": 30,
        "retry_count": 3
    },
    "polling": {
        "interval_sec": 10
    },
    "paths": {
        "task_directory": "./tasks",
        "result_directory": "./results",
        "log_file": "./logs/agent.log"
    },
    "execution": {
        "max_concurrent_tasks": 4,
        "default_timeout_sec": 300
    },
    "logging": {
        "level": "info",
        "console_output": true
    }
}
```

## Параметры конфигурации

**Секция agent**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| uid | string | Да | Уникальный идентификатор агента |
| name | string | Нет | Отображаемое имя агента |

**Секция server**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| url | string | Да | URL сервера (http:// или https://) |
| connection_timeout_sec | int | Нет | Таймаут соединения (сек), по умолчанию 30 |
| retry_count | int | Нет | Количество повторных попыток, по умолчанию 3 |

**Секция polling**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| interval_sec | int | Да | Интервал опроса сервера (сек) |

**Секция paths**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| task_directory | string | Да | Директория для хранения задач |
| result_directory | string | Да | Директория для результатов |
| log_file | string | Да | Путь к файлу лога |

**Секция execution**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| max_concurrent_tasks | int | Нет | Максимум параллельных задач, по умолчанию 1 |
| default_timeout_sec | int | Нет | Таймаут выполнения задачи (сек), по умолчанию 300 |

**Секция logging**

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| level | string | Нет | Уровень логирования: trace/debug/info/warn/error |
| console_output | bool | Нет | Вывод в консоль, по умолчанию false |

## Методы класса Config

```cpp
class Config {
public:
    bool loadFromFile(const std::filesystem::path& path);
    bool validate() const;

    std::string getUid() const;
    std::string getName() const;
    std::string getServerUrl() const;
    int getPollIntervalSec() const;
    int getConnectionTimeoutSec() const;
    int getRetryCount() const;
    std::filesystem::path getTaskDirectory() const;
    std::filesystem::path getResultDirectory() const;
    std::filesystem::path getLogFile() const;
    int getMaxConcurrentTasks() const;
    int getDefaultTimeoutSec() const;
    std::string getLogLevel() const;
    bool getConsoleOutput() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```

## Обработка ошибок

| Ошибка | Действие |
|--------|----------|
| Файл не найден | Логирование ERROR, выход с кодом 1 |
| Некорректный JSON | Логирование ERROR с указанием строки |
| Отсутствие обязательных полей | Логирование ERROR, выход с кодом 1 |
| Пути не доступны для записи | Логирование WARNING, попытка создания |
