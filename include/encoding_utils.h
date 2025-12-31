#ifndef ENCODING_UTILS_H
#define ENCODING_UTILS_H

#include <cstddef>

namespace line_editor {

/**
 * Initialize console encoding for Windows.
 * On Windows, sets console to UTF-8 mode for proper display of Unicode characters.
 * On other platforms, does nothing.
 *
 * @return true if successful or not applicable (non-Windows), false on failure
 */
bool initializeConsoleEncoding();

/**
 * Detect UTF-8 BOM (Byte Order Mark) in data.
 * UTF-8 BOM is the byte sequence: EF BB BF
 *
 * @param data The beginning of file data
 * @param size Size of the data
 * @return Number of BOM bytes to skip (0 or 3)
 */
size_t detectUtf8Bom(const char* data, size_t size);

} // namespace line_editor

#endif // ENCODING_UTILS_H
