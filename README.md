# 🛡️ Upload Monitor - Data Exfiltration Detection Tool

> A Windows-based security monitoring application written in pure C that detects and logs file uploads from employee workstations to the internet.

---

## 📋 Table of Contents

* [Overview](#overview)
* [How It Works](#how-it-works)
* [Features](#features)
* [Technical Architecture](#technical-architecture)
* [Requirements](#requirements)
* [Installation](#installation)
* [Compilation](#compilation)
* [Usage](#usage)
* [Output Format](#output-format)
* [Code Breakdown](#code-breakdown)
* [Project Structure](#project-structure)
* [Troubleshooting](#troubleshooting)
* [Security Notes](#security-notes)
* [License](#license)

---

## 📖 Overview

### What is This Project?

This is a **Security Monitoring Tool** designed to detect unauthorized data exfiltration from corporate workstations. It monitors network activity in real-time and identifies when applications upload files to remote servers.

### Real-World Use Cases

```
❌ Employee uploads confidential documents to Google Drive
❌ Worker sends source code via WhatsApp Web
❌ Staff transfers client data to personal Dropbox
✅ THIS TOOL DETECTS ALL OF THESE ACTIVITIES!
```

---

## 🔄 How It Works

### System Architecture

```
┌────────────────────────────────────────────┐
│ YOUR COMPUTER                             │
│                                           │
│ Chrome → file.pdf → Internet              │
│        ↓                                  │
│   142.250.80.110:443                      │
│                                           │
│ 👁️ Upload Monitor logs everything         │
└────────────────────────────────────────────┘
```

### Monitoring Flow

```
Start Program
   ↓
Get TCP Table
   ↓
Filter Connections
   ↓
Extract Process + IP + Port
   ↓
Log Detection
   ↓
Sleep (3s)
   ↓
Repeat
```

---

## ✨ Features

| Feature                 | Description                               |
| ----------------------- | ----------------------------------------- |
| 🕒 Real-time Monitoring | Polls network connections every 3 seconds |
| 📝 Dual Output          | Console AND log file                      |
| 🔍 Process Detection    | Identifies uploading app                  |
| 🌐 Network Tracking     | Captures remote IP & port                 |
| 📊 Size Formatting      | Bytes → KB/MB/GB                          |
| ⚡ Lightweight           | Low resource usage                        |
| 🔧 No Dependencies      | Pure C                                    |
| 📋 Timestamp Logging    | Exact time tracking                       |

---

## 🏗️ Technical Architecture

### Windows APIs Used

| API Function           | Purpose             | Library    |
| ---------------------- | ------------------- | ---------- |
| GetExtendedTcpTable()  | Get TCP connections | iphlpapi   |
| OpenProcess()          | Access process      | kernel32   |
| GetModuleFileNameExA() | Get process name    | psapi      |
| time()                 | Timestamp           | C standard |
| fopen()                | Logging             | C standard |

---

## 📦 Requirements

* Windows 10 or 11
* Administrator privileges
* MinGW or Visual Studio

---

## 📥 Installation

### Install MinGW

1. Download MinGW
2. Extract to `C:\mingw64`
3. Add `C:\mingw64\bin` to PATH

Verify:

```
gcc --version
```

---

## 🔨 Compilation

### GCC

```
gcc upload_monitor.c -o upload_monitor.exe -liphlpapi -lpsapi
```

### MSVC

```
cl upload_monitor.c /link iphlpapi.lib psapi.lib kernel32.lib
```

---

## 🚀 Usage

### Run as Administrator

```
upload_monitor.exe
```

---

## 📊 Output Format

```
[TIMESTAMP] IP:REMOTE_IP:PORT | App:PROCESS | File:PATH | Size:DATA
```

### Example

```
[2026-04-28 15:30:22] IP:142.250.80.110:443 | App:chrome.exe | File:C:\test.pdf | Size:2.45 MB
```

---

## 💻 Code Breakdown

### Timestamp Function

```
void print_timestamp() {
    time_t now = time(NULL);
}
```

### Process Name

```
OpenProcess(...)
```

### Main Loop

```
while (1) {
    GetExtendedTcpTable(...);
}
```

---

## 📁 Project Structure

```
Q2/
├── upload_monitor.c
├── upload_monitor.exe
├── README.md
├── sample_upload.log
└── screenshots/
```

---

## 🔧 Troubleshooting

### gcc not found

```
C:\mingw64\bin\gcc.exe upload_monitor.c
```

### Access denied

Run as Administrator

---

## 🔒 Security Notes

### Requires Admin because:

* Needs system-wide monitoring

### Does NOT:

* Read file contents
* Capture passwords
* Log keystrokes

---

## 📚 Learning Outcomes

* Windows API
* C programming
* Network monitoring
* Security tools

---

## 🎓 Submission Checklist

* Source code
* Executable
* README
* Log file
* Screenshots

---

## 📄 License

Educational use only.

---

Built for Synapse Assessment 🚀
