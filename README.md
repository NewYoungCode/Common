# Common Library

本库为 C++ 通用开发工具库，提供网络通信、加解密、文件处理、多线程、数据库访问、二维码生成、日志系统等通用能力模块，适用于 Win32/Windows 平台，可集成于各种客户端项目中。

---

## 📁 目录结构概览

```
Common/
│
├── curl/           # libcurl 支持文件
├── gzip/           # GZIP 压缩支持
├── json/           # JSON 库支持
├── mysql/          # MySQL C API 支持
├── occi/           # Oracle C++ 接口支持（OCCI）
├── openssl/        # OpenSSL 加解密支持
├── qrcode/         # 二维码生成功能支持
├── zip/            # ZIP 打包支持
├── zlib/           # Zlib 解压/压缩支持
│
├── Chat.hpp        # 简单的聊天协议工具或数据结构
├── Common.h        # 公共宏、基础类型定义
├── ConfigIni.h     # INI 配置文件读取工具
├── FileSystem.h    # 文件系统相关操作封装
├── Log.h           # 日志模块，支持 Info/Debug/Error 输出
├── Map.hpp         # STL map 封装/扩展工具
├── MySqlClient.hpp # MySQL 客户端封装类
├── OrcaleClient.hpp# Oracle 客户端封装类（OCCI）
├── QrenCode.hpp    # 二维码编码接口（基于 qrcode 模块）
├── Random.hpp      # 随机数工具封装
├── Regedit.hpp     # 注册表操作工具（Windows 专用）
├── Socket.h        # 网络 Socket 封装
├── Text.h          # 字符串工具类（大小写转换、编码等）
├── ThreadPool.hpp  # 通用线程池封装
├── Time.hpp        # 时间工具类（格式化、计时、获取时间戳等）
├── UnZiper.h       # 解压缩支持工具（支持 ZIP、GZIP 等）
├── Util.h          # 通用工具函数合集
├── WebClient.h     # HTTP(S) 客户端封装（基于 libcurl 或 WinInet）
├── WinTool.h       # Windows 工具集合（消息、窗口、系统等）
├── zconf.h         # Zlib 配置头文件
├── Ziper.h         # ZIP 打包工具封装
├── zlib.h          # Zlib 主头文件
├── base64.h        # Base64 编解码工具
├── JsonValue.h     # JSON 数据结构封装（内部格式转换）
├── httplib.h       # HTTP Server/Client 支持（如 cpp-httplib）
```

---

## 📖 模块功能说明

### 网络与通信
- `WebClient.h`：封装 HTTP(S) 请求发送、响应处理
- `Socket.h`：TCP/UDP Socket 支持，兼容 Windows
- `httplib.h`：轻量级 HTTP 服务/客户端支持（如 cpp-httplib）

### 压缩与打包
- `Ziper.h` / `UnZiper.h`：支持 ZIP 文件的压缩和解压
- `gzip/`, `zlib/`：对 GZIP/Zlib 文件压缩格式的原生支持

### 加解密与编码
- `openssl/`：支持 AES、RSA、SHA 等常用加解密算法
- `base64.h`：Base64 编解码封装

### 配置与文件系统
- `ConfigIni.h`：读取/写入 .ini 配置文件
- `FileSystem.h`：文件操作、目录创建、路径判断等

### 日志与工具
- `Log.h`：支持日志级别输出，控制台/文件输出可选
- `Util.h`：通用辅助函数（类型转换、路径处理等）
- `WinTool.h`：与 Windows API 相关的实用函数
- `Time.hpp`：时间戳、时间格式、计时器封装
- `Random.hpp`：随机数生成工具
- `Regedit.hpp`：Windows 注册表读写封装

### 字符串与数据结构
- `Text.h`：字符串大小写转换、分割、编码处理等
- `Map.hpp`：封装 Map 常用操作或扩展
- `JsonValue.h` / `json/`：JSON 解析与构造工具

### 多线程
- `ThreadPool.hpp`：线程池任务调度封装

### 数据库支持
- `MySqlClient.hpp` / `mysql/`：MySQL 客户端封装类
- `OrcaleClient.hpp` / `occi/`：Oracle（OCCI）客户端封装类

### 二维码
- `QrenCode.hpp` / `qrcode/`：生成二维码图片、支持文本编码等

### 其他模块
- `Common.h`：通用定义（类型别名、宏定义等）
- `Chat.hpp`：可用于基础通信协议的结构体或逻辑封装

---

## 📚 使用说明

你可以根据需要引入对应的头文件，例如：

```cpp
#include "Log.h"
#include "WebClient.h"
#include "ConfigIni.h"
#include "ThreadPool.hpp"
```

每个模块都设计为独立解耦，可以根据项目按需编译。

---

## 🔧 编译要求
- Windows 平台（Win32/Win64）
- C++11 或更高（建议）
- 需要链接第三方库（如 `libcurl`, `openssl`, `zlib`, `mysqlclient`, `occi` 等）

---

## 💬 反馈与贡献
欢迎在使用中提出建议、修复或扩展，未来将进一步模块化并支持跨平台封装。

