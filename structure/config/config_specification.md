# Проектирование формата конфигурации

## 1. Выбор формата

**Выбран формат: JSON**

### Обоснование выбора:

| Критерий | JSON | YAML | XML |
|----------|------|------|-----|
| Читаемость | Хорошая | Отличная | Средняя |
| Компактность | Высокая | Высокая | Низкая |
| Поддержка в C++ | Отличная | Средняя | Хорошая |
| Скорость парсинга | Высокая | Средняя | Низкая |
| Использование в REST API | Стандарт | Редко | Редко |

### Причины выбора JSON:
1. Нативная поддержка в REST API (протокол взаимодействия с сервером)
2. Высокая производительность парсинга (библиотека nlohmann/json)
3. Простота редактирования человеком
4. Компактный размер файла
5. Возможность валидации через JSON Schema

---

## 2. Структура конфигурационного файла

```json
{
    "agent": {
        "uid": "agent-001",
        "name": "Workstation-01",
        "description": "Основной рабочий агент"
    },
    "server": {
        "url": "https://command-server.example.com",
        "api_version": "v1",
        "connection_timeout_sec": 30,
        "request_timeout_sec": 60,
        "retry_count": 3,
        "retry_delay_sec": 5,
        "verify_ssl": true,
        "ssl_cert_path": "/etc/ssl/certs/ca-certificates.crt"
    },
    "polling": {
        "interval_sec": 10,
        "jitter_percent": 20,
        "max_tasks_per_poll": 5
    },
    "paths": {
        "task_directory": "./tasks",
        "result_directory": "./results",
        "temp_directory": "./temp",
        "log_file": "./logs/agent.log"
    },
    "execution": {
        "default_timeout_sec": 300,
        "max_concurrent_tasks": 4,
        "working_directory": "./work",
        "cleanup_temp": true
    },
    "logging": {
        "level": "info",
        "console_output": true,
        "file_rotation_size_mb": 10,
        "file_rotation_max_files": 7,
        "format": "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v"
    },
    "security": {
        "allowed_commands": [
            "echo",
            "ls",
            "dir",
            "ping",
            "whoami"
        ],
        "blocked_paths": [
            "/etc",
            "/boot",
            "C:\\Windows\\System32"
        ],
        "max_file_size_mb": 100
    }
}
```

---

## 3. Описание параметров конфигурации

### Секция agent

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| uid | string | Да | Уникальный идентификатор агента | - |
| name | string | Нет | Отображаемое имя агента | Значение uid |
| description | string | Нет | Описание агента | "" |

### Секция server

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| url | string | Да | URL сервера (http:// или https://) | - |
| api_version | string | Нет | Версия API | "v1" |
| connection_timeout_sec | int | Нет | Таймаут соединения (секунды) | 30 |
| request_timeout_sec | int | Нет | Таймаут запроса (секунды) | 60 |
| retry_count | int | Нет | Количество повторных попыток | 3 |
| retry_delay_sec | int | Нет | Задержка между попытками (секунды) | 5 |
| verify_ssl | bool | Нет | Проверять SSL сертификат | true |
| ssl_cert_path | string | Нет | Путь к CA сертификатам | Системный путь |

### Секция polling

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| interval_sec | int | Да | Интервал опроса сервера (секунды) | - |
| jitter_percent | int | Нет | Случайное отклонение интервала (%) | 0 |
| max_tasks_per_poll | int | Нет | Максимум задач за один опрос | 1 |

### Секция paths

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| task_directory | string | Да | Директория для хранения задач | - |
| result_directory | string | Да | Директория для результатов | - |
| temp_directory | string | Нет | Временная директория | "./temp" |
| log_file | string | Да | Путь к файлу лога | - |

### Секция execution

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| default_timeout_sec | int | Нет | Таймаут выполнения задачи (сек) | 300 |
| max_concurrent_tasks | int | Нет | Максимум параллельных задач | 1 |
| working_directory | string | Нет | Рабочая директория по умолчанию | "./work" |
| cleanup_temp | bool | Нет | Очищать временные файлы | true |

### Секция logging

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| level | string | Нет | Уровень логирования | "info" |
| console_output | bool | Нет | Вывод в консоль | false |
| file_rotation_size_mb | int | Нет | Размер ротации файла (MB) | 10 |
| file_rotation_max_files | int | Нет | Максимум файлов лога | 7 |
| format | string | Нет | Формат сообщения лога | spdlog default |

**Допустимые значения уровня логирования:**
- `trace` - детальная отладка
- `debug` - отладочная информация
- `info` - информационные сообщения
- `warning` - предупреждения
- `error` - ошибки
- `critical` - критические ошибки
- `off` - логирование отключено

### Секция security

| Параметр | Тип | Обязательный | Описание | Значение по умолчанию |
|----------|-----|--------------|----------|---------------------|
| allowed_commands | array | Нет | Белый список разрешенных команд | [] |
| blocked_paths | array | Нет | Черный список запрещенных путей | [] |
| max_file_size_mb | int | Нет | Максимальный размер файла (MB) | 100 |

---

## 4. Примеры конфигурации

### Пример 1: Минимальная конфигурация

```json
{
    "agent": {
        "uid": "agent-001"
    },
    "server": {
        "url": "https://server.example.com"
    },
    "polling": {
        "interval_sec": 10
    },
    "paths": {
        "task_directory": "./tasks",
        "result_directory": "./results",
        "log_file": "./agent.log"
    }
}
```

### Пример 2: Полная конфигурация с безопасностью

```json
{
    "agent": {
        "uid": "secure-agent-01",
        "name": "Production Worker",
        "description": "Агент на production сервере"
    },
    "server": {
        "url": "https://secure-server.example.com",
        "api_version": "v2",
        "connection_timeout_sec": 15,
        "request_timeout_sec": 30,
        "retry_count": 5,
        "retry_delay_sec": 2,
        "verify_ssl": true,
        "ssl_cert_path": "/etc/ssl/certs/custom-ca.crt"
    },
    "polling": {
        "interval_sec": 5,
        "jitter_percent": 10,
        "max_tasks_per_poll": 3
    },
    "paths": {
        "task_directory": "/var/web-agent/tasks",
        "result_directory": "/var/web-agent/results",
        "temp_directory": "/tmp/web-agent",
        "log_file": "/var/log/web-agent/agent.log"
    },
    "execution": {
        "default_timeout_sec": 600,
        "max_concurrent_tasks": 8,
        "working_directory": "/var/web-agent/work",
        "cleanup_temp": true
    },
    "logging": {
        "level": "info",
        "console_output": false,
        "file_rotation_size_mb": 50,
        "file_rotation_max_files": 14,
        "format": "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v"
    },
    "security": {
        "allowed_commands": [
            "echo",
            "cat",
            "grep",
            "awk",
            "sed",
            "python3",
            "bash"
        ],
        "blocked_paths": [
            "/etc/shadow",
            "/etc/passwd",
            "/root",
            "C:\\Windows"
        ],
        "max_file_size_mb": 500
    }
}
```

### Пример 3: Конфигурация для Windows

```json
{
    "agent": {
        "uid": "win-agent-001",
        "name": "Windows Workstation"
    },
    "server": {
        "url": "https://server.example.com",
        "verify_ssl": true
    },
    "polling": {
        "interval_sec": 15
    },
    "paths": {
        "task_directory": "C:\\WebAgent\\tasks",
        "result_directory": "C:\\WebAgent\\results",
        "temp_directory": "C:\\WebAgent\\temp",
        "log_file": "C:\\WebAgent\\logs\\agent.log"
    },
    "execution": {
        "default_timeout_sec": 120,
        "max_concurrent_tasks": 2
    },
    "logging": {
        "level": "debug",
        "console_output": true
    },
    "security": {
        "allowed_commands": [
            "ping",
            "ipconfig",
            "whoami",
            "dir"
        ],
        "blocked_paths": [
            "C:\\Windows\\System32\\config",
            "C:\\Windows\\System32\\drivers"
        ]
    }
}
```

---

## 5. Валидация конфигурации

### Обязательные поля для проверки:
- `agent.uid` - не пустая строка
- `server.url` - валидный URL (http:// или https://)
- `polling.interval_sec` - положительное число
- `paths.task_directory` - не пустая строка
- `paths.result_directory` - не пустая строка
- `paths.log_file` - не пустая строка

### Правила валидации:
1. Все обязательные поля должны присутствовать
2. Числовые поля должны быть в допустимых диапазонах
3. Пути должны быть доступны для записи
4. URL должен быть корректным
5. Уровень логирования должен быть из списка допустимых

### Обработка ошибок валидации:

| Ошибка | Действие |
|--------|----------|
| Отсутствует обязательное поле | Логирование ERROR, выход с кодом 1 |
| Некорректный тип данных | Логирование ERROR с указанием поля |
| Путь не доступен | Логирование WARNING, попытка создания |
| Некорректный URL | Логирование ERROR, выход с кодом 1 |
| Некорректный уровень логирования | Использование "info" по умолчанию |
