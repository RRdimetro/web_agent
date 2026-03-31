# Модуль логирования (Logger)

## Назначение

Потокобезопасное логирование событий агента с записью в файл и выводом в консоль.

---

## Библиотека

**spdlog** — быстрая C++ библиотека логирования

---

## Уровни логирования

| Уровень | Консоль | Файл | Описание |
|---------|---------|------|----------|
| **INFO** | ✅ | ✅ | Важные события (запуск, задачи, результаты) |
| **WARNING** | ✅ | ✅ | Предупреждения (сетевые ошибки, retry) |
| **ERROR** | ✅ | ✅ | Ошибки (неудачные задачи, сбои) |
| **DEBUG** | ❌ | ✅ | Отладочная информация (детали запросов) |

---

## Формат сообщений

### В файле (agent.log)

```
[2026-03-31 17:45:14.388] [web-agent] [info] Сообщение
[2026-03-31 17:45:14.389] [web-agent] [error] Ошибка
```

### Формат строки

```
[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
```

| Спецификатор | Описание |
|--------------|----------|
| `%Y-%m-%d %H:%M:%S.%e` | Дата и время с миллисекундами |
| `%n` | Имя логгера (web-agent) |
| `%l` | Уровень (info/error/warn/debug) |
| `%v` | Текст сообщения |

---

## Класс Logger

```cpp
class Logger {
public:
    // Конструктор
    explicit Logger(const std::filesystem::path& log_file);
    
    // Запись выполнения задания
    void logTask(const Task& task, const ExecutionResult& result);
    
    // Запись ошибки
    void logError(const std::string& task_id, const std::string& error);
    
    // Запись информации
    void logInfo(const std::string& message);
    
    // Запись предупреждения
    void logWarning(const std::string& message);

private:
    std::filesystem::path log_file_;
    std::mutex mutex_;  // Для потокобезопасности
    
    void write(const std::string& level, const std::string& message);
    std::string getTimestamp();
};
```

---

## Примеры использования

### Логирование выполнения задачи

```cpp
Logger logger("./agent.log");

Task task;
task.task_id = "task-001";
task.task_code = "TASK";

ExecutionResult result;
result.success = true;
result.exit_code = 0;
result.output_files = {"output.txt"};

logger.logTask(task, result);
// [2026-03-31 17:45:14.388] [web-agent] [info] Задание task-001 выполнено. 
// Успех: да, Код возврата: 0, Выходные файлы: 1
```

### Логирование ошибки

```cpp
logger.logError("task-001", "Команда не найдена");
// [2026-03-31 17:45:14.389] [web-agent] [error] Задание task-001 не выполнено: Команда не найдена
```

### Информационные сообщения

```cpp
logger.logInfo("WebAgent запущен");
logger.logWarning("Сетевая ошибка, увеличение интервала");
```

---

## Интеграция с spdlog

Logger использует spdlog для дублирования сообщений:

```cpp
void Logger::write(const std::string& level, const std::string& message) {
    // Запись в файл
    std::ofstream file(log_file_, std::ios::app);
    file << "[" << getTimestamp() << "] [" << level << "] " << message << std::endl;
    
    // Дублирование в spdlog
    if (level == "ОШИБКА") {
        spdlog::error(message);
    } else if (level == "ПРЕДУПРЕЖДЕНИЕ") {
        spdlog::warn(message);
    } else {
        spdlog::info(message);
    }
}
```

---

## Настройка в main.cpp

```cpp
// Консольный вывод (info и выше)
auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
console_sink->set_level(spdlog::level::info);

// Файловый вывод (debug и выше)
auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("agent.log", true);
file_sink->set_level(spdlog::level::debug);

// Создание логгера
std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
auto logger = std::make_shared<spdlog::logger>("web-agent", sinks.begin(), sinks.end());
spdlog::register_logger(logger);
spdlog::set_default_logger(logger);
```

---

## Потокобезопасность

Logger использует мьютекс для защиты файла:

```cpp
void Logger::write(...) {
    std::lock_guard<std::mutex> lock(mutex_);  // Блокировка
    
    std::ofstream file(log_file_, std::ios::app);
    file << ... << std::endl;
}
```

---

## Ротация логов

Текущая реализация:
- Один файл `agent.log`
- Перезапись при каждом запуске (режим `true` в `basic_file_sink_mt`)

Для ротации можно использовать `rotating_file_sink`:

```cpp
auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    "agent.log",
    10485760,  // 10 MB
    7          // 7 файлов
);
```

---

## Производительность

- spdlog использует асинхронное логирование
- Минимальное влияние на производительность агента
- Буферизация записи в файл
