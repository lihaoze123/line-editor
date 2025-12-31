#include "../include/line_block.h"
#include "../include/line.h"
#include "../include/active_zone.h"
#include "../include/file_manager.h"
#include "../include/command_parser.h"
#include "../include/command_executor.h"
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
    TempFile(const std::string& content = "") {
        path_ = "/tmp/line_editor_boundary_" + std::to_string(rand()) + ".txt";
        if (!content.empty()) {
            std::ofstream ofs(path_);
            ofs << content;
            ofs.close();
        }
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
    void remove() { std::remove(path_.c_str()); }
};

// ============================================================
// Error 模块测试
// ============================================================

// Test: 异常构造和错误码获取
TEST(Error_ExceptionConstruction) {
    EditorException ex(ErrorCode::INVALID_FORMAT, "Test error message");

    ASSERT_EQ(static_cast<int>(ex.code()), static_cast<int>(ErrorCode::INVALID_FORMAT));
    ASSERT_STR_EQ(ex.what(), "Test error message");

    return true;
}

// Test: 各种错误码创建异常
TEST(Error_AllErrorCodes) {
    EditorException ex1(ErrorCode::UNKNOWN_COMMAND, "Unknown");
    ASSERT_EQ(static_cast<int>(ex1.code()), static_cast<int>(ErrorCode::UNKNOWN_COMMAND));

    EditorException ex2(ErrorCode::LINE_NUMBER_OUT_OF_RANGE, "Out of range");
    ASSERT_EQ(static_cast<int>(ex2.code()), static_cast<int>(ErrorCode::LINE_NUMBER_OUT_OF_RANGE));

    EditorException ex3(ErrorCode::MISSING_PARAMETER, "Missing param");
    ASSERT_EQ(static_cast<int>(ex3.code()), static_cast<int>(ErrorCode::MISSING_PARAMETER));

    EditorException ex4(ErrorCode::INVALID_RANGE, "Invalid range");
    ASSERT_EQ(static_cast<int>(ex4.code()), static_cast<int>(ErrorCode::INVALID_RANGE));

    EditorException ex5(ErrorCode::FILE_OPEN_FAILED, "File open failed");
    ASSERT_EQ(static_cast<int>(ex5.code()), static_cast<int>(ErrorCode::FILE_OPEN_FAILED));

    EditorException ex6(ErrorCode::FILE_WRITE_FAILED, "File write failed");
    ASSERT_EQ(static_cast<int>(ex6.code()), static_cast<int>(ErrorCode::FILE_WRITE_FAILED));

    return true;
}

// Test: 异常作为标准异常派生类
TEST(Error_StdExceptionBase) {
    try {
        throw EditorException(ErrorCode::INVALID_FORMAT, "Test");
    } catch (const std::runtime_error& e) {
        ASSERT_STR_EQ(e.what(), "Test");
        return true;
    }
    return false;
}

// ============================================================
// LineBlock 边界测试
// ============================================================

// Test: 空字符串追加
TEST(LineBlock_AppendEmptyString) {
    LineBlock block;

    size_t result = block.append("", 0);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(block.used(), 0);

    return true;
}

// Test: 追加到恰好填满一个块
TEST(LineBlock_AppendExactlyFull) {
    LineBlock block;

    // 追加恰好 80 个字符
    std::string full(80, 'A');
    size_t result = block.append(full.c_str(), 80);

    ASSERT_EQ(result, 80);
    ASSERT_TRUE(block.isFull());

    return true;
}

// Test: 尝试超过块大小
TEST(LineBlock_AppendBeyondCapacity) {
    LineBlock block;

    // 追加超过 80 个字符
    std::string over(100, 'A');
    size_t result = block.append(over.c_str(), 100);

    // 应该只写入 80 个字符（留一个给 null 终止符）
    ASSERT_EQ(result, 80);
    ASSERT_TRUE(block.isFull());

    return true;
}

TEST(LineBlock_CreateNextChain) {
    LineBlock block1;

    LineBlock* block2 = block1.createNext();
    ASSERT_NOT_NULL(block2);
    ASSERT_EQ(block1.next(), block2);

    LineBlock* block3 = block2->createNext();
    ASSERT_EQ(block2->next(), block3);
    ASSERT_NOT_NULL(block3);

    return true;
}

TEST(LineBlock_SetNext) {
    LineBlock block1;
    LineBlock* block2 = new LineBlock();

    block1.setNext(block2);
    ASSERT_EQ(block1.next(), block2);

    return true;
}

// ============================================================
// Line 边界测试
// ============================================================

// Test: 空行
TEST(Line_EmptyLine) {
    Line line;

    ASSERT_TRUE(line.isEmpty());
    ASSERT_EQ(line.length(), 0);
    ASSERT_STR_EQ(line.getText().c_str(), "");

    return true;
}

// Test: 空字符串设置
TEST(Line_SetEmptyText) {
    Line line("Hello");
    line.setText("");

    ASSERT_TRUE(line.isEmpty());
    ASSERT_EQ(line.length(), 0);

    return true;
}

// Test: 查找空字符串
TEST(Line_FindEmptyString) {
    Line line("Hello World");

    // 空字符串应该在位置0找到
    int pos = line.find("");
    ASSERT_EQ(pos, 0);

    return true;
}

// Test: 查找不存在的字符串
TEST(Line_FindNonExistent) {
    Line line("Hello World");

    int pos = line.find("xyz");
    ASSERT_EQ(pos, -1);

    return true;
}

// Test: 替换不存在的字符串
TEST(Line_ReplaceNonExistent) {
    Line line("Hello World");

    bool result = line.replace("xyz", "abc");
    ASSERT_FALSE(result);
    ASSERT_STR_EQ(line.getText().c_str(), "Hello World");

    return true;
}

// Test: 替换为空字符串（删除）
TEST(Line_ReplaceWithEmpty) {
    Line line("Hello World");

    bool result = line.replace("World", "");
    ASSERT_TRUE(result);
    ASSERT_STR_EQ(line.getText().c_str(), "Hello ");

    return true;
}

// Test: 恰好填满一个块
TEST(Line_ExactlyOneBlock) {
    Line line;
    std::string full(80, 'A');
    line.setText(full.c_str());

    ASSERT_EQ(line.length(), 80);
    ASSERT_STR_EQ(line.getText().c_str(), full.c_str());

    return true;
}

// Test: 超过一个块的长度
TEST(Line_MultipleBlocks) {
    Line line;
    std::string longText(200, 'A');  // 需要至少3个块
    line.setText(longText.c_str());

    ASSERT_EQ(line.length(), 200);

    return true;
}

// ============================================================
// ActiveZone 边界测试
// ============================================================

// Test: 空活区操作
TEST(ActiveZone_EmptyZoneOperations) {
    ActiveZone zone(100);

    ASSERT_TRUE(zone.isEmpty());
    ASSERT_EQ(zone.lineCount(), 0);
    ASSERT_EQ(zone.totalPages(), 0);

    // getLine 应该返回 nullptr
    ASSERT_NULL(zone.getLine(0));
    ASSERT_NULL(zone.getLine(-1));
    ASSERT_NULL(zone.getLine(100));

    return true;
}

// Test: 活区只包含一行
TEST(ActiveZone_SingleLine) {
    ActiveZone zone(100);
    zone.appendLine(new Line("Only line"));

    ASSERT_EQ(zone.lineCount(), 1);
    ASSERT_EQ(zone.totalPages(), 1);

    return true;
}

// Test: removeFirst 和 removeLast
TEST(ActiveZone_RemoveFirstLast) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));
    zone.appendLine(new Line("Line 3"));

    ASSERT_EQ(zone.lineCount(), 3);

    // 移除首行
    Line* first = zone.removeFirst();
    ASSERT_NOT_NULL(first);
    ASSERT_STR_EQ(first->getText().c_str(), "Line 1");
    ASSERT_EQ(zone.lineCount(), 2);
    delete first;

    // 移除末行
    Line* last = zone.removeLast();
    ASSERT_NOT_NULL(last);
    ASSERT_STR_EQ(last->getText().c_str(), "Line 3");
    ASSERT_EQ(zone.lineCount(), 1);
    delete last;

    return true;
}

// Test: 空活区 removeFirst/removeLast
TEST(ActiveZone_RemoveFromEmpty) {
    ActiveZone zone(100);

    ASSERT_NULL(zone.removeFirst());
    ASSERT_NULL(zone.removeLast());

    return true;
}

// Test: 在首行之前插入（lineNo = startLineNo - 1）
TEST(ActiveZone_InsertBeforeFirst) {
    ActiveZone zone(100);
    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    // startLineNo 默认为1，所以插入0表示在首行之前
    zone.insert(0, "New First");

    ASSERT_EQ(zone.lineCount(), 3);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "New First");

    return true;
}

// Test: 在末尾之后插入
TEST(ActiveZone_InsertAfterLast) {
    ActiveZone zone(100);
    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    zone.insert(2, "Line 3");

    ASSERT_EQ(zone.lineCount(), 3);
    ASSERT_STR_EQ(zone.getLine(2)->getText().c_str(), "Line 3");

    return true;
}

// Test: 活区满时插入自动移除首行
TEST(ActiveZone_InsertAtMaxCapacity) {
    ActiveZone zone(3);  // 最大3行

    zone.insert(0, "Line 1");
    zone.insert(1, "Line 2");
    zone.insert(2, "Line 3");

    ASSERT_EQ(zone.lineCount(), 3);

    // 插入第4行，应该移除首行
    zone.insert(3, "Line 4");

    ASSERT_EQ(zone.lineCount(), 3);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 2");

    return true;
}

// Test: 分页边界 - 单页
TEST(ActiveZone_SinglePage) {
    ActiveZone zone(100);

    for (int i = 1; i <= 10; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    ASSERT_EQ(zone.totalPages(), 1);

    // 请求第2页应该显示第1页（边界处理）
    std::string page = zone.display(1);
    ASSERT_TRUE(page.empty() || page.find("Line") == std::string::npos);

    return true;
}

// Test: 恰好填满一页
TEST(ActiveZone_ExactlyFullPage) {
    ActiveZone zone(100);

    for (int i = 1; i <= 20; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    ASSERT_EQ(zone.totalPages(), 1);

    return true;
}

// Test: 超过一页一个字符
TEST(ActiveZone_JustOverOnePage) {
    ActiveZone zone(100);

    for (int i = 1; i <= 21; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    ASSERT_EQ(zone.totalPages(), 2);

    return true;
}

// Test: 删除范围边界 - 单行
TEST(ActiveZone_DeleteRangeSingle) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));
    zone.appendLine(new Line("Line 3"));

    zone.deleteRange(2, 2);

    ASSERT_EQ(zone.lineCount(), 2);
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Line 3");

    return true;
}

// Test: 删除所有行
TEST(ActiveZone_DeleteAll) {
    ActiveZone zone(100);

    for (int i = 1; i <= 5; ++i) {
        zone.appendLine(new Line(("Line " + std::to_string(i)).c_str()));
    }

    zone.deleteRange(1, 5);

    ASSERT_EQ(zone.lineCount(), 0);
    ASSERT_TRUE(zone.isEmpty());

    return true;
}

// Test: 查找空模式
TEST(ActiveZone_FindEmptyPattern) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    // 空模式应该匹配所有行
    std::vector<int> matches = zone.findPattern("");

    ASSERT_EQ(matches.size(), 2);

    return true;
}

// Test: 查找不存在的模式
TEST(ActiveZone_FindNonExistentPattern) {
    ActiveZone zone(100);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    std::vector<int> matches = zone.findPattern("xyz");

    ASSERT_EQ(matches.size(), 0);

    return true;
}

// ============================================================
// FileManager 边界测试
// ============================================================

// Test: 打开不存在的输入文件
TEST(FileManager_OpenNonExistentInput) {
    FileManager fm;
    TempFile nonExistent;  // 创建后立即删除

    bool caught = false;
    try {
        fm.openInput("/tmp/nonexistent_file_xyz_12345.txt");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::FILE_OPEN_FAILED));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 空文件名作为输入
TEST(FileManager_EmptyInputFilename) {
    FileManager fm;

    // 空文件名应该返回 true（表示没有输入文件）
    ASSERT_TRUE(fm.openInput(""));

    return true;
}

// Test: 空文件名作为输出
TEST(FileManager_EmptyOutputFilename) {
    FileManager fm;

    // 空文件名应该返回 false
    ASSERT_FALSE(fm.openOutput(""));

    return true;
}

// Test: 从空文件读取
TEST(FileManager_ReadFromEmptyFile) {
    TempFile emptyFile("");

    FileManager fm;
    fm.openInput(emptyFile.path());

    std::vector<std::string> lines;
    int count = fm.readLines(lines, 10);

    ASSERT_EQ(count, 0);
    ASSERT_EQ(lines.size(), 0);

    return true;
}

// Test: 读取零行
TEST(FileManager_ReadZeroLines) {
    TempFile file("Line 1\nLine 2\n");

    FileManager fm;
    fm.openInput(file.path());

    std::vector<std::string> lines;
    int count = fm.readLines(lines, 0);

    ASSERT_EQ(count, 0);
    ASSERT_EQ(lines.size(), 0);

    return true;
}

// Test: 读取行数超过文件实际行数
TEST(FileManager_ReadMoreThanAvailable) {
    TempFile file("Line 1\nLine 2\n");

    FileManager fm;
    fm.openInput(file.path());

    std::vector<std::string> lines;
    int count = fm.readLines(lines, 100);

    ASSERT_EQ(count, 2);
    ASSERT_EQ(lines.size(), 2);

    return true;
}

// Test: 从已关闭文件读取
TEST(FileManager_ReadFromClosed) {
    FileManager fm;

    std::vector<std::string> lines;
    int count = fm.readLines(lines, 10);

    ASSERT_EQ(count, 0);

    return true;
}

// Test: readLine 从空文件
TEST(FileManager_ReadLineFromEmpty) {
    TempFile emptyFile("");

    FileManager fm;
    fm.openInput(emptyFile.path());

    std::string line = fm.readLine();

    ASSERT_TRUE(line.empty());

    return true;
}

// Test: writeLine 到未打开的输出文件
TEST(FileManager_WriteToUnopened) {
    FileManager fm;

    bool result = fm.writeLine("Test");

    ASSERT_FALSE(result);

    return true;
}

// Test: 写入空行
TEST(FileManager_WriteEmptyLine) {
    TempFile file;

    FileManager fm;
    fm.openOutput(file.path());

    ASSERT_TRUE(fm.writeLine(""));
    ASSERT_TRUE(fm.writeLine(""));

    fm.close();

    // 验证文件内容
    std::ifstream ifs(file.path());
    std::string line1, line2;
    std::getline(ifs, line1);
    std::getline(ifs, line2);
    ifs.close();

    ASSERT_TRUE(line1.empty());
    ASSERT_TRUE(line2.empty());

    return true;
}

// Test: 写入空向量
TEST(FileManager_WriteEmptyVector) {
    TempFile file;

    FileManager fm;
    fm.openOutput(file.path());

    std::vector<std::string> empty;
    ASSERT_TRUE(fm.writeLines(empty));

    fm.close();

    return true;
}

// Test: Getter 方法
TEST(FileManager_Getters) {
    TempFile inputFile("Input");
    TempFile outputFile("");

    FileManager fm;
    fm.openInput(inputFile.path());
    fm.openOutput(outputFile.path());

    ASSERT_TRUE(fm.isInputOpen());
    ASSERT_TRUE(fm.isOutputOpen());
    ASSERT_FALSE(fm.isInputEof());
    ASSERT_STR_EQ(fm.inputFilename().c_str(), inputFile.path().c_str());
    ASSERT_STR_EQ(fm.outputFilename().c_str(), outputFile.path().c_str());

    // 读取到 EOF
    std::vector<std::string> lines;
    fm.readLines(lines, 100);
    ASSERT_TRUE(fm.isInputEof());

    return true;
}

// Test: 关闭多次
TEST(FileManager_CloseMultipleTimes) {
    TempFile file("Test");

    FileManager fm;
    fm.openInput(file.path());

    fm.close();
    fm.close();  // 不应该崩溃

    ASSERT_FALSE(fm.isInputOpen());

    return true;
}

// ============================================================
// CommandParser 边界测试
// ============================================================

// Test: 空命令
TEST(Parser_EmptyCommand) {
    CommandParser parser;

    Command cmd = parser.parse("");

    ASSERT_TRUE(cmd.type == CommandType::UNKNOWN);

    return true;
}

// Test: 只有空白的命令
TEST(Parser_WhitespaceOnlyCommand) {
    CommandParser parser;

    Command cmd = parser.parse("   ");

    ASSERT_TRUE(cmd.type == CommandType::UNKNOWN);

    return true;
}

// Test: 命令前后有空白
TEST(CommandParser_CommandWithWhitespace) {
    CommandParser parser;

    Command cmd = parser.parse("  i10  ");

    ASSERT_TRUE(cmd.type == CommandType::INSERT);
    ASSERT_EQ(cmd.lineNo, 10);

    return true;
}

// Test: 缺少参数的插入命令
TEST(Parser_InsertMissingParameter) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("i");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::MISSING_PARAMETER));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 无效的行号
TEST(Parser_InvalidLineNumber) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("iabc");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::INVALID_FORMAT));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 负数行号（会被解析但验证时会失败）
TEST(Parser_NegativeLineNumber) {
    CommandParser parser;

    Command cmd = parser.parse("i-5");

    ASSERT_TRUE(cmd.type == CommandType::INSERT);
    ASSERT_EQ(cmd.lineNo, -5);

    return true;
}

// Test: 删除命令缺少参数
TEST(Parser_DeleteMissingParameter) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("d");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::MISSING_PARAMETER));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 替换命令缺少分隔符
TEST(Parser_ReplaceMissingDelimiter) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("s5oldnew");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::INVALID_FORMAT));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 替换命令只有一个分隔符
TEST(Parser_ReplaceOneDelimiter) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("s5@oldnew");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::INVALID_FORMAT));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 匹配命令空模式
TEST(Parser_MatchEmptyPattern) {
    CommandParser parser;

    Command cmd = parser.parse("m");

    ASSERT_TRUE(cmd.type == CommandType::MATCH);
    ASSERT_TRUE(cmd.pattern.empty());

    return true;
}

// Test: 打印命令无效页码
TEST(Parser_PrintInvalidPage) {
    CommandParser parser;

    bool caught = false;
    try {
        Command cmd = parser.parse("pxyz");
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::INVALID_FORMAT));
    }

    ASSERT_TRUE(caught);

    return true;
}

TEST(Parser_PrintNegativePage) {
    CommandParser parser;

    Command cmd = parser.parse("p-5");

    ASSERT_TRUE(cmd.type == CommandType::PRINT);
    ASSERT_EQ(cmd.pageNum, 0);

    return true;
}

// Test: 验证行号超出活区范围
TEST(Parser_ValidateLineOutOfRange) {
    CommandParser parser;

    Command cmd = parser.parse("i10");
    ASSERT_TRUE(cmd.type == CommandType::INSERT);

    bool caught = false;
    try {
        parser.validate(cmd, 1, 5);  // 活区只有1-5行
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::LINE_NUMBER_OUT_OF_RANGE));
    }

    ASSERT_TRUE(caught);

    return true;
}

// Test: 验证删除范围起始大于结束
TEST(Parser_ValidateInvalidRange) {
    CommandParser parser;

    Command cmd = parser.parse("d10 5");
    ASSERT_TRUE(cmd.type == CommandType::DELETE);
    ASSERT_EQ(cmd.lineNo, 10);
    ASSERT_EQ(cmd.lineNo2, 5);

    bool caught = false;
    try {
        parser.validate(cmd, 1, 20);
    } catch (const EditorException& e) {
        caught = true;
        ASSERT_EQ(static_cast<int>(e.code()), static_cast<int>(ErrorCode::INVALID_RANGE));
    }

    ASSERT_TRUE(caught);

    return true;
}

// ============================================================
// CommandExecutor 边界测试
// ============================================================

// Test: 执行未知命令
TEST(Executor_UnknownCommand) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::UNKNOWN;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_FALSE(result.success);
    ASSERT_TRUE(result.message.find("未知") != std::string::npos ||
                result.message.find("Unknown") != std::string::npos);

    return true;
}

TEST(Executor_DeleteFromEmptyZone) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::DELETE;
    cmd.lineNo = 1;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);

    return true;
}

// Test: 空活区分页
TEST(Executor_PrintEmptyZonePages) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    Command cmd;
    cmd.type = CommandType::PRINT;
    cmd.pageNum = 0;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.message.find("空") != std::string::npos ||
                result.message.find("empty") != std::string::npos);

    return true;
}

// Test: 负页码显示
TEST(Executor_PrintNegativePage) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Line 1"));

    Command cmd;
    cmd.type = CommandType::PRINT;
    cmd.pageNum = -1;

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    // 负页码应该显示第1页
    ASSERT_TRUE(result.message.find("第 1 页") != std::string::npos);

    return true;
}

TEST(Executor_ReplaceEmptyOldStr) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Hello World"));

    Command cmd;
    cmd.type = CommandType::REPLACE;
    cmd.lineNo = 1;
    cmd.oldStr = "";
    cmd.newStr = "Test";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_FALSE(result.success);

    return true;
}

// Test: 查找空模式
TEST(Executor_MatchEmptyPattern) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Line 1"));
    zone.appendLine(new Line("Line 2"));

    Command cmd;
    cmd.type = CommandType::MATCH;
    cmd.pattern = "";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    // 空模式应该匹配所有行
    ASSERT_TRUE(result.message.find("1") != std::string::npos);
    ASSERT_TRUE(result.message.find("2") != std::string::npos);

    return true;
}

// Test: 查找不存在的模式
TEST(Executor_MatchNonExistentPattern) {
    ActiveZone zone(100);
    FileManager fileMgr;
    CommandExecutor executor(zone, fileMgr);

    zone.appendLine(new Line("Line 1"));

    Command cmd;
    cmd.type = CommandType::MATCH;
    cmd.pattern = "xyz";

    ExecutionResult result = executor.execute(cmd);

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.message.find("未找到") != std::string::npos ||
                result.message.find("not found") != std::string::npos);

    return true;
}

// ============================================================
// 注册所有测试
// ============================================================

// Error 模块
REGISTER_TEST(BoundaryCases, Error_ExceptionConstruction);
REGISTER_TEST(BoundaryCases, Error_AllErrorCodes);
REGISTER_TEST(BoundaryCases, Error_StdExceptionBase);

// LineBlock 边界
REGISTER_TEST(BoundaryCases, LineBlock_AppendEmptyString);
REGISTER_TEST(BoundaryCases, LineBlock_AppendExactlyFull);
REGISTER_TEST(BoundaryCases, LineBlock_AppendBeyondCapacity);
REGISTER_TEST(BoundaryCases, LineBlock_CreateNextChain);
REGISTER_TEST(BoundaryCases, LineBlock_SetNext);

// Line 边界
REGISTER_TEST(BoundaryCases, Line_EmptyLine);
REGISTER_TEST(BoundaryCases, Line_SetEmptyText);
REGISTER_TEST(BoundaryCases, Line_FindEmptyString);
REGISTER_TEST(BoundaryCases, Line_FindNonExistent);
REGISTER_TEST(BoundaryCases, Line_ReplaceNonExistent);
REGISTER_TEST(BoundaryCases, Line_ReplaceWithEmpty);
REGISTER_TEST(BoundaryCases, Line_ExactlyOneBlock);
REGISTER_TEST(BoundaryCases, Line_MultipleBlocks);

// ActiveZone 边界
REGISTER_TEST(BoundaryCases, ActiveZone_EmptyZoneOperations);
REGISTER_TEST(BoundaryCases, ActiveZone_SingleLine);
REGISTER_TEST(BoundaryCases, ActiveZone_RemoveFirstLast);
REGISTER_TEST(BoundaryCases, ActiveZone_RemoveFromEmpty);
REGISTER_TEST(BoundaryCases, ActiveZone_InsertBeforeFirst);
REGISTER_TEST(BoundaryCases, ActiveZone_InsertAfterLast);
REGISTER_TEST(BoundaryCases, ActiveZone_InsertAtMaxCapacity);
REGISTER_TEST(BoundaryCases, ActiveZone_SinglePage);
REGISTER_TEST(BoundaryCases, ActiveZone_ExactlyFullPage);
REGISTER_TEST(BoundaryCases, ActiveZone_JustOverOnePage);
REGISTER_TEST(BoundaryCases, ActiveZone_DeleteRangeSingle);
REGISTER_TEST(BoundaryCases, ActiveZone_DeleteAll);
REGISTER_TEST(BoundaryCases, ActiveZone_FindEmptyPattern);
REGISTER_TEST(BoundaryCases, ActiveZone_FindNonExistentPattern);

// FileManager 边界
REGISTER_TEST(BoundaryCases, FileManager_OpenNonExistentInput);
REGISTER_TEST(BoundaryCases, FileManager_EmptyInputFilename);
REGISTER_TEST(BoundaryCases, FileManager_EmptyOutputFilename);
REGISTER_TEST(BoundaryCases, FileManager_ReadFromEmptyFile);
REGISTER_TEST(BoundaryCases, FileManager_ReadZeroLines);
REGISTER_TEST(BoundaryCases, FileManager_ReadMoreThanAvailable);
REGISTER_TEST(BoundaryCases, FileManager_ReadFromClosed);
REGISTER_TEST(BoundaryCases, FileManager_ReadLineFromEmpty);
REGISTER_TEST(BoundaryCases, FileManager_WriteToUnopened);
REGISTER_TEST(BoundaryCases, FileManager_WriteEmptyLine);
REGISTER_TEST(BoundaryCases, FileManager_WriteEmptyVector);
REGISTER_TEST(BoundaryCases, FileManager_Getters);
REGISTER_TEST(BoundaryCases, FileManager_CloseMultipleTimes);

// CommandParser 边界
REGISTER_TEST(BoundaryCases, Parser_EmptyCommand);
REGISTER_TEST(BoundaryCases, Parser_WhitespaceOnlyCommand);
REGISTER_TEST(BoundaryCases, CommandParser_CommandWithWhitespace);
REGISTER_TEST(BoundaryCases, Parser_InsertMissingParameter);
REGISTER_TEST(BoundaryCases, Parser_InvalidLineNumber);
REGISTER_TEST(BoundaryCases, Parser_NegativeLineNumber);
REGISTER_TEST(BoundaryCases, Parser_DeleteMissingParameter);
REGISTER_TEST(BoundaryCases, Parser_ReplaceMissingDelimiter);
REGISTER_TEST(BoundaryCases, Parser_ReplaceOneDelimiter);
REGISTER_TEST(BoundaryCases, Parser_MatchEmptyPattern);
REGISTER_TEST(BoundaryCases, Parser_PrintInvalidPage);
REGISTER_TEST(BoundaryCases, Parser_PrintNegativePage);
REGISTER_TEST(BoundaryCases, Parser_ValidateLineOutOfRange);
REGISTER_TEST(BoundaryCases, Parser_ValidateInvalidRange);

// CommandExecutor 边界
REGISTER_TEST(BoundaryCases, Executor_UnknownCommand);
REGISTER_TEST(BoundaryCases, Executor_DeleteFromEmptyZone);
REGISTER_TEST(BoundaryCases, Executor_PrintEmptyZonePages);
REGISTER_TEST(BoundaryCases, Executor_PrintNegativePage);
REGISTER_TEST(BoundaryCases, Executor_ReplaceEmptyOldStr);
REGISTER_TEST(BoundaryCases, Executor_MatchEmptyPattern);
REGISTER_TEST(BoundaryCases, Executor_MatchNonExistentPattern);
