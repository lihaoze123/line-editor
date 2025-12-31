#include "line_block.h"
#include "error.h"
#include <algorithm>

namespace line_editor {

LineBlock::LineBlock() : used_(0), next_(nullptr) {
    data_[0] = '\0';
}

LineBlock::LineBlock(const char* data, size_t len) : next_(nullptr) {
    used_ = std::min(len, BLOCK_SIZE - 1);
    std::memcpy(data_, data, used_);
    data_[used_] = '\0';
}

LineBlock::~LineBlock() {
    LineBlock* current = next_;
    while (current) {
        LineBlock* next = current->next_;
        current->next_ = nullptr;
        delete current;
        current = next;
    }
}

LineBlock::LineBlock(LineBlock&& other) noexcept
    : used_(other.used_), next_(other.next_) {
    std::memcpy(data_, other.data_, BLOCK_SIZE);
    other.used_ = 0;
    other.next_ = nullptr;
    other.data_[0] = '\0';
}

LineBlock& LineBlock::operator=(LineBlock&& other) noexcept {
    if (this != &other) {
        LineBlock* otherNext = other.next_;
        other.next_ = nullptr;

        // Delete the entire chain, not just the first block
        // Use same recursive deletion pattern as destructor
        LineBlock* current = next_;
        while (current) {
            LineBlock* next = current->next_;
            current->next_ = nullptr;
            delete current;
            current = next;
        }

        used_ = other.used_;
        next_ = otherNext;
        std::memcpy(data_, other.data_, BLOCK_SIZE);

        other.used_ = 0;
        other.data_[0] = '\0';
    }
    return *this;
}

bool LineBlock::append(char c) {
    if (isFull()) {
        return false;
    }
    data_[used_++] = c;
    data_[used_] = '\0';
    return true;
}

size_t LineBlock::append(const char* str, size_t len) {
    size_t available = BLOCK_SIZE - 1 - used_;
    size_t toCopy = std::min(len, available);
    std::memcpy(data_ + used_, str, toCopy);
    used_ += toCopy;
    data_[used_] = '\0';
    return toCopy;
}

void LineBlock::clear() {
    used_ = 0;
    data_[0] = '\0';
}

LineBlock* LineBlock::createNext() {
    if (next_) {
        return next_;
    }
    next_ = new LineBlock();
    return next_;
}

} // namespace line_editor
