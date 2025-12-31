#include "command_parser.h"
#include <cctype>
#include <sstream>
#include <algorithm>

namespace line_editor {

Command CommandParser::parse(const std::string& input) const {
    std::string trimmed = input;
    trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
        [](int ch) { return !std::isspace(ch); }));
    trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(),
        [](int ch) { return !std::isspace(ch); }).base(), trimmed.end());

    if (trimmed.empty()) {
        Command cmd;
        cmd.type = CommandType::UNKNOWN;
        cmd.raw = input;
        return cmd;
    }

    Command cmd;
    cmd.raw = input;

    char first = std::tolower(trimmed[0]);

    switch (first) {
        case 'i':
            return parseInsert(trimmed);
        case 'd':
            return parseDelete(trimmed);
        case 'n':
            cmd.type = CommandType::NEXT_ZONE;
            return cmd;
        case 'p':
            return parsePrint(trimmed);
        case 's':
            return parseReplace(trimmed);
        case 'm':
            return parseMatch(trimmed);
        case 'q':
            cmd.type = CommandType::QUIT;
            return cmd;
        default:
            cmd.type = CommandType::UNKNOWN;
            return cmd;
    }
}

void CommandParser::validate(const Command& cmd, int zoneStart, int zoneEnd) const {
    switch (cmd.type) {
        case CommandType::INSERT:
            if (cmd.lineNo < zoneStart - 1 || cmd.lineNo > zoneEnd) {
                throw EditorException(ErrorCode::LINE_NUMBER_OUT_OF_RANGE,
                    "插入行号超出范围");
            }
            break;

        case CommandType::DELETE:
            if (cmd.lineNo < zoneStart || cmd.lineNo > zoneEnd) {
                throw EditorException(ErrorCode::LINE_NUMBER_OUT_OF_RANGE,
                    "删除行号超出范围");
            }
            if (cmd.lineNo2 != 0) {
                if (cmd.lineNo2 < zoneStart || cmd.lineNo2 > zoneEnd) {
                    throw EditorException(ErrorCode::LINE_NUMBER_OUT_OF_RANGE,
                        "删除结束行号超出范围");
                }
                if (cmd.lineNo > cmd.lineNo2) {
                    throw EditorException(ErrorCode::INVALID_RANGE,
                        "起始行号大于结束行号");
                }
            }
            break;

        case CommandType::REPLACE:
            if (cmd.lineNo < zoneStart || cmd.lineNo > zoneEnd) {
                throw EditorException(ErrorCode::LINE_NUMBER_OUT_OF_RANGE,
                    "替换行号超出范围");
            }
            break;

        default:
            break;
    }
}

int CommandParser::parseLineNumber(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (...) {
        throw EditorException(ErrorCode::INVALID_FORMAT,
            "无效的行号: " + str);
    }
}

Command CommandParser::parseInsert(const std::string& input) const {
    Command cmd;
    cmd.type = CommandType::INSERT;

    if (input.length() < 2) {
        throw EditorException(ErrorCode::MISSING_PARAMETER,
            "插入命令需要行号: i<行号>");
    }

    size_t spacePos = input.find(' ', 1);
    std::string numStr;

    if (spacePos != std::string::npos) {
        numStr = input.substr(1, spacePos - 1);
        cmd.text = input.substr(spacePos + 1);
    } else {
        numStr = input.substr(1);
    }

    cmd.lineNo = parseLineNumber(numStr);
    return cmd;
}

Command CommandParser::parseDelete(const std::string& input) const {
    Command cmd;
    cmd.type = CommandType::DELETE;

    if (input.length() < 2) {
        throw EditorException(ErrorCode::MISSING_PARAMETER,
            "删除命令需要行号: d<行号> 或 d<起始> <结束>");
    }

    size_t spacePos = input.find(' ', 1);

    if (spacePos != std::string::npos) {
        std::string startStr = input.substr(1, spacePos - 1);
        std::string endStr = input.substr(spacePos + 1);
        endStr.erase(endStr.begin(), std::find_if(endStr.begin(), endStr.end(),
            [](int ch) { return !std::isspace(ch); }));

        cmd.lineNo = parseLineNumber(startStr);
        cmd.lineNo2 = parseLineNumber(endStr);
    } else {
        cmd.lineNo = parseLineNumber(input.substr(1));
        cmd.lineNo2 = 0;
    }

    return cmd;
}

Command CommandParser::parseReplace(const std::string& input) const {
    Command cmd;
    cmd.type = CommandType::REPLACE;

    if (input.length() < 3) {
        throw EditorException(ErrorCode::MISSING_PARAMETER,
            "替换命令需要: s<行号>@<旧字符串>@<新字符串>");
    }

    size_t at1 = input.find('@', 1);
    if (at1 == std::string::npos) {
        throw EditorException(ErrorCode::INVALID_FORMAT,
            "替换命令需要 @ 分隔符: s<行号>@<旧字符串>@<新字符串>");
    }

    size_t at2 = input.find('@', at1 + 1);
    if (at2 == std::string::npos) {
        throw EditorException(ErrorCode::INVALID_FORMAT,
            "替换命令需要两个 @ 分隔符: s<行号>@<旧字符串>@<新字符串>");
    }

    std::string lineStr = input.substr(1, at1 - 1);
    cmd.lineNo = parseLineNumber(lineStr);
    cmd.oldStr = input.substr(at1 + 1, at2 - at1 - 1);
    cmd.newStr = input.substr(at2 + 1);

    return cmd;
}

Command CommandParser::parseMatch(const std::string& input) const {
    Command cmd;
    cmd.type = CommandType::MATCH;

    if (input.length() > 1) {
        cmd.pattern = input.substr(1);
    } else {
        cmd.pattern = "";
    }

    return cmd;
}

Command CommandParser::parsePrint(const std::string& input) const {
    Command cmd;
    cmd.type = CommandType::PRINT;

    if (input.length() > 1) {
        try {
            int page = parseLineNumber(input.substr(1));
            cmd.pageNum = (page > 0) ? (page - 1) : 0;
        } catch (...) {
            throw EditorException(ErrorCode::INVALID_FORMAT,
                "无效的页码: " + input.substr(1));
        }
    } else {
        cmd.pageNum = 0;
    }

    return cmd;
}

} // namespace line_editor
