#include "file_manager.h"
#include "error.h"
#include <iostream>

namespace line_editor {

bool FileManager::openInput(const std::string& filename) {
    if (filename.empty()) {
        return true;
    }

    input_.close();
    input_.open(filename);
    inputFilename_ = filename;

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

int FileManager::readLines(std::vector<std::string>& lines, int maxLines) {
    lines.clear();

    if (!input_.is_open() || input_.eof()) {
        return 0;
    }

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
