#include "test_framework.h"
#include "../include/line.h"

using namespace line_editor;

TEST(Line_CreateEmpty) {
    Line line;
    ASSERT_TRUE(line.isEmpty());
    ASSERT_EQ(line.length(), 0);
    return true;
}

TEST(Line_CreateWithText) {
    Line line("Hello, World!");
    ASSERT_FALSE(line.isEmpty());
    ASSERT_EQ(line.length(), 13);
    return true;
}

TEST(Line_LongText) {
    std::string longText;
    for (int i = 0; i < 200; i++) {
        longText += 'a' + (i % 26);
    }
    Line line(longText.c_str());
    ASSERT_EQ(line.length(), 200);
    return true;
}

REGISTER_TEST(Line, Line_CreateEmpty);
REGISTER_TEST(Line, Line_CreateWithText);
REGISTER_TEST(Line, Line_LongText);

int main() {
    return TestRegistry::instance().runAll();
}
