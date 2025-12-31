#ifndef ACTIVE_ZONE_H
#define ACTIVE_ZONE_H

#include "line.h"
#include <cstddef>
#include <vector>
#include <string>

namespace line_editor {

constexpr int DEFAULT_MAX_LINES = 100;
constexpr int PAGE_SIZE = 20;

class ActiveZone {
public:
    explicit ActiveZone(int maxLines = DEFAULT_MAX_LINES);
    ~ActiveZone();

    ActiveZone(const ActiveZone&) = delete;
    ActiveZone& operator=(const ActiveZone&) = delete;

    Line* head() const { return head_; }
    Line* tail() const { return tail_; }
    int startLineNo() const { return startLineNo_; }
    int lineCount() const { return lineCount_; }
    int maxLines() const { return maxLines_; }
    bool isEmpty() const { return lineCount_ == 0; }
    bool isFull() const { return lineCount_ >= maxLines_; }

    Line* getLine(int relativeIndex);
    Line* getLineByNumber(int lineNo);
    int getRelativeIndex(int lineNo) const;

    void insert(int afterLineNo, const char* text);

    void deleteLine(int lineNo);
    void deleteRange(int startLineNo, int endLineNo);

    bool replaceInLine(int lineNo, const char* oldStr, const char* newStr);
    std::vector<int> findPattern(const char* pattern) const;

    std::string display(int page = 0) const;
    int totalPages() const;

    void clear();
    void appendLine(Line* line);
    Line* removeFirst();
    Line* removeLast();

    void setStartLineNo(int lineNo) { startLineNo_ = lineNo; }

private:
    Line* head_;
    Line* tail_;
    int startLineNo_;
    int lineCount_;
    int maxLines_;

    void insertAfter(Line* position, Line* newLine);
    void removeLine(Line* line);
    Line* findLine(int lineNo) const;
};

} // namespace line_editor

#endif // ACTIVE_ZONE_H
