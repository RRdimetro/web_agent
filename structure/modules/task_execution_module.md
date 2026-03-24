# Спецификация модуля выполнения задач

## Назначение
Запуск и управление процессами и системными командами, сбор результатов выполнения.

## Типы задач

| Тип | Описание | Исполнитель |
|-----|----------|-------------|
| PROCESS_EXECUTION | Запуск исполняемого файла | ProcessExecutor |
| COMMAND_EXECUTION | Выполнение системной команды | CommandExecutor |

## Интерфейс ITaskExecutor

```cpp
class ITaskExecutor {
public:
    virtual ~ITaskExecutor() = default;

    virtual ExecutionResult execute(const nlohmann::json& params,
                                     const std::filesystem::path& workingDir) = 0;

    virtual void cancel() = 0;
    virtual bool isRunning() const = 0;
    virtual TaskType getType() const = 0;
};
```

## Класс ProcessExecutor

```cpp
class ProcessExecutor : public ITaskExecutor {
public:
    ProcessExecutor();
    explicit ProcessExecutor(int timeoutSeconds);

    ExecutionResult execute(const nlohmann::json& params,
                             const std::filesystem::path& workingDir) override;

    void cancel() override;
    bool isRunning() const override;
    TaskType getType() const override;

private:
    struct ProcessResult {
        int exitCode;
        std::string stdout;
        std::string stderr;
    };

    ProcessResult runExecutable(const std::string& path,
                                 const std::vector<std::string>& args,
                                 const std::filesystem::path& cwd);

    int waitForProcess(pid_t pid, std::chrono::seconds timeout);
    bool killProcess(pid_t pid);

    std::atomic<bool> m_cancelled{false};
    std::atomic<pid_t> m_processId{0};
    int m_timeoutSeconds;
};
```

## Параметры ProcessExecutor

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| executable | string | Да | Путь к исполняемому файлу |
| arguments | array | Нет | Аргументы командной строки |
| working_directory | string | Нет | Рабочая директория |
| environment | object | Нет | Переменные окружения |
| timeout_sec | int | Нет | Таймаут выполнения |
| capture_output | bool | Да | Захват вывода (true/false) |

## Класс CommandExecutor

```cpp
class CommandExecutor : public ITaskExecutor {
public:
    CommandExecutor();
    CommandExecutor(int timeoutSeconds, const std::vector<std::string>& allowedCommands);

    ExecutionResult execute(const nlohmann::json& params,
                             const std::filesystem::path& workingDir) override;

    void cancel() override;
    bool isRunning() const override;
    TaskType getType() const override;

    void setAllowedCommands(const std::vector<std::string>& commands);
    void setShell(const std::string& shell);

private:
    bool validateCommand(const std::string& cmd);
    std::string sanitizeCommand(const std::string& cmd);
    ProcessResult executeShell(const std::string& cmd, const std::filesystem::path& cwd);

    std::atomic<bool> m_cancelled{false};
    int m_timeoutSeconds;
    std::vector<std::string> m_allowedCommands;
    std::string m_shell;
};
```

## Параметры CommandExecutor

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| command | string | Да | Команда для выполнения |
| shell | string | Нет | Интерпретатор команд |
| working_directory | string | Нет | Рабочая директория |
| timeout_sec | int | Нет | Таймаут выполнения |

## Класс TaskRunner

```cpp
class TaskRunner {
public:
    TaskRunner();
    ~TaskRunner();

    void initialize(int threadCount);
    void shutdown();

    std::future<ExecutionResult> submitTask(const Task& task,
                                             std::shared_ptr<ITaskExecutor> executor);

    void waitForAll();
    bool cancelTask(const std::string& taskId);
    int cancelAllTasks();

    int getActiveCount() const;
    int getQueueSize() const;

private:
    void workerFunction();

    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
```

## Структура Task

```cpp
struct Task {
    std::string taskId;
    std::string sessionId;
    TaskType type;
    int priority;
    nlohmann::json parameters;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::seconds timeout;
    std::map<std::string, std::string> environment;

    nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& json);
    bool validate() const;
};
```

## Структура ExecutionResult

```cpp
struct ExecutionResult {
    int exitCode;
    std::string stdout;
    std::string stderr;
    std::vector<std::filesystem::path> resultFiles;
    std::chrono::milliseconds durationMs;
    bool success;
    std::string errorMessage;

    nlohmann::json toJson() const;
    static ExecutionResult fromJson(const nlohmann::json& json);
};
```

## Пример использования

```cpp
auto executor = std::make_shared<ProcessExecutor>(60);

Task task;
task.taskId = "task-001";
task.type = TaskType::PROCESS_EXECUTION;
task.parameters = {
    {"executable", "/usr/bin/python3"},
    {"arguments", {"script.py", "--input", "data.txt"}}
};

TaskRunner runner;
runner.initialize(4);
auto future = runner.submitTask(task, executor);

auto result = future.get();
if (result.success) {
    std::cout << "Output: " << result.stdout << std::endl;
} else {
    std::cerr << "Error: " << result.errorMessage << std::endl;
}
```
