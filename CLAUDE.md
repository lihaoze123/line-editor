# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Test Commands

```bash
# Build the project
make

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild

# Run all tests
make test

# Run the editor
make run

# Run with specific files
./bin/line-editor input.txt output.txt
```

## Architecture Overview

This is a line editor implemented in C++17 for a data structures course project. It uses a dual-layer linked list design with an "active zone" pattern that loads only a portion of the file into memory at a time.

### Core Data Structures

1. **LineBlock** (`line_block.h`) - Fixed 81-byte storage unit (80 chars + null terminator). Multiple blocks chain together via singly-linked list to store a single line longer than 80 characters.

2. **Line** (`line.h`) - Represents a single text line. Contains a head pointer to a LineBlock chain. Lines are doubly-linked with `prev` and `next` pointers.

3. **ActiveZone** (`active_zone.h`) - Manages the active working set (max 100 lines). Maintains a doubly-linked list of Lines, tracks the starting global line number, and handles insertion/deletion/operations within the zone.

### Layered Architecture

```
UI Layer (main.cpp, editor.h)
    ↓
Command Layer (command_parser.h → command_executor.h)
    ↓
Business Layer (active_zone.h ← file_manager.h)
    ↓
Data Layer (line.h → line_block.h)
```

### Module Responsibilities

| Module | Responsibility |
|--------|---------------|
| `LineBlock` | Fixed-size character block storage, singly-linked within a line |
| `Line` | Row-level text management, provides text access/search/replace interfaces |
| `ActiveZone` | Zone management, bidirectional line list, add/delete/find/replace operations |
| `FileManager` | File stream handling, line-by-line read/write |
| `CommandParser` | Command parsing and validation (supports i, d, n, p, s, m, q commands) |
| `CommandExecutor` | Command dispatch and execution |
| `Editor` | Main controller integrating all components |

### Command Syntax

- `i<n>` - Insert after line n (n=0 inserts before first line)
- `d<n1> [n2]` - Delete line n1 or range n1 to n2
- `n` - Switch active zone (write current zone to output, load next section)
- `p` - Display active zone (20 lines per page)
- `s<n>@<old>@<new>` - Replace old with new in line n
- `m<pattern>` - Search for pattern in active zone
- `q` - Quit editor

### Key Design Constraints

- **Block size**: 81 bytes (80 characters + null terminator)
- **Max active zone size**: 100 lines
- **Page display size**: 20 lines
- **Memory management**: All classes use move semantics, copy is deleted (ownership transfer model)

### Testing

The project uses a custom test framework (`test/test_framework.h`) with macros like `ASSERT_TRUE`, `ASSERT_EQ`, `ASSERT_STR_EQ`. Tests are registered via `REGISTER_TEST(suite, name)` macro. Test files follow `test_*.cpp` naming pattern.

### Error Handling

Uses `EditorException` with `ErrorCode` enum for typed errors. Common error cases include unknown commands, line numbers out of range, invalid ranges, file operation failures.
