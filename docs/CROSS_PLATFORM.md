# Кроссплатформенная сборка Web Agent

## Поддерживаемые платформы

| Платформа | Статус | Примечания |
|-----------|--------|------------|
| **Linux** | ✅ Полная | Ubuntu 20.04+, Debian 11+, Fedora 35+, Arch Linux |
| **Windows** | ✅ Полная | Windows 10/11, MSVC 2019+, MinGW-w64 |
| **macOS** | ✅ Полная | macOS 11+, Xcode 13+, Homebrew |

---

## Зависимости

### Общие для всех платформ

| Библиотека | Версия | Назначение |
|------------|--------|------------|
| **CMake** | 3.14+ | Система сборки |
| **C++17** | | Стандарт языка |
| **cpr** | 1.10.5 | HTTP клиент (загружается автоматически) |
| **nlohmann/json** | 3.11.2 | JSON парсинг (загружается автоматически) |
| **spdlog** | 1.12.0 | Логирование (загружается автоматически) |
| **libcurl** | 7.68+ | HTTP/HTTPS (системная) |
| **OpenSSL** | 1.1.1+ | SSL/TLS (системная) |

---

## Сборка на Linux

### 1. Установка зависимостей

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y cmake g++ libcurl4-openssl-dev libssl-dev git
```

**Fedora:**
```bash
sudo dnf install -y cmake gcc-c++ libcurl-devel openssl-devel git
```

**Arch Linux:**
```bash
sudo pacman -S --noconfirm cmake gcc curl openssl git
```

### 2. Сборка

```bash
cd web_agent
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Запуск
./web-agent
```

### 3. Сборка с тестами

```bash
cmake -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
./tests
```

---

## Сборка на Windows

### Вариант 1: Visual Studio 2019/2022

**1. Установка зависимостей:**
- Visual Studio 2019/2022 с компонентом "C++ Desktop Development"
- CMake (входит в VS 2019 16.4+)
- Git for Windows

**2. Сборка в Visual Studio:**
```powershell
# Открыть Developer Command Prompt
cd C:\path\to\web_agent
mkdir build && cd build

# Генерация решения Visual Studio
cmake -G "Visual Studio 16 2019" -A x64 ..

# Или для VS 2022
cmake -G "Visual Studio 17 2022" -A x64 ..

# Открыть в Visual Studio
start WebAgent.sln

# Собрать в Visual Studio (Ctrl+Shift+B)
```

**3. Сборка через командную строку:**
```powershell
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

# Запуск
.\Release\web-agent.exe
```

### Вариант 2: MinGW-w64

**1. Установка:**
```powershell
# Через Chocolatey
choco install mingw cmake git

# Или через winget
winget install MSYS2.MSYS2
```

**2. Сборка:**
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make -j4

# Запуск
.\web-agent.exe
```

---

## Сборка на macOS

### 1. Установка зависимостей

```bash
# Установка Xcode Command Line Tools
xcode-select --install

# Установка Homebrew (если не установлен)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Установка зависимостей через Homebrew
brew install cmake openssl curl git
```

### 2. Сборка

```bash
cd web_agent
mkdir build && cd build

# С указанием пути к OpenSSL (если требуется)
cmake -DCMAKE_BUILD_TYPE=Release \
      -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) \
      ..

make -j$(sysctl -n hw.ncpu)

# Запуск
./web-agent
```

### 3. Сборка с тестами

```bash
cmake -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release \
      -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) \
      ..
make -j$(sysctl -n hw.ncpu)
./tests
```

---

## Кроссплатформенные особенности

### Пути к файлам

Агент использует `std::filesystem`, который автоматически обрабатывает разделители путей:

| Платформа | Разделитель | Пример |
|-----------|-------------|--------|
| Linux/Unix | `/` | `./config/agent_config.json` |
| Windows | `\` | `config\agent_config.json` |
| macOS | `/` | `./config/agent_config.json` |

**В коде:**
```cpp
// Кроссплатформенно
std::filesystem::path config_path = "config" / "agent_config.json";
```

### Выполнение команд

| Платформа | Команды | Shell |
|-----------|---------|-------|
| Linux | `ls`, `grep`, `awk` | `/bin/bash` |
| Windows | `dir`, `ping`, `ipconfig` | `cmd.exe` |
| macOS | `ls`, `grep`, `uname` | `/bin/bash` |

**В коде:**
```cpp
#ifdef _WIN32
    // Windows команды
    task.command = "dir";
#else
    // Unix-подобные (Linux, macOS)
    task.command = "ls -la";
#endif
```

### Переменные окружения

| Платформа | Синтаксис | Пример |
|-----------|-----------|--------|
| Linux/macOS | `$VAR` | `echo $HOME` |
| Windows | `%VAR%` | `echo %USERPROFILE%` |

---

## Конфигурация для разных платформ

### Linux (`config/agent_config.json`)
```json
{
    "uid": "linux-agent-01",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "tasks_folder": "./tasks",
    "results_folder": "./results",
    "log_file": "./agent.log",
    "poll_interval": 5
}
```

### Windows (`config\agent_config.json`)
```json
{
    "uid": "windows-agent-01",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "tasks_folder": ".\\tasks",
    "results_folder": ".\\results",
    "log_file": ".\\agent.log",
    "poll_interval": 5
}
```

### macOS (`config/agent_config.json`)
```json
{
    "uid": "macos-agent-01",
    "server_url": "https://xdev.arkcom.ru:9999/app/webagent1/api",
    "tasks_folder": "./tasks",
    "results_folder": "./results",
    "log_file": "./agent.log",
    "poll_interval": 5
}
```

---

## Запуск как сервис/демон

### Linux (systemd)

**1. Создать файл сервиса:**
```ini
# /etc/systemd/system/web-agent.service
[Unit]
Description=Web Agent Service
After=network.target

[Service]
Type=simple
User=webagent
WorkingDirectory=/opt/web-agent
ExecStart=/opt/web-agent/build/web-agent
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

**2. Включить и запустить:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable web-agent
sudo systemctl start web-agent
sudo systemctl status web-agent
```

### Windows (Task Scheduler)

**1. Создать .bat файл:**
```batch
@echo off
cd C:\web-agent\build
web-agent.exe
```

**2. Добавить в Task Scheduler:**
- Открыть Task Scheduler
- Create Basic Task
- Trigger: At startup
- Action: Start a program → `web-agent.bat`

### macOS (launchd)

**1. Создать plist файл:**
```xml
<!-- ~/Library/LaunchAgents/com.webagent.agent.plist -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" 
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.webagent.agent</string>
    <key>ProgramArguments</key>
    <array>
        <string>/Users/user/web-agent/build/web-agent</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

**2. Загрузить:**
```bash
launchctl load ~/Library/LaunchAgents/com.webagent.agent.plist
launchctl list | grep webagent
```

---

## Тестирование кроссплатформенности

### Проверка сборки

```bash
# Linux
mkdir build-linux && cd build-linux
cmake -DBUILD_TESTS=ON ..
make -j4 && ./tests

# Windows (PowerShell)
mkdir build-win && cd build-win
cmake -DBUILD_TESTS=ON -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
.\Release\tests.exe

# macOS
mkdir build-mac && cd build-mac
cmake -DBUILD_TESTS=ON ..
make -j$(sysctl -n hw.ncpu) && ./tests
```

### Таблица совместимости

| Функция | Linux | Windows | macOS |
|---------|-------|---------|-------|
| **Сборка** | ✅ | ✅ | ✅ |
| **HTTP/HTTPS** | ✅ | ✅ | ✅ |
| **Выполнение команд** | ✅ | ✅ | ✅ |
| **Работа с файлами** | ✅ | ✅ | ✅ |
| **Логирование** | ✅ | ✅ | ✅ |
| **Тесты** | ✅ | ✅ | ✅ |
| **Сервис** | systemd | Task Scheduler | launchd |

---

## Устранение проблем

### Linux

**Ошибка: `libcurl not found`**
```bash
sudo apt install libcurl4-openssl-dev
```

**Ошибка: `SSL certificate problem`**
```bash
# В конфиге добавить
"verify_ssl": false
```

### Windows

**Ошибка: `MSVCP140.dll missing`**
```powershell
# Установить Visual C++ Redistributable
choco install vcredist2019
```

**Ошибка: `CMake generator not found`**
```powershell
# Использовать правильный генератор
cmake -G "Visual Studio 16 2019" -A x64 ..
```

### macOS

**Ошибка: `OpenSSL not found`**
```bash
brew install openssl
cmake -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) ..
```

**Ошибка: `Certificate verification failed`**
```bash
# Обновить сертификаты
/Applications/Python\ 3.*/Install\ Certificates.command
```

---

## Архитектурные решения для кроссплатформенности

### 1. Абстракция выполнения команд

```cpp
#ifdef _WIN32
    #include <windows.h>
    #define PLATFORM_NAME "Windows"
#else
    #include <sys/wait.h>
    #include <unistd.h>
    #ifdef __APPLE__
        #define PLATFORM_NAME "macOS"
    #else
        #define PLATFORM_NAME "Linux"
    #endif
#endif
```

### 2. Кроссплатформенные пути

```cpp
// Правильно
auto path = std::filesystem::current_path() / "config" / "agent_config.json";

// Неправильно (только для Unix)
auto path = "./config/agent_config.json";
```

### 3. Разделители строк

```cpp
// Кроссплатформенно
std::string line = "text";
line += std::string(1, '\n');  // '\n' работает везде
```

---

## Чеклист перед релизом

- [ ] Сборка на Linux (Ubuntu/Debian)
- [ ] Сборка на Windows (MSVC)
- [ ] Сборка на macOS (Xcode)
- [ ] Все тесты проходят на всех платформах
- [ ] Конфигурационные файлы корректны
- [ ] Документация актуальна
- [ ] Инструкции по установке работают

---

## Поддержка

При возникновении проблем на конкретной платформе:
1. Проверьте версию CMake (`cmake --version`)
2. Проверьте компилятор (`g++ --version` или `cl`)
3. Убедитесь, что все зависимости установлены
4. Очистите build директорию и пересоберите
5. Проверьте логи в `agent.log`
