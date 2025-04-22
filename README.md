# 🤖 PC Interactor Bot

**PC Interactor Bot** is a Telegram bot for remotely controlling your PC. It allows you to perform system actions, monitor resources, manage processes, take screenshots, and even control media — all via Telegram.

## 🛠 Features

Commands are divided into regular and admin-only ones:

### 👥 Public Commands:
- `/start` — greeting and access information
- `/help` — list of available commands

### 🛠 Admin Commands:
- `/systeminfo` — detailed system information (CPU, RAM, GPU, etc.)
- `/monitor` — current resource usage
- `/screenshot` — capture a screenshot
- `/camera` — capture from webcam
- `/lock` — lock the workstation
- `/hibernate`, `/sleep`, `/shutdown` — power actions
- `/volume [0–100]` — get/set system volume
- `/say <text>` — speak text aloud
- `/run <command>` — run a shell command
- `/processes` — list running processes with pagination
- `/find <name>` — find processes by name with kill option
- `/kill <pid>` — terminate process by PID
- Media controls: `/media_toggle`, `/media_next`, `/media_prev`

## 📦 Release

The release includes:
```
release/
📂 pc_interactor_bot.exe
📂 config.json
```

### Example `config.json`:
```json
{
  "bot_name": "PC Interactor",
  "bot_description": "💻 Control your PC remotely via Telegram",
  "bot_short_description": "Remote computer control",
  "telegram_api_token": "YOUR_TOKEN_HERE",
  "admin_ids": [123456789],
  "long_polling_timeout": 30,
  "skip_old_updates": true
}
```

## 🔧 Building the Project

The project is written in **C++23** using **CMake** and the following dependencies:

### Dependencies:
- [CPR](https://github.com/libcpr/cpr) (HTTP client)
- [nlohmann/json](https://github.com/nlohmann/json) (JSON handling)
- [fmt](https://github.com/fmtlib/fmt) (logging/formatting)
- Windows APIs: `WMI`, `Shell`, `MF`, `SAPI`, `PowerProf`, `COM`, etc.

### Build Instructions:

```bash
git clone https://github.com/1floppa3/pc-interactor-bot.git
cd pc_interactor_bot
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

> Minimum CMake version: `3.29`  
> Platform: **Windows** (uses WMI, COM, Shell32, etc.)

## ⚙️ Highlights

- Auto registration of commands and buttons
- Pagination support in Telegram UI
- Auto restart and structured logging
- Optional autostart via Task Scheduler

## 📌 Administrator Privileges

Some commands require administrator rights. The bot checks and prompts for elevation at startup if necessary.

---

License: MIT
