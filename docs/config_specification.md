# Спецификация конфигурации Web Agent

## 1. Формат файла

**Выбран формат: JSON**

### Обоснование:
- Простота чтения и редактирования
- Нативная поддержка в REST API
- Высокая производительность парсинга (nlohmann/json)
- Компактный размер

---

## 2. Структура файла

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

## 3. Параметры конфигурации

| Параметр | Тип | Обязательный | По умолчанию | Описание |
|----------|-----|--------------|--------------|----------|
| `uid` | string | Да | - | Уникальный идентификатор агента |
| `server_url` | string | Да | - | URL сервера (https://...) |
| `access_code` | string | Нет | "" | Код доступа (получается при регистрации) |
| `tasks_folder` | path | Нет | "./tasks" | Папка для хранения задач |
| `results_folder` | path | Нет | "./results" | Папка для результатов |
| `log_file` | path | Нет | "./agent.log" | Путь к файлу логов |
| `poll_interval` | int | Нет | 5 | Интервал опроса сервера (сек) |
| `max_poll_interval` | int | Нет | 300 | Максимальный интервал при ошибках (сек) |
| `timeout` | int | Нет | 30 | Таймаут HTTP запросов (сек) |
| `max_retries` | int | Нет | 3 | Максимум попыток при ошибке |
| `retry_delay` | int | Нет | 5 | Задержка между попытками (сек) |

---

## 4. Примеры конфигурации

### 4.1 Минимальная конфигурация

```json
{
    "uid": "007",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api"
}
```

Остальные параметры устанавливаются в значения по умолчанию.

---

### 4.2 Полная конфигурация

```json
{
    "uid": "agent-office_clerk_v4",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "access_code": "dced4c-5e23-42b7-c67f-366c61b2",
    "tasks_folder": "./tasks",
    "results_folder": "./results",
    "log_file": "./agent.log",
    "poll_interval": 10,
    "max_poll_interval": 300,
    "timeout": 30,
    "max_retries": 3,
    "retry_delay": 5
}
```

---

### 4.3 Конфигурация для быстрой отладки

```json
{
    "uid": "test-agent",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "poll_interval": 2,
    "timeout": 10,
    "max_retries": 5
}
```

---

## 5. Валидация

### Обязательные поля:
- `uid` — не пустая строка
- `server_url` — валидный HTTPS URL

### Правила валидации:

| Поле | Проверка |
|------|----------|
| `uid` | Не пустое, уникальное |
| `server_url` | Начинается с `https://` |
| `poll_interval` | > 0, <= max_poll_interval |
| `max_poll_interval` | > poll_interval |
| `timeout` | > 0 |
| `max_retries` | >= 0 |
| `retry_delay` | > 0 |

---

## 6. Изменение конфигурации "на лету"

Агент поддерживает динамическое изменение параметров через задачу **CONF**:

```json
{
    "task_code": "CONF",
    "options": "{\"key\":\"poll_interval_sec\",\"value\":\"30\"}"
}
```

### Поддерживаемые ключи:

| Ключ | Описание |
|------|----------|
| `poll_interval_sec` | Интервал опроса сервера |
| `task_timeout` | Таймаут выполнения задачи |
| `max_poll_interval` | Макс. интервал при ошибках |
| `max_retries` | Макс. попыток при ошибке |
| `retry_delay` | Задержка между попытками |

---

## 7. Расположение файла

### Рекомендуемые пути:

| ОС | Путь |
|----|------|
| Linux | `./config/agent_config.json` |
| Windows | `config\\agent_config.json` |

### Запуск с указанием конфига:

```bash
./web-agent -c config/agent_config.json
```

---

## 8. Шаблон конфигурации

Файл `config/agent_config.example.json` содержит шаблон для копирования:

```bash
cp config/agent_config.example.json config/agent_config.json
```

После копирования отредактируйте:
1. `uid` — получите на сайте в Web Agent Settings
2. `server_url` — URL вашего сервера
3. `access_code` — заполняется автоматически при регистрации (или вручную)

---

## 9. Обработка ошибок

| Ошибка | Действие |
|--------|----------|
| Файл не найден | Логирование ERROR, выход с кодом 1 |
| Некорректный JSON | Логирование ERROR, выход с кодом 1 |
| Отсутствует uid | Логирование ERROR, выход с кодом 1 |
| Отсутствует server_url | Логирование ERROR, выход с кодом 1 |
| Некорректный URL | Логирование ERROR, выход с кодом 1 |
| Недоступна папка для записи | Попытка создания, WARNING |
