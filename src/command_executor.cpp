#include "command_executor.h"
#include <sstream>
#include <iomanip>

namespace line_editor {

CommandExecutor::CommandExecutor(ActiveZone& zone, FileManager& fileMgr)
    : zone_(zone), fileMgr_(fileMgr), pendingInsertLineNo_(-1) {
}

ExecutionResult CommandExecutor::execute(const Command& cmd) {
    switch (cmd.type) {
        case CommandType::INSERT:
            return executeInsert(cmd);
        case CommandType::DELETE:
            return executeDelete(cmd);
        case CommandType::NEXT_ZONE:
            return executeNextZone(cmd);
        case CommandType::PRINT:
            return executePrint(cmd);
        case CommandType::REPLACE:
            return executeReplace(cmd);
        case CommandType::MATCH:
            return executeMatch(cmd);
        case CommandType::QUIT:
            return executeQuit(cmd);
        default:
            ExecutionResult result;
            result.success = false;
            result.message = "未知命令";
            return result;
    }
}

ExecutionResult CommandExecutor::executeInsert(int lineNo, const std::string& text) {
    ExecutionResult result;

    try {
        zone_.insert(lineNo, text.c_str());
        result.success = true;
        result.message = "已在第 " + std::to_string(lineNo) + " 行后插入";
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executeInsert(const Command& cmd) {
    ExecutionResult result;

    if (cmd.text.empty()) {
        result.needsInput = true;
        setPendingInsertLineNo(cmd.lineNo);
        result.message = "请输入要插入的文本（空行完成）:";
        return result;
    }

    return executeInsert(cmd.lineNo, cmd.text);
}

ExecutionResult CommandExecutor::executeDelete(const Command& cmd) {
    ExecutionResult result;

    try {
        if (cmd.lineNo2 != 0) {
            zone_.deleteRange(cmd.lineNo, cmd.lineNo2);
            result.message = "已删除第 " + std::to_string(cmd.lineNo) +
                           " 到 " + std::to_string(cmd.lineNo2) + " 行";
        } else {
            zone_.deleteLine(cmd.lineNo);
            result.message = "已删除第 " + std::to_string(cmd.lineNo) + " 行";
        }
        result.success = true;
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executeNextZone(const Command& cmd) {
    ExecutionResult result;

    try {
        if (fileMgr_.isOutputOpen()) {
            for (Line* line = zone_.head(); line; line = line->next()) {
                fileMgr_.writeLine(line->getText());
            }
        }

        int newStart = zone_.startLineNo() + zone_.lineCount();
        zone_.clear();
        zone_.setStartLineNo(newStart);

        if (fileMgr_.isInputOpen() && !fileMgr_.isInputEof()) {
            std::vector<std::string> lines;
            int count = fileMgr_.readLines(lines, 80);

            for (const auto& lineStr : lines) {
                zone_.appendLine(new Line(lineStr.c_str()));
            }

            result.message = "活区已刷新。已加载 " + std::to_string(count) + " 行。";
        } else {
            result.message = "活区已写入输出。没有更多输入。";
        }

        result.success = true;
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executePrint(const Command& cmd) {
    ExecutionResult result;

    try {
        if (zone_.isEmpty()) {
            result.message = "活区为空";
        } else {
            int totalPages = zone_.totalPages();
            int displayPage = cmd.pageNum;

            if (displayPage < 0) {
                displayPage = 0;
            } else if (displayPage >= totalPages) {
                displayPage = totalPages - 1;
            }

            result.output = zone_.display(displayPage);
            result.message = "正在显示第 " + std::to_string(displayPage + 1) +
                           " 页，共 " + std::to_string(totalPages) + " 页";
        }
        result.success = true;
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executeReplace(const Command& cmd) {
    ExecutionResult result;

    try {
        bool replaced = zone_.replaceInLine(cmd.lineNo, cmd.oldStr.c_str(), cmd.newStr.c_str());

        if (replaced) {
            result.message = "已在第 " + std::to_string(cmd.lineNo) + " 行将 '" + cmd.oldStr + "' 替换为 '" + cmd.newStr + "'";
            result.success = true;
        } else {
            result.message = "在第 " + std::to_string(cmd.lineNo) + " 行中未找到模式 '" + cmd.oldStr + "'";
            result.success = false;
        }
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executeMatch(const Command& cmd) {
    ExecutionResult result;

    try {
        std::vector<int> matches = zone_.findPattern(cmd.pattern.c_str());

        if (matches.empty()) {
            result.message = "未找到模式 '" + cmd.pattern + "'";
        } else {
            std::ostringstream oss;
            oss << "模式 '" << cmd.pattern << "' 在以下行中找到: ";
            for (size_t i = 0; i < matches.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << matches[i];
            }
            result.message = oss.str();
        }
        result.success = true;
    } catch (const EditorException& e) {
        result.success = false;
        result.message = e.what();
    }

    return result;
}

ExecutionResult CommandExecutor::executeQuit(const Command& cmd) {
    ExecutionResult result;
    result.success = true;
    result.shouldExit = true;
    result.message = "正在退出编辑器...";
    return result;
}

} // namespace line_editor
