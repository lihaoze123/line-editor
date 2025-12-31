#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include "command_parser.h"
#include "active_zone.h"
#include "file_manager.h"
#include <string>

namespace line_editor {

struct ExecutionResult {
    bool success;
    std::string message;
    std::string output;
    bool shouldExit;
    bool needsInput;

    ExecutionResult()
        : success(true), shouldExit(false), needsInput(false) {}
};

class CommandExecutor {
public:
    CommandExecutor(ActiveZone& zone, FileManager& fileMgr);
    ~CommandExecutor() = default;

    ExecutionResult execute(const Command& cmd);

    ExecutionResult executeInsert(int lineNo, const std::string& text);

    void setPendingInsertLineNo(int lineNo) { pendingInsertLineNo_ = lineNo; }
    int getPendingInsertLineNo() const { return pendingInsertLineNo_; }
    void clearPendingInsert() { pendingInsertLineNo_ = -1; }

private:
    ActiveZone& zone_;
    FileManager& fileMgr_;
    int pendingInsertLineNo_;

    ExecutionResult executeInsert(const Command& cmd);
    ExecutionResult executeDelete(const Command& cmd);
    ExecutionResult executeNextZone(const Command& cmd);
    ExecutionResult executePrint(const Command& cmd);
    ExecutionResult executeReplace(const Command& cmd);
    ExecutionResult executeMatch(const Command& cmd);
    ExecutionResult executeQuit(const Command& cmd);
};

} // namespace line_editor

#endif // COMMAND_EXECUTOR_H
