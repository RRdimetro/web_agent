# Спецификация модуля логирования

## Назначение
Потокобезопасное логирование с поддержкой уровней, ротации файлов и форматирования.

## Библиотека
**spdlog** - высокопроизводительная библиотека логирования для C++

## Уровни логирования

| Уровень | Описание | Использование |
|---------|----------|---------------|
| TRACE | Детальная отладка | Разработка, отладка |
| DEBUG | Отладочная информация | Разработка, тестирование |
| INFO | Информационные сообщения | Нормальная работа |
| WARNING | Предупреждения | Потенциальные проблемы |
| ERROR | Ошибки | Сбои, требующие внимания |
| CRITICAL | Критические ошибки | Аварийное завершение |
| OFF | Логирование отключено | - |

## Класс Logger

```cpp
class Logger {
public:
    static Logger& getInstance();

    void initialize(const Config& config);
    void initialize(const std::string& logFile, LogLevel level, bool consoleOutput);

    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        info(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        error(fmt::format(fmt, std::forward<Args>(args)...));
    }

    void setLevel(LogLevel level);
    void setPattern(const std::string& pattern);
    void flush();
    bool shouldLog(LogLevel level) const;

private:
    Logger() = default;
    std::shared_ptr<spdlog::logger> m_logger;
    LogLevel m_level;
};
```

## Формат логов

```
[2024-01-15 10:30:45.123] [INFO] [Application] Configuration loaded successfully
[2024-01-15 10:30:45.456] [INFO] [ApiClient] Registered with sessionId: sess_abc123
[2024-01-15 10:30:55.789] [DEBUG] [TaskRunner] Polling server for tasks
[2024-01-15 10:30:56.012] [ERROR] [HttpClient] Connection timeout after 30s
```

**Компоненты формата:**
- `[%Y-%m-%d %H:%M:%S.%e]` - дата и время с микросекундами
- `[%l]` - уровень логирования
- `[%n]` - имя логгера (модуль)
- `%v` - сообщение

## Ротация логов

| Параметр | Описание | Значение по умолчанию |
|----------|----------|----------------------|
| Максимальный размер файла | Размер после которого создается новый файл | 10 MB |
| Максимальное количество файлов | Количество хранимых архивов | 7 |
| Политика ротации | По размеру или по времени | По размеру |

## Пример использования

```cpp
auto& logger = Logger::getInstance();

Config config;
config.loadFromFile("config.json");
logger.initialize(config);

logger.info("Agent started with UID: {}", config.getUid());
logger.debug("Poll interval: {} seconds", config.getPollIntervalSec());

try {
    someOperation();
} catch (const std::exception& e) {
    logger.error("Operation failed: {}", e.what());
}

logger.info("Task {} completed in {} ms", taskId, duration);
```

## Логирование в разных модулях

```cpp
// В модуле конфигурации
logger.info("Configuration loaded from: {}", configPath);
logger.error("Invalid configuration: missing field 'uid'");

// В HTTP модуле
logger.debug("Sending {} request to: {}", method, url);
logger.warn("Request failed, retrying ({}/{})", attempt, maxRetries);

// В модуле выполнения
logger.info("Process started: pid={}", pid);
logger.warn("Process timeout after {} seconds, killing", timeout);

// В модуле API
logger.info("Registered successfully, sessionId: {}", sessionId);
logger.error("Registration failed after {} retries", retryCount);
```

## Потокобезопасность

Логгер полностью потокобезопасен:
- spdlog использует внутренние мьютексы
- Несколько потоков могут одновременно писать в лог
- Ротация файлов не нарушает целостности данных
