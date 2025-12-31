#ifndef EDITOR_H
#define EDITOR_H

#include "active_zone.h"
#include "file_manager.h"
#include "command_parser.h"
#include "command_executor.h"
#include <string>

namespace line_editor {

class Editor {
public:
    Editor();
    ~Editor() = default;

    bool init(const std::string& inputFile, const std::string& outputFile);

    void run();

    void showWelcome() const;
    void showHelp() const;

    void displayZone(int page = 0) const;

    bool isInitialized() const { return initialized_; }
    ActiveZone& zone() { return zone_; }
    const ActiveZone& zone() const { return zone_; }

private:
    ActiveZone zone_;
    FileManager fileMgr_;
    CommandParser parser_;
    CommandExecutor executor_;

    bool initialized_;
    std::string inputFile_;
    std::string outputFile_;

    bool processCommand(const std::string& input);

    void handleInsertMode(int lineNo);
};

} // namespace line_editor

#endif // EDITOR_H
