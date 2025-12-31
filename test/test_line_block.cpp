#include "../include/line_block.h"
#include "test_framework.h"
#include <cstring>

using namespace line_editor;

// Test: LineBlock creation and initialization
TEST(LineBlock_CreateEmpty) {
    LineBlock block;
    ASSERT_EQ(block.used(), 0);
    ASSERT_TRUE(block.data()[0] == '\0');
    ASSERT_TRUE(block.next() == nullptr);
    return true;
}

// Test: LineBlock with data
TEST(LineBlock_CreateWithData) {
    const char* data = "Hello, World!";
    LineBlock block(data, strlen(data));

    ASSERT_EQ(block.used(), strlen(data));
    ASSERT_STR_EQ(block.data(), data);
    return true;
}

// Test: LineBlock append single char
TEST(LineBlock_AppendChar) {
    LineBlock block;

    for (int i = 0; i < 80; i++) {
        ASSERT_TRUE(block.append('a' + (i % 26)));
    }

    ASSERT_EQ(block.used(), 80);
    ASSERT_TRUE(block.isFull());

    // Should fail when full
    ASSERT_FALSE(block.append('x'));

    return true;
}

// Test: LineBlock append string
TEST(LineBlock_AppendString) {
    LineBlock block;
    const char* str = "Hello";

    size_t written = block.append(str, strlen(str));
    ASSERT_EQ(written, strlen(str));
    ASSERT_EQ(block.used(), strlen(str));
    ASSERT_STR_EQ(block.data(), str);

    return true;
}

// Test: LineBlock append multiple blocks
TEST(LineBlock_AppendMultiple) {
    LineBlock block;

    // Fill exactly to capacity
    for (int i = 0; i < 80; i++) {
        block.append('a' + (i % 26));
    }

    ASSERT_EQ(block.used(), 80);
    ASSERT_TRUE(block.isFull());

    // Create next block
    LineBlock* next = block.createNext();
    ASSERT_NOT_NULL(next);
    ASSERT_EQ(block.next(), next);

    // Next block should be empty
    ASSERT_EQ(next->used(), 0);

    return true;
}

// Test: LineBlock clear
TEST(LineBlock_Clear) {
    LineBlock block("Test data", 9);

    block.clear();

    ASSERT_EQ(block.used(), 0);
    ASSERT_TRUE(block.data()[0] == '\0');

    return true;
}

// Register tests
REGISTER_TEST(LineBlock, LineBlock_CreateEmpty);
REGISTER_TEST(LineBlock, LineBlock_CreateWithData);
REGISTER_TEST(LineBlock, LineBlock_AppendChar);
REGISTER_TEST(LineBlock, LineBlock_AppendString);
REGISTER_TEST(LineBlock, LineBlock_AppendMultiple);
REGISTER_TEST(LineBlock, LineBlock_Clear);
