# Модуль выполнения задач (TaskExecutor)

## Назначение

Выполнение задач, полученных от сервера:
- FILE — поиск и отправка файлов
- TASK — выполнение команд
- CONF — изменение конфигурации
- TIMEOUT — изменение интервала опроса

---

## Типы задач

```cpp
enum class TaskType {
    None,           // Нет задания
    ExecuteCommand, // Выполнить команду (TASK)
    RunProgram,     // Запустить программу
    TransferFile,   // Передать файл (FILE)
    ChangeConfig,   // Изменить конфиг (CONF)
    ChangeTimeout   // Изменить интервал (TIMEOUT)
};
```

---

## Структура Task

```cpp
struct Task {
    TaskType type = TaskType::None;
    std::string task_code;         // Код задачи: FILE, TASK, CONF, TIMEOUT
    std::string task_id;           // ID задачи
    std::string session_id;        // ID сессии
    std::string command;           // Команда для выполнения
    std::string program_path;      // Путь к программе
    std::vector<std::string> arguments;  // Аргументы
    std::vector<std::string> source_files;  // Исходные файлы
    std::string destination_folder;  // Папка назначения
    std::string options;           // JSON с параметрами
    std::chrono::seconds timeout{300};  // Таймаут
};
```

---

## Структура ExecutionResult

```cpp
struct ExecutionResult {
    bool success;                               // Успех выполнения
    int exit_code;                              // Код возврата
    std::string error_message;                  // Сообщение об ошибке
    std::vector<std::filesystem::path> output_files;  // Выходные файлы
};
```

---

## Класс TaskExecutor

```cpp
class TaskExecutor {
public:
    // Конструктор (требует Config для CONF/TIMEOUT)
    TaskExecutor(std::shared_ptr<Config> config);
    
    // Выполнить задание
    ExecutionResult executeWithConfig(const Task& task, 
                                       const std::filesystem::path& results_folder);
    
    // Статический метод (без изменения конфига)
    static ExecutionResult execute(const Task& task,
                                    const std::filesystem::path& results_folder);

private:
    std::shared_ptr<Config> config_;
    
    static ExecutionResult executeCommand(const Task& task, ...);
    static ExecutionResult runProgram(const Task& task, ...);
    static ExecutionResult transferFiles(const Task& task, ...);
    ExecutionResult changeConfig(const Task& task);
    ExecutionResult changeTimeout(const Task& task);
};
```

---

## Обработка типов задач

### 1. FILE — Поиск и отправка файла

**Входные данные:**
```json
{"filename": "config.json"}
```

**Логика:**
```cpp
ExecutionResult TaskExecutor::transferFiles(...) {
    // 1. Найти файл в source_files
    // 2. Скопировать в destination_folder
    // 3. Вернуть путь к скопированному файлу
}
```

**Результат:**
- success = true, если файл найден и скопирован
- output_files содержит путь к файлу

---

### 2. TASK — Выполнить команду

**Входные данные:**
```json
{"command": "uname -a"}
```

**Логика:**
```cpp
ExecutionResult TaskExecutor::executeCommand(...) {
    // 1. Выполнить команду через std::system()
    // 2. Зафиксировать код возврата
    // 3. Вернуть результат
}
```

**Результат:**
- success = true, если exit_code == 0
- error_message содержит вывод ошибки

---

### 3. CONF — Изменить конфигурацию

**Входные данные:**
```json
{"key": "poll_interval_sec", "value": "30"}
```

**Поддерживаемые ключи:**
- `poll_interval_sec` — интервал опроса
- `task_timeout` — таймаут задачи
- `max_poll_interval` — макс. интервал при ошибках
- `max_retries` — макс. попыток
- `retry_delay` — задержка между попытками

**Логика:**
```cpp
ExecutionResult TaskExecutor::changeConfig(...) {
    auto config_json = json::parse(task.options);
    std::string key = config_json["key"];
    std::string value = config_json["value"];
    
    if (key == "poll_interval_sec") {
        config_->poll_interval = std::stoi(value);
    }
    // ... другие ключи
}
```

---

### 4. TIMEOUT — Изменить интервал опроса

**Входные данные:**
```json
{"interval": "15"}
```

**Логика:**
```cpp
ExecutionResult TaskExecutor::changeTimeout(...) {
    auto json = json::parse(task.options);
    int new_interval = std::stoi(json["interval"]);
    config_->poll_interval = std::chrono::seconds(new_interval);
}
```

---

## Пример использования

```cpp
// Создание TaskExecutor
auto config = std::make_shared<Config>(Config::load("config.json"));
TaskExecutor executor(config);

// Создание задачи
Task task;
task.task_code = "TASK";
task.type = TaskType::ExecuteCommand;
task.command = "uname -a";
task.session_id = "session-123";

// Выполнение
ExecutionResult result = executor.executeWithConfig(task, "./results");

// Обработка результата
if (result.success) {
    spdlog::info("Задание выполнено, код: {}", result.exit_code);
} else {
    spdlog::error("Ошибка: {}", result.error_message);
}
```

---

## Обработка ошибок

| Ошибка | Возврат |
|--------|---------|
| Файл не найден (FILE) | success = false, error_message |
| Команда не найдена (TASK) | success = false, exit_code != 0 |
| Неверный JSON (CONF/TIMEOUT) | success = false, error_message |
| Неизвестный ключ (CONF) | success = false, error_message |

---

## Безопасность

- Валидация путей к файлам
- Ограничение таймаута выполнения
- Захват stdout/stderr для отладки
- Логирование всех операций
