#include "../include/line.h"
#include "test_framework.h"

using namespace line_editor;

// Test: Line creation
TEST(Line_CreateEmpty) {
    Line line;
    ASSERT_TRUE(line.isEmpty());
    ASSERT_EQ(line.length(), 0);
    ASSERT_NULL(line.head());
    return true;
}

// Test: Line with text
TEST(Line_CreateWithText) {
    Line line("Hello, World!");
    ASSERT_FALSE(line.isEmpty());
    ASSERT_EQ(line.length(), 13);
    ASSERT_STR_EQ(line.getText().c_str(), "Hello, World!");
    return true;
}

// Test: Line setText
TEST(Line_SetText) {
    Line line;
    ASSERT_TRUE(line.isEmpty());

    line.setText("New text");
    ASSERT_FALSE(line.isEmpty());
    ASSERT_STR_EQ(line.getText().c_str(), "New text");

    // Replace with new text
    line.setText("Different text");
    ASSERT_STR_EQ(line.getText().c_str(), "Different text");

    return true;
}

// Test: Line long text (multiple blocks)
TEST(Line_LongText) {
    // Create text longer than BLOCK_SIZE
    std::string longText;
    for (int i = 0; i < 200; i++) {
        longText += 'a' + (i % 26);
    }

    Line line(longText.c_str());
    ASSERT_EQ(line.length(), 200);
    ASSERT_STR_EQ(line.getText().c_str(), longText.c_str());

    return true;
}

// Test: Line find
TEST(Line_Find) {
    Line line("Hello, World! This is a test.");

    int pos = line.find("World");
    ASSERT_EQ(pos, 7);

    pos = line.find("test");
    ASSERT_TRUE(pos >= 0);

    pos = line.find("notfound");
    ASSERT_EQ(pos, -1);

    return true;
}

// Test: Line contains
TEST(Line_Contains) {
    Line line("Hello, World!");

    ASSERT_TRUE(line.contains("Hello"));
    ASSERT_TRUE(line.contains("World"));
    ASSERT_TRUE(line.contains("!"));
    ASSERT_FALSE(line.contains("Goodbye"));

    return true;
}

// Test: Line replace
TEST(Line_Replace) {
    Line line("Hello, World!");

    bool replaced = line.replace("World", "There");
    ASSERT_TRUE(replaced);
    ASSERT_STR_EQ(line.getText().c_str(), "Hello, There!");

    replaced = line.replace("xyz", "abc");
    ASSERT_FALSE(replaced);

    return true;
}

// Test: Line empty after clear
TEST(Line_SetEmpty) {
    Line line("Some text");
    ASSERT_FALSE(line.isEmpty());

    line.setText("");
    ASSERT_TRUE(line.isEmpty());

    return true;
}

// Test: Line with exactly BLOCK_SIZE-1 chars
TEST(Line_ExactlyFullBlock) {
    std::string text(80, 'A');  // Exactly 80 chars

    Line line(text.c_str());
    ASSERT_EQ(line.length(), 80);
    ASSERT_STR_EQ(line.getText().c_str(), text.c_str());

    return true;
}

// Test: Line with BLOCK_SIZE chars (requires 2 blocks)
TEST(Line_OneOverBlock) {
    std::string text(81, 'A');  // 81 chars - needs 2 blocks

    Line line(text.c_str());
    ASSERT_EQ(line.length(), 81);
    ASSERT_STR_EQ(line.getText().c_str(), text.c_str());

    return true;
}

// Register tests
REGISTER_TEST(Line, Line_CreateEmpty);
REGISTER_TEST(Line, Line_CreateWithText);
REGISTER_TEST(Line, Line_SetText);
REGISTER_TEST(Line, Line_LongText);
REGISTER_TEST(Line, Line_Find);
REGISTER_TEST(Line, Line_Contains);
REGISTER_TEST(Line, Line_Replace);
REGISTER_TEST(Line, Line_SetEmpty);
REGISTER_TEST(Line, Line_ExactlyFullBlock);
REGISTER_TEST(Line, Line_OneOverBlock);
