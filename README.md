# 行编辑程序 (Line Editor)

数据结构课程设计项目 - 简单的文本行编辑程序。

## 功能特性

### 基本功能
- `i<n>` - 行插入，在第n行后插入文本（n=0表示插入首行前）
- `d<n>` - 删除第n行
- `d<n1> <n2>` - 删除第n1到n2行
- `n` - 活区切换，将当前活区写入输出文件，读取下一段
- `p` - 显示活区内容（每页20行）

### 高级功能
- `s<n>@<old>@<new>` - 在第n行将old替换为new
- `m<pattern>` - 在活区内搜索匹配pattern的行
- `q` - 退出编辑器

## 编译

### Linux/macOS (使用 Make)

```bash
make
```

### Windows (使用 CMake + MSVC)

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Linux (使用 CMake)

```bash
cmake -B build
cmake --build build
```

### 清理构建产物

```bash
make clean          # Make
cmake --build build --target clean  # CMake
make rebuild        # 完全重新构建
```

## 运行

```bash
# 命令行参数指定文件
./bin/line-editor input.txt output.txt

# 或交互式输入文件名
./bin/line-editor
```

Windows 可执行文件位于 `build/bin/Release/line-editor.exe`。

## 测试

```bash
make test
```

## 架构设计

本项目采用双层链表结构和"活区"模式，仅将文件的一部分加载到内存中。

### 核心数据结构

| 模块 | 描述 |
|------|------|
| `LineBlock` | 固定81字节存储单元（80字符 + 终止符），多个块通过单向链表连接存储超长行 |
| `Line` | 表示单行文本，包含指向LineBlock链的头指针，行之间通过双向链表连接 |
| `ActiveZone` | 管理活动工作集（最多100行），维护双向行链表，处理插入/删除/替换操作 |

### 分层架构

```
UI Layer (main.cpp, editor.h)
    ↓
Command Layer (command_parser.h → command_executor.h)
    ↓
Business Layer (active_zone.h ← file_manager.h)
    ↓
Data Layer (line.h → line_block.h)
```

### 设计约束

- **块大小**: 81字节（80字符 + 终止符）
- **最大活区**: 100行
- **分页显示**: 每页20行
- **内存管理**: 所有权转移模型，禁用拷贝，仅使用移动语义

## 项目结构

```
line-editor/
├── include/               # 头文件
│   ├── line_block.h       # 行块数据结构
│   ├── line.h             # 行数据结构
│   ├── active_zone.h      # 活区管理
│   ├── file_manager.h     # 文件管理
│   ├── command_parser.h   # 命令解析
│   ├── command_executor.h # 命令执行
│   ├── editor.h           # 主编辑器
│   └── error.h            # 错误处理
│
├── src/                   # 源文件
│   └── ...
│
├── test/                  # 测试文件
│   ├── test_framework.h   # 测试框架
│   ├── test_line_block.cpp
│   ├── test_line.cpp
│   ├── test_active_zone.cpp
│   └── test_command_parser.cpp
│
├── .github/workflows/     # CI/CD 配置
│   ├── linux-ci.yml
│   └── windows-ci.yml
│
├── Makefile               # Make 构建配置
├── CMakeLists.txt         # CMake 构建配置
└── CLAUDE.md              # 项目指南
```

## 技术细节

- **语言**: C++17
- **支持平台**: Linux, macOS, Windows (MSVC)
- **数据结构**: 双向链表 + 行块存储
- **构建系统**: Make (Unix), CMake (跨平台)
- **编码**: UTF-8 (Windows 下改进控制台编码支持)

## 作者

数据结构课程设计小组
