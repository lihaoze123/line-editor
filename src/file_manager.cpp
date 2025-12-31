#include "file_manager.h"
#include "error.h"
#include "encoding_utils.h"
#include <iostream>

namespace line_editor {

bool FileManager::openInput(const std::string& filename) {
    if (filename.empty()) {
        return true;
    }

    input_.close();
    input_.open(filename);
    inputFilename_ = filename;
    bomChecked_ = false;  // Reset BOM flag for new file

    if (!input_.is_open()) {
        throw EditorException(ErrorCode::FILE_OPEN_FAILED,
            "Failed to open input file: " + filename);
    }

    return true;
}

bool FileManager::openOutput(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }

    output_.close();
    output_.open(filename);
    outputFilename_ = filename;

    if (!output_.is_open()) {
        throw EditorException(ErrorCode::FILE_OPEN_FAILED,
            "Failed to open output file: " + filename);
    }

    return true;
}

void FileManager::close() {
    if (input_.is_open()) {
        input_.close();
    }
    if (output_.is_open()) {
        output_.close();
    }
}

void FileManager::skipUtf8Bom() {
    if (bomChecked_) {
        return;
    }

    if (!input_.is_open() || input_.tellg() != 0) {
        return;
    }

    char bom[3] = {0};
    input_.read(bom, 3);
    std::streamsize bytesRead = input_.gcount();

    // Only check BOM if we actually read 3 bytes
    if (bytesRead == 3 && detectUtf8Bom(bom, 3) == 3) {
        // BOM found and skipped, we're past it
    } else {
        // Not a BOM or file too small, seek back
        input_.clear();  // Clear any flags (especially eofbit)
        input_.seekg(0, std::ios::beg);
    }
    bomChecked_ = true;
}

int FileManager::readLines(std::vector<std::string>& lines, int maxLines) {
    lines.clear();

    if (!input_.is_open() || input_.eof()) {
        return 0;
    }

    skipUtf8Bom();

    std::string line;
    int count = 0;

    while (count < maxLines && std::getline(input_, line)) {
        lines.push_back(line);
        count++;
    }

    return count;
}

std::string FileManager::readLine() {
    if (!input_.is_open() || input_.eof()) {
        return "";
    }

    skipUtf8Bom();

    std::string line;
    if (std::getline(input_, line)) {
        return line;
    }
    return "";
}

bool FileManager::writeLine(const std::string& line) {
    if (!output_.is_open()) {
        return false;
    }

    output_ << line << "\n";

    if (output_.fail()) {
        throw EditorException(ErrorCode::FILE_WRITE_FAILED,
            "Failed to write to output file");
    }

    return true;
}

bool FileManager::writeLines(const std::vector<std::string>& lines) {
    if (!output_.is_open()) {
        return false;
    }

    for (const auto& line : lines) {
        output_ << line << "\n";
    }

    if (output_.fail()) {
        throw EditorException(ErrorCode::FILE_WRITE_FAILED,
            "Failed to write to output file");
    }

    return true;
}

} // namespace line_editor
