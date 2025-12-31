#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <stdexcept>

namespace line_editor {

enum class ErrorCode {
    SUCCESS = 0,
    UNKNOWN_COMMAND,
    LINE_NUMBER_OUT_OF_RANGE,
    MISSING_PARAMETER,
    INVALID_RANGE,
    FILE_OPEN_FAILED,
    FILE_WRITE_FAILED,
    MEMORY_ALLOCATION_FAILED,
    INVALID_FORMAT,
    PATTERN_NOT_FOUND,
    EMPTY_ACTIVE_ZONE
};

class EditorException : public std::runtime_error {
public:
    EditorException(ErrorCode code, const std::string& msg);
    ErrorCode code() const { return code_; }

private:
    ErrorCode code_;
};

} // namespace line_editor

#endif // ERROR_H
