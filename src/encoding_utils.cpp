#include "encoding_utils.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace line_editor {

// CP_UTF8 is already defined in windows.h as 65001

bool initializeConsoleEncoding() {
#ifdef _WIN32
    // Set console code pages to UTF-8
    // These functions are available on all Windows versions since NT
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    return true;
#else
    // On non-Windows platforms (Linux, macOS), UTF-8 is the default
    return true;
#endif
}

size_t detectUtf8Bom(const char* data, size_t size) {
    // UTF-8 BOM is: EF BB BF
    if (size >= 3 &&
        static_cast<unsigned char>(data[0]) == 0xEF &&
        static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        return 3;
    }
    return 0;
}

} // namespace line_editor
