# Спецификация модуля файловой системы

## Назначение
Выполнение операций с файлами и директориями: создание, чтение, запись, перемещение, удаление.

## Класс FileManager

```cpp
class FileManager {
public:
    FileManager() = delete;

    // Работа с директориями
    static bool createDirectory(const std::filesystem::path& path);
    static bool createDirectories(const std::filesystem::path& path);
    static bool removeDirectory(const std::filesystem::path& path);
    static bool removeDirectoryRecursive(const std::filesystem::path& path);
    static bool exists(const std::filesystem::path& path);
    static bool isDirectory(const std::filesystem::path& path);
    static bool isFile(const std::filesystem::path& path);

    // Работа с файлами
    static bool moveFile(const std::filesystem::path& src,
                         const std::filesystem::path& dst);
    static bool copyFile(const std::filesystem::path& src,
                         const std::filesystem::path& dst);
    static std::string readFile(const std::filesystem::path& path);
    static std::vector<uint8_t> readBinaryFile(const std::filesystem::path& path);
    static bool writeFile(const std::filesystem::path& path,
                          const std::string& content);
    static bool writeBinaryFile(const std::filesystem::path& path,
                                 const std::vector<uint8_t>& data);
    static bool appendFile(const std::filesystem::path& path,
                           const std::string& content);

    // Информация о файлах
    static uintmax_t getFileSize(const std::filesystem::path& path);
    static std::time_t getLastModified(const std::filesystem::path& path);
    static uintmax_t getFreeSpace(const std::filesystem::path& path);

    // Поиск файлов
    static std::vector<std::filesystem::path> listFiles(
        const std::filesystem::path& directory,
        const std::string& pattern = "",
        bool recursive = false);

    // Временные файлы
    static std::filesystem::path createUniqueDirectory(
        const std::filesystem::path& base,
        const std::string& prefix);

    static std::filesystem::path createUniqueFile(
        const std::filesystem::path& directory,
        const std::string& prefix,
        const std::string& extension);

    // Очистка
    static size_t deleteOldFiles(const std::filesystem::path& directory,
                                  std::chrono::seconds maxAge,
                                  const std::string& pattern = "");
};
```

## Основные операции

**Создание директорий**

```cpp
// Создание одной директории
FileManager::createDirectory("./tasks");

// Создание вложенных директорий
FileManager::createDirectories("./tasks/2024/01/15");
```

**Чтение и запись файлов**

```cpp
// Чтение текстового файла
std::string content = FileManager::readFile("./config.json");

// Запись текстового файла
FileManager::writeFile("./result.txt", "Task completed");

// Чтение бинарного файла
auto data = FileManager::readBinaryFile("./image.png");

// Запись бинарного файла
FileManager::writeBinaryFile("./output.bin", data);
```

**Работа с временными файлами**

```cpp
// Создание уникальной временной директории
auto tempDir = FileManager::createUniqueDirectory("./temp", "task_");
// Результат: ./temp/task_abc123/

// Создание уникального временного файла
auto tempFile = FileManager::createUniqueFile("./temp", "output", ".txt");
// Результат: ./temp/output_xyz789.txt
```

**Поиск файлов**

```cpp
// Поиск всех .log файлов
auto logFiles = FileManager::listFiles("./logs", "*.log");

// Рекурсивный поиск всех .json файлов
auto jsonFiles = FileManager::listFiles("./", "*.json", true);
```

**Очистка старых файлов**

```cpp
// Удаление файлов старше 7 дней
auto deletedCount = FileManager::deleteOldFiles(
    "./logs",
    std::chrono::hours(24 * 7),
    "*.log");
```

## Обработка ошибок

| Ошибка | Действие |
|--------|----------|
| Путь не существует | Логирование ERROR, возврат false |
| Нет прав доступа | Логирование ERROR, возврат false |
| Диск заполнен | Логирование ERROR, возврат false |
| Файл уже существует | Логирование WARNING, перезапись |

## Пример использования

```cpp
// Создание структуры директорий
auto taskDir = FileManager::createUniqueDirectory("./tasks", "task_");
FileManager::createDirectories(taskDir / "input");
FileManager::createDirectories(taskDir / "output");

// Сохранение результата
auto resultPath = taskDir / "output" / "result.txt";
FileManager::writeFile(resultPath, executionResult.stdout);

// Перемещение в постоянную директорию
FileManager::moveFile(resultPath, "./results/" + taskId + ".txt");

// Очистка временной директории
FileManager::removeDirectoryRecursive(taskDir);
```
