#include "../include/command_executor.h"
#include "../include/command_parser.h"
#include "../include/active_zone.h"
#include "../include/file_manager.h"
#include "../include/line.h"
#include "test_framework.h"
#include <fstream>
#include <cstdio>

using namespace line_editor;

// 测试用临时文件管理
class TempFile {
    std::string path_;
public:
    TempFile(const std::string& content) {
        path_ = "/tmp/line_editor_test_" + std::to_string(rand()) + ".txt";
        std::ofstream ofs(path_);
        ofs << content;
        ofs.close();
    }

    // 禁止拷贝
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    // 启用移动
    TempFile(TempFile&& other) noexcept : path_(std::move(other.path_)) {
        other.path_.clear();
    }

    TempFile& operator=(TempFile&& other) noexcept {
        if (this != &other) {
            std::remove(path_.c_str());
            path_ = std::move(other.path_);
            other.path_.clear();
        }
        return *this;
    }

    ~TempFile() {
        if (!path_.empty()) {
            std::remove(path_.c_str());
        }
    }

    std::string path() const { return path_; }
};

// Test: 执行插入命令
TEST(Executor_Insert) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::INSERT;
    cmd.lineNo = 0;
    cmd.text = "Hello World";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(zone.lineCount(), 1);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Hello World");

    return true;
}

// Test: 在中间位置插入
TEST(Executor_InsertMiddle) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    // 先添加3行
    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));
    zone.appendLine(new Line("Line 3"));

    // 在第2行后插入
    Command cmd;
    cmd.type = CommandType::INSERT;
    cmd.lineNo = 2;
    cmd.text = "Inserted Line";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(zone.lineCount(), 4);
    ASSERT_STR_EQ(zone.getLine(2)->getText().c_str(), "Inserted Line");
    ASSERT_STR_EQ(zone.getLine(3)->getText().c_str(), "Line 3");

    return true;
}

// Test: 删除单行
TEST(Executor_DeleteSingle) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));
    zone.appendLine(new Line("Line 3"));

    Command cmd;
    cmd.type = CommandType::DELETE;
    cmd.lineNo = 2;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(zone.lineCount(), 2);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Line 3");

    return true;
}

// Test: 删除范围
TEST(Executor_DeleteRange) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    for (int i = 1; i <= 5; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    Command cmd;
    cmd.type = CommandType::DELETE;
    cmd.lineNo = 2;
    cmd.lineNo2 = 4;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(zone.lineCount(), 2);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Line 5");

    return true;
}

// Test: 打印命令
TEST(Executor_Print) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    for (int i = 1; i <= 25; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    Command cmd;
    cmd.type = CommandType::PRINT;
    cmd.pageNum = 0;  // 第1页

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(!result.output.empty());
    // 第1页应该包含20行
    size_t newlineCount = 0;
    for (char c : result.output) {
        if (c == '\n') newlineCount++;
    }
    ASSERT_EQ(newlineCount, 20);

    return true;
}

// Test: 打印第2页
TEST(Executor_PrintSecondPage) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    for (int i = 1; i <= 25; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    Command cmd;
    cmd.type = CommandType::PRINT;
    cmd.pageNum = 1;  // 第2页

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(!result.output.empty());
    // 第2页应该包含5行
    size_t newlineCount = 0;
    for (char c : result.output) {
        if (c == '\n') newlineCount++;
    }
    ASSERT_EQ(newlineCount, 5);

    return true;
}

// Test: 页码超出范围（显示最后一页）
TEST(Executor_PrintPageOutOfRange) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    for (int i = 1; i <= 25; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    Command cmd;
    cmd.type = CommandType::PRINT;
    cmd.pageNum = 999;  // 超出范围

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    // 应该显示最后一页
    ASSERT_TRUE(result.message.find("第 2 页") != std::string::npos);

    return true;
}

// Test: 替换命令
TEST(Executor_Replace) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Hello World"));
    zone.appendLine(new Line("Goodbye World"));

    Command cmd;
    cmd.type = CommandType::REPLACE;
    cmd.lineNo = 1;
    cmd.oldStr = "World";
    cmd.newStr = "Universe";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Hello Universe");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Goodbye World");

    return true;
}

// Test: 模式匹配
TEST(Executor_Match) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Hello World"));
    zone.appendLine(new Line("Hello Universe"));
    zone.appendLine(new Line("Goodbye World"));

    Command cmd;
    cmd.type = CommandType::MATCH;
    cmd.pattern = "Hello";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.message.find("1") != std::string::npos);
    ASSERT_TRUE(result.message.find("2") != std::string::npos);

    return true;
}

// Test: 退出命令
TEST(Executor_Quit) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::QUIT;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.shouldExit);

    return true;
}

// Test: 活区切换 - 写入输出文件
TEST(Executor_NextZone_WriteOutput) {
    TempFile inputFile("Line 1\nLine 2\nLine 3\n");
    TempFile outputFile("");  // 空输出文件

    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    // 打开文件
    ASSERT_TRUE(fileMgr.openInput(inputFile.path()));
    ASSERT_TRUE(fileMgr.openOutput(outputFile.path()));

    // 添加一些内容到活区
    zone.appendLine(new Line("Test Line 1"));
    zone.appendLine(new Line("Test Line 2"));

    // 执行 n 命令
    Command cmd;
    cmd.type = CommandType::NEXT_ZONE;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);

    // 关闭文件管理器以刷新缓冲区
    fileMgr.close();

    // 验证输出文件内容
    std::ifstream ifs(outputFile.path());
    std::string content;
    std::string line;
    while (std::getline(ifs, line)) {
        content += line + "\n";
    }
    ifs.close();

    ASSERT_TRUE(content.find("Test Line 1") != std::string::npos);
    ASSERT_TRUE(content.find("Test Line 2") != std::string::npos);

    return true;
}

// Test: 空活区打印
TEST(Executor_PrintEmptyZone) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::PRINT;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.message.find("空") != std::string::npos);

    return true;
}

// Test: 多页显示
TEST(Executor_MultiplePages) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    // 添加50行，应该有3页
    for (int i = 1; i <= 50; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    ASSERT_EQ(zone.totalPages(), 3);

    // 测试各页
    for (int page = 0; page < 3; ++page) {
        Command cmd;
        cmd.type = CommandType::PRINT;
        cmd.pageNum = page;

        ExecutionResult result = executor.execute(cmd);

        ASSERT_TRUE(result.success);
        ASSERT_TRUE(result.message.find("第 " + std::to_string(page + 1) + " 页") != std::string::npos);
    }

    return true;
}

// 注册测试
REGISTER_TEST(CommandExecutor, Executor_Insert);
REGISTER_TEST(CommandExecutor, Executor_InsertMiddle);
REGISTER_TEST(CommandExecutor, Executor_DeleteSingle);
REGISTER_TEST(CommandExecutor, Executor_DeleteRange);
REGISTER_TEST(CommandExecutor, Executor_Print);
REGISTER_TEST(CommandExecutor, Executor_PrintSecondPage);
REGISTER_TEST(CommandExecutor, Executor_PrintPageOutOfRange);
REGISTER_TEST(CommandExecutor, Executor_Replace);
REGISTER_TEST(CommandExecutor, Executor_Match);
REGISTER_TEST(CommandExecutor, Executor_Quit);
REGISTER_TEST(CommandExecutor, Executor_NextZone_WriteOutput);
REGISTER_TEST(CommandExecutor, Executor_PrintEmptyZone);
REGISTER_TEST(CommandExecutor, Executor_MultiplePages);
