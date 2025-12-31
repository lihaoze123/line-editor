# 行编辑程序 (Line Editor)

数据结构课程设计项目 - 简单的文本行编辑程序。

## 功能特性

### 基本功能
- `i<n>` - 行插入，在第n行后插入文本（n=0表示插入首行前）
- `d<n>` - 删除第n行
- `d<n1> <n2>` - 删除第n1到n2行
- `n` - 活区切换，将当前活区写入输出文件，读取下一段
- `p` - 显示活区内容（每页20行）

### 选作功能
- `s<n>@<old>@<new>` - 在第n行将old替换为new
- `m<pattern>` - 在活区内搜索匹配pattern的行
- `q` - 退出编辑器

## 编译

```bash
cd line-editor
make
```

## 运行

```bash
# 命令行参数指定文件
./bin/line-editor input.txt output.txt

# 或交互式输入文件名
./bin/line-editor
```

## 测试

```bash
make test
```

## 项目结构

```
line-editor/
├── include/           # 头文件
│   ├── line_block.h   # 行块数据结构
│   ├── line.h         # 行数据结构
│   ├── active_zone.h  # 活区管理
│   ├── file_manager.h # 文件管理
│   ├── command_parser.h   # 命令解析
│   ├── command_executor.h # 命令执行
│   ├── editor.h       # 主编辑器
│   └── error.h        # 错误处理
│
├── src/               # 源文件
│   └── ...
│
├── test/              # 测试文件
│   ├── test_framework.h    # 测试框架
│   ├── test_line_block.cpp
│   ├── test_line.cpp
│   ├── test_active_zone.cpp
│   └── test_command_parser.cpp
│
└── docs/              # 文档
    └── plans/2024-12-29-line-editor-design.md
```

## 技术细节

- **语言**: C++17
- **数据结构**: 双向链表 + 行块存储
- **行块大小**: 81字节（80字符 + 终止符）
- **活区大小**: 最多100行

## 作者

数据结构课程设计小组
