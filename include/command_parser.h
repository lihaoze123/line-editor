#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

// Undefine Windows macros that conflict with CommandType enum values
// Windows.h defines DELETE and INSERT as macros that must be removed
// unconditional #undef is safe - if macro isn't defined, it's a no-op
#undef DELETE
#undef INSERT

#include "error.h"
#include <string>

namespace line_editor {

enum class CommandType {
    INSERT,
    DELETE,
    NEXT_ZONE,
    PRINT,
    REPLACE,
    MATCH,
    QUIT,
    UNKNOWN
};

struct Command {
    CommandType type;
    std::string raw;

    int lineNo;
    int lineNo2;
    int pageNum;
    std::string text;
    std::string oldStr;
    std::string newStr;
    std::string pattern;

    Command() : type(CommandType::UNKNOWN), lineNo(0), lineNo2(0), pageNum(0) {}
};

class CommandParser {
public:
    CommandParser() = default;
    ~CommandParser() = default;

    Command parse(const std::string& input) const;

    void validate(const Command& cmd, int zoneStart, int zoneEnd) const;

    static int parseLineNumber(const std::string& str);

private:
    Command parseInsert(const std::string& input) const;
    Command parseDelete(const std::string& input) const;
    Command parsePrint(const std::string& input) const;
    Command parseReplace(const std::string& input) const;
    Command parseMatch(const std::string& input) const;
};

} // namespace line_editor

#endif // COMMAND_PARSER_H
