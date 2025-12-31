#ifndef LINE_H
#define LINE_H

#include "line_block.h"
#include <string>

namespace line_editor {

class Line {
public:
    Line();
    explicit Line(const char* text);
    ~Line();

    Line(const Line&) = delete;
    Line& operator=(const Line&) = delete;

    Line(Line&& other) noexcept;
    Line& operator=(Line&& other) noexcept;

    LineBlock* head() const { return head_; }
    Line* prev() const { return prev_; }
    Line* next() const { return next_; }

    void setPrev(Line* prev) { prev_ = prev; }
    void setNext(Line* next) { next_ = next; }

    void setText(const char* text);
    std::string getText() const;
    size_t length() const;
    bool isEmpty() const;

    int find(const char* substr) const;
    bool replace(const char* oldStr, const char* newStr);
    bool contains(const char* pattern) const;

private:
    LineBlock* head_;
    Line* prev_;
    Line* next_;

    void clearBlocks();
    size_t countBlocks() const;
    size_t countChars() const;
};

} // namespace line_editor

#endif // LINE_H
