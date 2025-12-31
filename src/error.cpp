#include "error.h"

namespace line_editor {

EditorException::EditorException(ErrorCode code, const std::string& msg)
    : std::runtime_error(msg), code_(code) {}

} // namespace line_editor
