#ifndef LINE_BLOCK_H
#define LINE_BLOCK_H

#include <cstddef>
#include <cstring>

namespace line_editor {

constexpr size_t BLOCK_SIZE = 81;

class LineBlock {
public:
    LineBlock();
    explicit LineBlock(const char* data, size_t len);
    ~LineBlock();

    LineBlock(const LineBlock&) = delete;
    LineBlock& operator=(const LineBlock&) = delete;

    LineBlock(LineBlock&& other) noexcept;
    LineBlock& operator=(LineBlock&& other) noexcept;

    char* data() { return data_; }
    const char* data() const { return data_; }
    LineBlock* next() const { return next_; }
    size_t used() const { return used_; }
    bool isFull() const { return used_ >= BLOCK_SIZE - 1; }

    void setNext(LineBlock* next) { next_ = next; }

    bool append(char c);
    size_t append(const char* str, size_t len);
    void clear();

    LineBlock* createNext();

private:
    char data_[BLOCK_SIZE];
    size_t used_;
    LineBlock* next_;
};

} // namespace line_editor

#endif // LINE_BLOCK_H
