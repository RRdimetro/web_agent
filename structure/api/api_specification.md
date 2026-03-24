# Спецификация API

## Общие сведения

### Базовый URL
```
https://{server_url}/api/{api_version}/
```

### Протокол
- **Транспорт:** HTTPS
- **Формат данных:** JSON
- **Кодировка:** UTF-8

### Заголовки
```
Content-Type: application/json
Accept: application/json
```

---

## 1. Регистрация агента

### Запрос
```
POST /api/v1/agent/register
```

### Тело запроса

```json
{
    "uid": "agent-001",
    "name": "Workstation-01",
    "version": "1.0.0",
    "os": {
        "type": "linux",
        "version": "22.04",
        "architecture": "x86_64"
    },
    "capabilities": ["process_execution", "command_execution", "file_upload"]
}
```

### Параметры запроса

| Поле | Тип | Обязательный | Описание |
|------|-----|--------------|----------|
| uid | string | Да | Уникальный идентификатор агента |
| name | string | Нет | Отображаемое имя агента |
| version | string | Нет | Версия агента |
| os.type | string | Нет | Тип ОС (windows/linux/macos) |
| os.version | string | Нет | Версия ОС |
| os.architecture | string | Нет | Архитектура (x86_64/arm64) |
| capabilities | array | Нет | Список поддерживаемых функций |

### Успешный ответ (200 OK)

```json
{
    "status": "success",
    "session_id": "sess_abc123def456",
    "session_ttl": 86400,
    "server_time": "2024-01-15T10:30:00Z",
    "poll_interval_override": null
}
```

| Поле | Тип | Описание |
|------|-----|----------|
| status | string | Статус операции ("success") |
| session_id | string | Идентификатор сессии |
| session_ttl | int | Время жизни сессии (секунды) |
| server_time | string | Время на сервере (ISO 8601) |
| poll_interval_override | int/null | Переопределение интервала опроса |

### Ошибки

| Код | Описание | Тело ответа | Действие агента |
|-----|----------|-------------|-----------------|
| 400 | Неверный формат запроса | `{"code": "INVALID_REQUEST", "message": "..."}` | Логирование, без retry |
| 401 | Неавторизован | `{"code": "UNAUTHORIZED", "message": "..."}` | - |
| 409 | UID уже зарегистрирован | `{"code": "UID_EXISTS", "message": "..."}` | Логирование, повтор через 60 сек |
| 500 | Внутренняя ошибка сервера | `{"code": "INTERNAL_ERROR", "message": "..."}` | Retry с экспоненциальной задержкой |

---

## 2. Получение задачи

### Запрос
```
GET /api/v1/agent/task?uid={uid}&session_id={session_id}
```

### Параметры запроса

| Параметр | Тип | Обязательный | Описание |
|----------|-----|--------------|----------|
| uid | string | Да | UID агента |
| session_id | string | Да | ID сессии (получен при регистрации) |

### Успешный ответ с задачей (200 OK)

```json
{
    "status": "success",
    "has_task": true,
    "task": {
        "task_id": "task_12345",
        "session_id": "sess_abc123def456",
        "type": "process_execution",
        "priority": 5,
        "created_at": "2024-01-15T10:30:00Z",
        "timeout_sec": 60,
        "parameters": {
            "executable": "/usr/bin/python3",
            "arguments": ["script.py", "--input", "data.txt"],
            "working_directory": "./tasks/task_12345",
            "environment": {
                "PYTHONPATH": "/opt/lib"
            },
            "capture_output": true
        }
    }
}
```

### Успешный ответ без задачи (204 No Content)

```json
{
    "status": "success",
    "has_task": false
}
```

### Типы задач

#### Тип 1: Запуск исполняемого файла (process_execution)

```json
{
    "type": "process_execution",
    "parameters": {
        "executable": "/usr/bin/python3",
        "arguments": ["script.py", "--input", "data.txt"],
        "working_directory": "/work",
        "environment": {"VAR": "value"},
        "capture_output": true,
        "timeout_sec": 120
    }
}
```

#### Тип 2: Выполнение системной команды (command_execution)

```json
{
    "type": "command_execution",
    "parameters": {
        "command": "ls -la /tmp",
        "shell": "/bin/bash",
        "working_directory": "/work",
        "capture_output": true,
        "timeout_sec": 30
    }
}
```

#### Тип 3: Передача файла (file_upload)

```json
{
    "type": "file_upload",
    "parameters": {
        "source_path": "./results/output.txt",
        "destination_url": "https://server.com/upload",
        "compression": "gzip"
    }
}
```

#### Тип 4: Скачивание файла (file_download)

```json
{
    "type": "file_download",
    "parameters": {
        "source_url": "https://server.com/file.zip",
        "destination_path": "./downloads/file.zip",
        "verify_checksum": true,
        "expected_checksum": "sha256:abc123..."
    }
}
```

### Ошибки

| Код | Описание | Действие агента |
|-----|----------|-----------------|
| 401 | Сессия истекла | Повторная регистрация |
| 404 | Агент не найден | Повторная регистрация |
| 429 | Слишком много запросов | Задержка перед следующим опросом |

---

## 3. Отправка результата

### Запрос
```
POST /api/v1/agent/result
```

### Тело запроса

```json
{
    "uid": "agent-001",
    "session_id": "sess_abc123def456",
    "task_id": "task_12345",
    "status": "completed",
    "exit_code": 0,
    "output": "Task executed successfully\n",
    "error": null,
    "duration_ms": 1234,
    "result_files": [
        {
            "name": "output.txt",
            "path": "./results/task_12345/output.txt",
            "size_bytes": 1024,
            "content_type": "text/plain",
            "checksum": "sha256:abc123..."
        }
    ],
    "metrics": {
        "cpu_time_ms": 500,
        "memory_bytes": 10485760
    },
    "completed_at": "2024-01-15T10:32:00Z"
}
```

### Параметры запроса

| Поле | Тип | Обязательный | Описание |
|------|-----|--------------|----------|
| uid | string | Да | UID агента |
| session_id | string | Да | ID сессии |
| task_id | string | Да | ID задачи |
| status | string | Да | Статус выполнения (completed/failed/timeout) |
| exit_code | int | Да | Код возврата (0 - успех) |
| output | string | Нет | Вывод программы (stdout) |
| error | string | Нет | Вывод ошибок (stderr) |
| duration_ms | int | Да | Время выполнения (мс) |
| result_files | array | Нет | Список файлов результата |
| metrics | object | Нет | Метрики выполнения |
| completed_at | string | Да | Время завершения (ISO 8601) |

### Успешный ответ (200 OK)

```json
{
    "status": "success",
    "message": "Result received",
    "task_id": "task_12345"
}
```

### Ошибки

| Код | Описание | Действие агента |
|-----|----------|-----------------|
| 400 | Неверный формат | Логирование, без retry |
| 401 | Сессия истекла | Повторная регистрация |
| 404 | Задача не найдена | Логирование, без retry |
| 500 | Ошибка сервера | Retry с экспоненциальной задержкой |

---

## 4. Отправка心跳 (Heartbeat)

### Запрос
```
POST /api/v1/agent/heartbeat
```

### Тело запроса

```json
{
    "uid": "agent-001",
    "session_id": "sess_abc123def456",
    "timestamp": "2024-01-15T10:35:00Z",
    "status": "idle",
    "active_tasks": 2,
    "queue_size": 0
}
```

### Успешный ответ (200 OK)

```json
{
    "status": "success",
    "session_valid": true,
    "session_ttl_remaining": 86300
}
```

---

## 5. Коды ошибок

| Код | Название | Описание | Обработка агентом |
|-----|----------|----------|-------------------|
| 400 | BAD_REQUEST | Некорректный JSON | Логирование, без повторной отправки |
| 401 | UNAUTHORIZED | Неверный session_id | Повторная регистрация |
| 404 | NOT_FOUND | Ресурс не найден | Логирование, ожидание следующего цикла |
| 408 | REQUEST_TIMEOUT | Таймаут запроса | Повторная отправка |
| 409 | CONFLICT | Конфликт данных | Логирование, ожидание |
| 429 | TOO_MANY_REQUESTS | Слишком много запросов | Экспоненциальная задержка |
| 500 | INTERNAL_ERROR | Внутренняя ошибка сервера | Повторная отправка с задержкой |
| 503 | SERVICE_UNAVAILABLE | Сервис недоступен | Повторная отправка с задержкой |

---

## 6. Примеры полных сценариев

### Сценарий 1: Успешная регистрация и выполнение задачи

**Шаг 1: Регистрация**

```
POST /api/v1/agent/register
{
    "uid": "agent-001",
    "name": "Workstation-01"
}
```

**Ответ:**
```
200 OK
{
    "status": "success",
    "session_id": "sess_abc123",
    "session_ttl": 86400
}
```

**Шаг 2: Получение задачи**

```
GET /api/v1/agent/task?uid=agent-001&session_id=sess_abc123
```

**Ответ:**
```
200 OK
{
    "status": "success",
    "has_task": true,
    "task": {
        "task_id": "task_001",
        "type": "command_execution",
        "parameters": {
            "command": "echo Hello World"
        }
    }
}
```

**Шаг 3: Отправка результата**

```
POST /api/v1/agent/result
{
    "uid": "agent-001",
    "session_id": "sess_abc123",
    "task_id": "task_001",
    "status": "completed",
    "exit_code": 0,
    "output": "Hello World\n",
    "duration_ms": 50,
    "completed_at": "2024-01-15T10:32:00Z"
}
```

**Ответ:**
```
200 OK
{
    "status": "success",
    "message": "Result received"
}
```

### Сценарий 2: Истекшая сессия

**Шаг 1: Попытка получить задачу с истекшей сессией**

```
GET /api/v1/agent/task?uid=agent-001&session_id=expired_session
```

**Ответ:**
```
401 Unauthorized
{
    "status": "error",
    "code": "UNAUTHORIZED",
    "message": "Session expired or invalid"
}
```

**Шаг 2: Агент выполняет повторную регистрацию**

```
POST /api/v1/agent/register
{
    "uid": "agent-001"
}
```

**Ответ:**
```
200 OK
{
    "status": "success",
    "session_id": "sess_new123",
    "session_ttl": 86400
}
```

### Сценарий 3: Ошибка выполнения задачи

**Шаг 1: Отправка результата с ошибкой**

```
POST /api/v1/agent/result
{
    "uid": "agent-001",
    "session_id": "sess_abc123",
    "task_id": "task_002",
    "status": "failed",
    "exit_code": 1,
    "error": "File not found: input.txt",
    "duration_ms": 100,
    "completed_at": "2024-01-15T10:35:00Z"
}
```

**Ответ:**
```
200 OK
{
    "status": "success",
    "message": "Result received"
}
```

---

## 7. Схемы валидации (JSON Schema)

### Схема регистрации

```json
{
    "type": "object",
    "required": ["uid"],
    "properties": {
        "uid": {"type": "string", "minLength": 1},
        "name": {"type": "string"},
        "version": {"type": "string"},
        "os": {
            "type": "object",
            "properties": {
                "type": {"type": "string", "enum": ["windows", "linux", "macos"]},
                "version": {"type": "string"},
                "architecture": {"type": "string"}
            }
        },
        "capabilities": {
            "type": "array",
            "items": {"type": "string"}
        }
    }
}
```

### Схема задачи

```json
{
    "type": "object",
    "required": ["task_id", "type", "parameters"],
    "properties": {
        "task_id": {"type": "string"},
        "session_id": {"type": "string"},
        "type": {"type": "string", "enum": ["process_execution", "command_execution", "file_upload", "file_download"]},
        "priority": {"type": "integer", "minimum": 1, "maximum": 10},
        "timeout_sec": {"type": "integer", "minimum": 1},
        "parameters": {"type": "object"}
    }
}
```

### Схема результата

```json
{
    "type": "object",
    "required": ["uid", "session_id", "task_id", "status", "exit_code", "duration_ms", "completed_at"],
    "properties": {
        "uid": {"type": "string"},
        "session_id": {"type": "string"},
        "task_id": {"type": "string"},
        "status": {"type": "string", "enum": ["completed", "failed", "timeout"]},
        "exit_code": {"type": "integer"},
        "output": {"type": "string"},
        "error": {"type": "string"},
        "duration_ms": {"type": "integer"},
        "completed_at": {"type": "string", "format": "date-time"},
        "result_files": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "name": {"type": "string"},
                    "path": {"type": "string"},
                    "size_bytes": {"type": "integer"},
                    "content_type": {"type": "string"},
                    "checksum": {"type": "string"}
                }
            }
        }
    }
}
```
