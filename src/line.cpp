#include "line.h"
#include <algorithm>
#include <cstring>

namespace line_editor {

Line::Line() : head_(nullptr), prev_(nullptr), next_(nullptr) {
}

Line::Line(const char* text) : head_(nullptr), prev_(nullptr), next_(nullptr) {
    setText(text);
}

Line::~Line() {
    clearBlocks();
}

Line::Line(Line&& other) noexcept
    : head_(other.head_), prev_(other.prev_), next_(other.next_) {
    other.head_ = nullptr;
    other.prev_ = nullptr;
    other.next_ = nullptr;
}

Line& Line::operator=(Line&& other) noexcept {
    if (this != &other) {
        clearBlocks();

        head_ = other.head_;
        prev_ = other.prev_;
        next_ = other.next_;

        other.head_ = nullptr;
        other.prev_ = nullptr;
        other.next_ = nullptr;
    }
    return *this;
}

void Line::setText(const char* text) {
    clearBlocks();

    if (!text || text[0] == '\0') {
        return;
    }

    head_ = new LineBlock();
    LineBlock* current = head_;

    size_t textLen = std::strlen(text);
    size_t offset = 0;

    while (offset < textLen) {
        size_t written = current->append(text + offset, textLen - offset);
        offset += written;

        if (offset < textLen) {
            current = current->createNext();
        }
    }
}

std::string Line::getText() const {
    std::string result;
    LineBlock* current = head_;

    while (current) {
        result.append(current->data());
        current = current->next();
    }

    return result;
}

size_t Line::length() const {
    size_t len = 0;
    LineBlock* current = head_;

    while (current) {
        len += current->used();
        current = current->next();
    }

    return len;
}

bool Line::isEmpty() const {
    return head_ == nullptr || head_->used() == 0;
}

void Line::clearBlocks() {
    delete head_;
    head_ = nullptr;
}

size_t Line::countBlocks() const {
    size_t count = 0;
    LineBlock* current = head_;

    while (current) {
        count++;
        current = current->next();
    }

    return count;
}

size_t Line::countChars() const {
    return length();
}

int Line::find(const char* substr) const {
    if (!substr || substr[0] == '\0') {
        return 0;
    }

    std::string text = getText();
    size_t pos = text.find(substr);

    return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

bool Line::contains(const char* pattern) const {
    return find(pattern) != -1;
}

bool Line::replace(const char* oldStr, const char* newStr) {
    if (!oldStr || oldStr[0] == '\0') {
        return false;
    }

    std::string text = getText();
    size_t pos = text.find(oldStr);

    if (pos == std::string::npos) {
        return false;
    }

    text.replace(pos, std::strlen(oldStr), newStr ? newStr : "");
    setText(text.c_str());
    return true;
}

} // namespace line_editor
