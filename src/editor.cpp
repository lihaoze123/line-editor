#include "editor.h"
#include <iostream>
#include <iomanip>

namespace line_editor {

Editor::Editor()
    : zone_(DEFAULT_MAX_LINES),
      executor_(zone_, fileMgr_),
      initialized_(false) {
}

bool Editor::init(const std::string& inputFile, const std::string& outputFile) {
    if (inputFile == outputFile && !inputFile.empty()) {
        std::cerr << "错误: 输入文件和输出文件不能相同。\n";
        return false;
    }

    inputFile_ = inputFile;
    outputFile_ = outputFile;

    if (!inputFile.empty()) {
        try {
            fileMgr_.openInput(inputFile);
        } catch (const EditorException& e) {
            std::cerr << "错误: " << e.what() << "\n";
            return false;
        }
    }

    if (!outputFile.empty()) {
        try {
            fileMgr_.openOutput(outputFile);
        } catch (const EditorException& e) {
            std::cerr << "错误: " << e.what() << "\n";
            return false;
        }
    }

    if (fileMgr_.isInputOpen()) {
        std::vector<std::string> lines;
        fileMgr_.readLines(lines, 80);

        for (const auto& lineStr : lines) {
            zone_.appendLine(new Line(lineStr.c_str()));
        }
    }

    initialized_ = true;
    return true;
}

void Editor::run() {
    if (!initialized_) {
        std::cerr << "编辑器未初始化。请先调用 init()。\n";
        return;
    }

    showWelcome();

    std::string input;
    bool running = true;

    while (running) {
        std::cout << "\n> ";
        std::flush(std::cout);

        if (!std::getline(std::cin, input)) {
            break;
        }

        running = processCommand(input);
    }

    if (fileMgr_.isOutputOpen() && !zone_.isEmpty()) {
        for (Line* line = zone_.head(); line; line = line->next()) {
            fileMgr_.writeLine(line->getText());
        }
    }

    fileMgr_.close();
}

void Editor::showWelcome() const {
    std::cout << "\n===========================================\n";
    std::cout << "     简易行编辑器 v1.0\n";
    std::cout << "===========================================\n";
    std::cout << "输入 'h' 获取帮助，'q' 退出\n";

    if (!zone_.isEmpty()) {
        std::cout << "\n已加载 " << zone_.lineCount() << " 行";
        if (!inputFile_.empty()) {
            std::cout << "，来自 " << inputFile_;
        }
        std::cout << ".\n";
        displayZone();
    }
}

void Editor::showHelp() const {
    std::cout << "\n命令:\n";
    std::cout << "  i<n> [文本]  - 在第 n 行后插入文本（n=0 表示在第一行之前）\n";
    std::cout << "  d<n>         - 删除第 n 行\n";
    std::cout << "  d<n1> <n2>   - 删除第 n1 到 n2 行\n";
    std::cout << "  n            - 下一活区（保存当前，加载下一个）\n";
    std::cout << "  p [n]        - 打印当前活区（n=页码，默认第1页）\n";
    std::cout << "  s<n>@o@n     - 在第 n 行将 'o' 替换为 'n'\n";
    std::cout << "  m<pattern>   - 在活区中查找模式\n";
    std::cout << "  h            - 显示此帮助\n";
    std::cout << "  q            - 退出编辑器\n";
}

void Editor::displayZone(int page) const {
    if (zone_.isEmpty()) {
        std::cout << "\n[活区为空]\n";
        return;
    }

    std::cout << "\n" << zone_.display(page);
    std::cout << "\n已显示第 " << zone_.startLineNo() << " - "
              << (zone_.startLineNo() + zone_.lineCount() - 1) << " 行。\n";
}

bool Editor::processCommand(const std::string& input) {
    if (input.empty()) {
        return true;
    }

    if (input[0] == 'h' || input[0] == 'H') {
        showHelp();
        return true;
    }

    Command cmd;
    try {
        cmd = parser_.parse(input);
    } catch (const EditorException& e) {
        std::cerr << "解析错误: " << e.what() << "\n";
        return true;
    }

    if (cmd.type == CommandType::UNKNOWN) {
        std::cerr << "未知命令: " << input << "\n";
        std::cerr << "输入 'h' 获取帮助。\n";
        return true;
    }

    try {
        parser_.validate(cmd, zone_.startLineNo(),
                        zone_.startLineNo() + zone_.lineCount() - 1);
    } catch (const EditorException& e) {
        std::cerr << "验证错误: " << e.what() << "\n";
        return true;
    }

    ExecutionResult result = executor_.execute(cmd);

    if (!result.success) {
        std::cerr << "错误: " << result.message << "\n";
        return true;
    }

    if (result.shouldExit) {
        std::cout << result.message << "\n";
        return false;
    }

    if (result.needsInput) {
        std::cout << result.message << "\n";
        handleInsertMode(executor_.getPendingInsertLineNo());
        return true;
    }

    if (!result.message.empty()) {
        std::cout << result.message << "\n";
    }

    if (!result.output.empty()) {
        std::cout << "\n" << result.output;
    }

    if (cmd.type == CommandType::INSERT || cmd.type == CommandType::DELETE ||
        cmd.type == CommandType::REPLACE) {
        displayZone();
    }

    return true;
}

void Editor::handleInsertMode(int lineNo) {
    std::string text;
    int insertedCount = 0;

    while (true) {
        std::cout << "  ";
        if (!std::getline(std::cin, text)) {
            break;
        }

        if (text.empty()) {
            break;
        }

        try {
            zone_.insert(lineNo + insertedCount, text.c_str());
            insertedCount++;
        } catch (const EditorException& e) {
            std::cerr << "Error: " << e.what() << "\n";
            break;
        }
    }

    if (insertedCount > 0) {
        std::cout << "已插入 " << insertedCount << " 行。\n";
        displayZone();
    }

    executor_.clearPendingInsert();
}

} // namespace line_editor
