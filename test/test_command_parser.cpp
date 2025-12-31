#include "../include/command_parser.h"
#include "test_framework.h"

using namespace line_editor;

// Test: Parse insert command
TEST(Parser_Insert) {
    CommandParser parser;
    Command cmd = parser.parse("i10");

    ASSERT_TRUE(cmd.type == CommandType::INSERT);
    ASSERT_EQ(cmd.lineNo, 10);

    return true;
}

// Test: Parse insert with text
TEST(Parser_InsertWithText) {
    CommandParser parser;
    Command cmd = parser.parse("i5 Hello World");

    ASSERT_TRUE(cmd.type == CommandType::INSERT);
    ASSERT_EQ(cmd.lineNo, 5);
    ASSERT_STR_EQ(cmd.text.c_str(), "Hello World");

    return true;
}

// Test: Parse delete single line
TEST(Parser_DeleteSingle) {
    CommandParser parser;
    Command cmd = parser.parse("d10");

    ASSERT_TRUE(cmd.type == CommandType::DELETE);
    ASSERT_EQ(cmd.lineNo, 10);
    ASSERT_EQ(cmd.lineNo2, 0);

    return true;
}

// Test: Parse delete range
TEST(Parser_DeleteRange) {
    CommandParser parser;
    Command cmd = parser.parse("d5 10");

    ASSERT_TRUE(cmd.type == CommandType::DELETE);
    ASSERT_EQ(cmd.lineNo, 5);
    ASSERT_EQ(cmd.lineNo2, 10);

    return true;
}

// Test: Parse next zone command
TEST(Parser_NextZone) {
    CommandParser parser;
    Command cmd = parser.parse("n");

    ASSERT_TRUE(cmd.type == CommandType::NEXT_ZONE);

    return true;
}

// Test: Parse print command
TEST(Parser_Print) {
    CommandParser parser;
    Command cmd = parser.parse("p");

    ASSERT_TRUE(cmd.type == CommandType::PRINT);

    return true;
}

// Test: Parse replace command
TEST(Parser_Replace) {
    CommandParser parser;
    Command cmd = parser.parse("s5@old@new");

    ASSERT_TRUE(cmd.type == CommandType::REPLACE);
    ASSERT_EQ(cmd.lineNo, 5);
    ASSERT_STR_EQ(cmd.oldStr.c_str(), "old");
    ASSERT_STR_EQ(cmd.newStr.c_str(), "new");

    return true;
}

// Test: Parse match command
TEST(Parser_Match) {
    CommandParser parser;
    Command cmd = parser.parse("mHello");

    ASSERT_TRUE(cmd.type == CommandType::MATCH);
    ASSERT_STR_EQ(cmd.pattern.c_str(), "Hello");

    return true;
}

// Test: Parse quit command
TEST(Parser_Quit) {
    CommandParser parser;
    Command cmd = parser.parse("q");

    ASSERT_TRUE(cmd.type == CommandType::QUIT);

    return true;
}

// Test: Parse unknown command
TEST(Parser_Unknown) {
    CommandParser parser;
    Command cmd = parser.parse("x");

    ASSERT_TRUE(cmd.type == CommandType::UNKNOWN);

    return true;
}

// Test: Case insensitive
TEST(Parser_CaseInsensitive) {
    CommandParser parser;

    ASSERT_TRUE(parser.parse("I10").type == CommandType::INSERT);
    ASSERT_TRUE(parser.parse("D10").type == CommandType::DELETE);
    ASSERT_TRUE(parser.parse("N").type == CommandType::NEXT_ZONE);
    ASSERT_TRUE(parser.parse("P").type == CommandType::PRINT);
    ASSERT_TRUE(parser.parse("Q").type == CommandType::QUIT);

    return true;
}

// Register tests
REGISTER_TEST(CommandParser, Parser_Insert);
REGISTER_TEST(CommandParser, Parser_InsertWithText);
REGISTER_TEST(CommandParser, Parser_DeleteSingle);
REGISTER_TEST(CommandParser, Parser_DeleteRange);
REGISTER_TEST(CommandParser, Parser_NextZone);
REGISTER_TEST(CommandParser, Parser_Print);
REGISTER_TEST(CommandParser, Parser_Replace);
REGISTER_TEST(CommandParser, Parser_Match);
REGISTER_TEST(CommandParser, Parser_Quit);
REGISTER_TEST(CommandParser, Parser_Unknown);
REGISTER_TEST(CommandParser, Parser_CaseInsensitive);
