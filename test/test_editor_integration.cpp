#include "../include/editor.h"
#include "../include/command_parser.h"
#include "../include/error.h"
#include "test_framework.h"
#include <fstream>
#include <cstdio>
#include <sstream>

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

    TempFile() {
        path_ = "/tmp/line_editor_test_" + std::to_string(rand()) + ".txt";
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

    std::string readContent() const {
        std::ifstream ifs(path_);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());
        return content;
    }

    void write(const std::string& content) {
        std::ofstream ofs(path_);
        ofs << content;
        ofs.close();
    }
};

// 测试用辅助函数：创建测试输入文件
TempFile createTestInputFile() {
    return TempFile(
        "Line 1\n"
        "Line 2\n"
        "Line 3\n"
        "Line 4\n"
        "Line 5\n"
        "Line 6\n"
        "Line 7\n"
        "Line 8\n"
        "Line 9\n"
        "Line 10\n"
        "Line 11\n"
        "Line 12\n"
        "Line 13\n"
        "Line 14\n"
        "Line 15\n"
    );
}

// Test: 编辑器初始化
TEST(Editor_Init) {
    TempFile inputFile = createTestInputFile();
    TempFile outputFile;

    Editor editor;
    bool success = editor.init(inputFile.path(), outputFile.path());

    ASSERT_TRUE(success);
    ASSERT_TRUE(editor.isInitialized());
    ASSERT_EQ(editor.zone().lineCount(), 15);

    return true;
}

// Test: 输入输出文件不能相同
TEST(Editor_SameInputOutputFile) {
    TempFile file("test content");

    Editor editor;
    bool success = editor.init(file.path(), file.path());

    ASSERT_FALSE(success);

    return true;
}

// Test: 解析简单命令
TEST(Editor_ParseSimpleCommand) {
    CommandParser parser;

    Command cmd = parser.parse("i10");
    ASSERT_TRUE(cmd.type == CommandType::INSERT);
    ASSERT_EQ(cmd.lineNo, 10);

    cmd = parser.parse("d5");
    ASSERT_TRUE(cmd.type == CommandType::DELETE);
    ASSERT_EQ(cmd.lineNo, 5);

    cmd = parser.parse("n");
    ASSERT_TRUE(cmd.type == CommandType::NEXT_ZONE);

    cmd = parser.parse("p");
    ASSERT_TRUE(cmd.type == CommandType::PRINT);

    cmd = parser.parse("q");
    ASSERT_TRUE(cmd.type == CommandType::QUIT);

    return true;
}

// Test: 解析带页码的打印命令
TEST(Editor_ParsePrintWithPage) {
    CommandParser parser;

    Command cmd = parser.parse("p");
    ASSERT_EQ(cmd.pageNum, 0);

    cmd = parser.parse("p1");
    ASSERT_EQ(cmd.pageNum, 0);

    cmd = parser.parse("p2");
    ASSERT_EQ(cmd.pageNum, 1);

    cmd = parser.parse("p5");
    ASSERT_EQ(cmd.pageNum, 4);

    return true;
}

// Test: 解析替换命令
TEST(Editor_ParseReplaceCommand) {
    CommandParser parser;

    Command cmd = parser.parse("s5@old@new");
    ASSERT_TRUE(cmd.type == CommandType::REPLACE);
    ASSERT_EQ(cmd.lineNo, 5);
    ASSERT_STR_EQ(cmd.oldStr.c_str(), "old");
    ASSERT_STR_EQ(cmd.newStr.c_str(), "new");

    return true;
}

// Test: 解析匹配命令
TEST(Editor_ParseMatchCommand) {
    CommandParser parser;

    Command cmd = parser.parse("mHello");
    ASSERT_TRUE(cmd.type == CommandType::MATCH);
    ASSERT_STR_EQ(cmd.pattern.c_str(), "Hello");

    cmd = parser.parse("mtest pattern");
    ASSERT_TRUE(cmd.type == CommandType::MATCH);
    ASSERT_STR_EQ(cmd.pattern.c_str(), "test pattern");

    return true;
}

// Test: 解析删除范围命令
TEST(Editor_ParseDeleteRange) {
    CommandParser parser;

    Command cmd = parser.parse("d3 7");
    ASSERT_TRUE(cmd.type == CommandType::DELETE);
    ASSERT_EQ(cmd.lineNo, 3);
    ASSERT_EQ(cmd.lineNo2, 7);

    return true;
}

// Test: 验证行号范围
TEST(Editor_ValidateLineNumber) {
    CommandParser parser;

    // 正常范围的命令
    Command cmd = parser.parse("i5");
    ASSERT_TRUE(cmd.type == CommandType::INSERT);

    // 不应该抛出异常
    try {
        parser.validate(cmd, 1, 10);
    } catch (...) {
        return false;  // 不应该抛出异常
    }

    return true;
}

// Test: 超出范围的行号应该抛出异常
TEST(Editor_ValidateOutOfRange) {
    CommandParser parser;

    Command cmd = parser.parse("i15");
    ASSERT_TRUE(cmd.type == CommandType::INSERT);

    bool caught = false;
    try {
        parser.validate(cmd, 1, 10);  // 活区只有1-10行
    } catch (const EditorException& e) {
        caught = true;
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 活区基本操作
TEST(Editor_ActiveZoneBasicOperations) {
    ActiveZone zone(100);

    // 测试空活区
    ASSERT_TRUE(zone.isEmpty());
    ASSERT_EQ(zone.lineCount(), 0);

    // 添加行
    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    ASSERT_FALSE(zone.isEmpty());
    ASSERT_EQ(zone.lineCount(), 2);

    // 删除行
    zone.deleteLine(1);
    ASSERT_EQ(zone.lineCount(), 1);

    return true;
}

// Test: 活区插入操作
TEST(Editor_ActiveZoneInsert) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 3"));

    // 在第1行后插入
    zone.insert(1, "Line 2");

    ASSERT_EQ(zone.lineCount(), 3);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Line 2");
    ASSERT_STR_EQ(zone.getLine(2)->getText().c_str(), "Line 3");

    return true;
}

// Test: 活区替换操作
TEST(Editor_ActiveZoneReplace) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Hello World"));

    bool replaced = zone.replaceInLine(1, "World", "Universe");

    ASSERT_TRUE(replaced);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Hello Universe");

    return true;
}

// Test: 活区模式查找
TEST(Editor_ActiveZoneFindPattern) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Hello World"));
    zone.appendLine(new Line("Hello Universe"));
    zone.appendLine(new Line("Goodbye World"));

    std::vector<int> matches = zone.findPattern("Hello");

    ASSERT_EQ(matches.size(), 2);
    ASSERT_EQ(matches[0], 1);
    ASSERT_EQ(matches[1], 2);

    return true;
}

// Test: 活区分页显示
TEST(Editor_ActiveZonePagination) {
    ActiveZone zone(100);

    // 添加25行，应该有2页
    for (int i = 1; i <= 25; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    ASSERT_EQ(zone.totalPages(), 2);

    std::string page1 = zone.display(0);
    std::string page2 = zone.display(1);

    // 第1页应该包含20行
    int newlineCount1 = 0;
    for (char c : page1) {
        if (c == '\n') newlineCount1++;
    }
    ASSERT_EQ(newlineCount1, 20);

    // 第2页应该包含5行
    int newlineCount2 = 0;
    for (char c : page2) {
        if (c == '\n') newlineCount2++;
    }
    ASSERT_EQ(newlineCount2, 5);

    return true;
}

// Test: 活区清空操作
TEST(Editor_ActiveZoneClear) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    ASSERT_EQ(zone.lineCount(), 2);

    zone.clear();

    ASSERT_EQ(zone.lineCount(), 0);
    ASSERT_TRUE(zone.isEmpty());

    return true;
}

// Test: 活区最大行数限制
TEST(Editor_ActiveZoneMaxLines) {
    ActiveZone zone(5);  // 设置最大5行

    // 先添加5行
    for (int i = 1; i <= 5; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }
    ASSERT_EQ(zone.lineCount(), 5);

    // 使用 insert 超过最大行数，会移除首行
    zone.insert(5, "Line 6");
    ASSERT_EQ(zone.lineCount(), 5);
    // 第一行应该是原来的 "Line 2"，因为 "Line 1" 被移除了
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 2");

    return true;
}

// Test: 文件管理器读取
TEST(Editor_FileManagerRead) {
    TempFile file = createTestInputFile();

    FileManager fm;
    ASSERT_TRUE(fm.openInput(file.path()));

    std::vector<std::string> lines;
    int count = fm.readLines(lines, 5);

    ASSERT_EQ(count, 5);
    ASSERT_EQ(lines.size(), 5);
    ASSERT_STR_EQ(lines[0].c_str(), "Line 1");
    ASSERT_STR_EQ(lines[4].c_str(), "Line 5");

    fm.close();

    return true;
}

// Test: 文件管理器写入
TEST(Editor_FileManagerWrite) {
    TempFile file;

    FileManager fm;
    ASSERT_TRUE(fm.openOutput(file.path()));

    ASSERT_TRUE(fm.writeLine("Line 1"));
    ASSERT_TRUE(fm.writeLine("Line 2"));

    fm.close();

    // 验证写入内容
    std::string content = file.readContent();
    ASSERT_TRUE(content.find("Line 1") != std::string::npos);
    ASSERT_TRUE(content.find("Line 2") != std::string::npos);

    return true;
}

// Test: 完整编辑流程（集成测试）
TEST(Editor_FullWorkflow) {
    TempFile inputFile("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\n");
    TempFile outputFile;

    Editor editor;
    ASSERT_TRUE(editor.init(inputFile.path(), outputFile.path()));

    // 初始状态
    ASSERT_EQ(editor.zone().lineCount(), 5);

    // 插入一行
    editor.zone().insert(0, "New First Line");
    ASSERT_EQ(editor.zone().lineCount(), 6);
    ASSERT_STR_EQ(editor.zone().getLine(0)->getText().c_str(), "New First Line");

    // 删除一行
    editor.zone().deleteLine(2);
    ASSERT_EQ(editor.zone().lineCount(), 5);

    // 替换内容
    ASSERT_TRUE(editor.zone().replaceInLine(1, "New First Line", "Updated Line"));

    return true;
}

// 注册测试
REGISTER_TEST(EditorIntegration, Editor_Init);
REGISTER_TEST(EditorIntegration, Editor_SameInputOutputFile);
REGISTER_TEST(EditorIntegration, Editor_ParseSimpleCommand);
REGISTER_TEST(EditorIntegration, Editor_ParsePrintWithPage);
REGISTER_TEST(EditorIntegration, Editor_ParseReplaceCommand);
REGISTER_TEST(EditorIntegration, Editor_ParseMatchCommand);
REGISTER_TEST(EditorIntegration, Editor_ParseDeleteRange);
REGISTER_TEST(EditorIntegration, Editor_ValidateLineNumber);
REGISTER_TEST(EditorIntegration, Editor_ValidateOutOfRange);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneBasicOperations);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneInsert);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneReplace);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneFindPattern);
REGISTER_TEST(EditorIntegration, Editor_ActiveZonePagination);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneClear);
REGISTER_TEST(EditorIntegration, Editor_ActiveZoneMaxLines);
REGISTER_TEST(EditorIntegration, Editor_FileManagerRead);
REGISTER_TEST(EditorIntegration, Editor_FileManagerWrite);
REGISTER_TEST(EditorIntegration, Editor_FullWorkflow);
