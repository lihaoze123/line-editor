cmake_minimum_required(VERSION 3.14)
project(LineEditor VERSION 1.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
add_compile_options(-Wall -Wextra)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 核心库源文件（不包括 main.cpp）
set(CORE_SOURCES
    src/line_block.cpp
    src/line.cpp
    src/error.cpp
    src/active_zone.cpp
    src/file_manager.cpp
    src/command_parser.cpp
    src/command_executor.cpp
    src/editor.cpp
)

# 创建核心库
add_library(line_editor_core STATIC ${CORE_SOURCES})
target_include_directories(line_editor_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# 主可执行文件
add_executable(line-editor src/main.cpp)
target_link_libraries(line-editor PRIVATE line_editor_core)

# 测试可执行文件
set(TEST_SOURCES
    test/test_main.cpp
    test/test_line_block.cpp
    test/test_line.cpp
    test/test_active_zone.cpp
    test/test_command_parser.cpp
    test/test_command_executor.cpp
    test/test_editor_integration.cpp
    test/test_boundary_cases.cpp
)

add_executable(test_runner ${TEST_SOURCES})
target_link_libraries(test_runner PRIVATE line_editor_core)
target_include_directories(test_runner PRIVATE
    ${PROJECT_SOURCE_DIR}/test
)

# 启用测试
enable_testing()
add_test(NAME LineEditorTests COMMAND test_runner)

# 安装目标
install(TARGETS line-editor
    RUNTIME DESTINATION bin
)

# 自定义目标
add_custom_target(run
    COMMAND line-editor
    DEPENDS line-editor
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行行编辑器"
)

add_custom_target(test-run
    COMMAND test_runner
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "运行测试"
)

# 打印构建信息
message(STATUS "Line Editor Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Binary Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
