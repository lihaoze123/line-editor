#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <fstream>

namespace line_editor {

class FileManager {
public:
    FileManager() = default;
    ~FileManager() = default;

    bool openInput(const std::string& filename);
    bool openOutput(const std::string& filename);
    void close();

    int readLines(std::vector<std::string>& lines, int maxLines = 80);
    std::string readLine();

    bool writeLine(const std::string& line);
    bool writeLines(const std::vector<std::string>& lines);

    bool isInputOpen() const { return input_.is_open(); }
    bool isOutputOpen() const { return output_.is_open(); }
    bool isInputEof() const { return input_.eof(); }

    const std::string& inputFilename() const { return inputFilename_; }
    const std::string& outputFilename() const { return outputFilename_; }

private:
    std::ifstream input_;
    std::ofstream output_;
    std::string inputFilename_;
    std::string outputFilename_;
};

} // namespace line_editor

#endif // FILE_MANAGER_H
