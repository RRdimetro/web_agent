# Модуль файловой системы

## Назначение

Операции с файлами и директориями при выполнении задач типа **FILE**.

---

## Функции

### Копирование файлов

```cpp
ExecutionResult TaskExecutor::transferFiles(...) {
    std::filesystem::create_directories(task.destination_folder);
    
    for (const auto& source : task.source_files) {
        std::filesystem::path src(source);
        std::filesystem::path dest = task.destination_folder / src.filename();
        
        std::filesystem::copy(src, dest, 
            std::filesystem::copy_options::overwrite_existing);
    }
}
```

### Поиск файлов

```cpp
std::vector<std::filesystem::path> TaskExecutor::findNewFiles(
    const std::filesystem::path& folder,
    std::chrono::system_clock::time_point since) 
{
    std::vector<std::filesystem::path> new_files;
    
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (std::filesystem::is_regular_file(entry)) {
            auto write_time = std::filesystem::last_write_time(entry);
            // Конвертация и сравнение времени
            if (write_time_sys > since) {
                new_files.push_back(entry.path());
            }
        }
    }
    
    return new_files;
}
```

### Создание директорий

```cpp
std::filesystem::create_directories(path);
```

---

## Используемые пути

| Путь | Назначение | По умолчанию |
|------|------------|--------------|
| `tasks_folder` | Хранение задач | `./tasks` |
| `results_folder` | Результаты выполнения | `./results` |
| `log_file` | Файл логов | `./agent.log` |

---

## Обработка ошибок

| Ошибка | Обработка |
|--------|-----------|
| Файл не найден | Логирование ERROR, возврат ошибки |
| Нет прав на запись | Логирование ERROR, возврат ошибки |
| Директория не существует | Попытка создания |
| Файл заблокирован | Логирование ERROR, возврат ошибки |

---

## Пример: задача FILE

**Запрос от сервера:**
```json
{
    "task_code": "FILE",
    "options": "{\"filename\":\"config.json\"}",
    "session_id": "..."
}
```

**Действия агента:**
1. Парсинг `options` → `filename = "config.json"`
2. Поиск файла в рабочей директории
3. Копирование в `results_folder`
4. Отправка пути к файлу на сервер

**Код:**
```cpp
Task task;
task.type = TaskType::TransferFile;
task.source_files = {"config.json"};
task.destination_folder = "./results";

ExecutionResult result = TaskExecutor::execute(task, "./results");

if (result.success) {
    // result.output_files содержит путь к скопированному файлу
}
```

---

## Безопасность

### Проверка путей

```cpp
// Запрет выхода за пределы рабочей директории
if (source.string().find("..") != std::string::npos) {
    spdlog::error("Недопустимый путь: {}", source);
    return {false, -1, "Invalid path", {}};
}
```

### Ограничение размера файлов

```cpp
constexpr size_t MAX_FILE_SIZE = 100 * 1024 * 1024;  // 100 MB

auto size = std::filesystem::file_size(source);
if (size > MAX_FILE_SIZE) {
    spdlog::error("Файл слишком большой: {} bytes", size);
    return {false, -1, "File too large", {}};
}
```

---

## Временные файлы

При выполнении задач могут создаваться временные файлы:

```cpp
// Создание временной директории
auto temp_dir = std::filesystem::temp_directory_path() / "web-agent";
std::filesystem::create_directories(temp_dir);

// Очистка после выполнения
std::filesystem::remove_all(temp_dir);
```

---

## Производительность

- Использование `std::filesystem` (C++17)
- Минимальное количество системных вызовов
- Кэширование результатов `directory_iterator`
