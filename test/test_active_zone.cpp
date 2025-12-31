#include "../include/active_zone.h"
#include "test_framework.h"

using namespace line_editor;

// Test: ActiveZone creation
TEST(ActiveZone_Create) {
    ActiveZone zone(100);

    ASSERT_TRUE(zone.isEmpty());
    ASSERT_EQ(zone.lineCount(), 0);
    ASSERT_EQ(zone.maxLines(), 100);
    ASSERT_NULL(zone.head());
    ASSERT_NULL(zone.tail());

    return true;
}

// Test: ActiveZone append line
TEST(ActiveZone_AppendLine) {
    ActiveZone zone;

    Line* line1 = new Line("First line");
    Line* line2 = new Line("Second line");

    zone.appendLine(line1);
    ASSERT_EQ(zone.lineCount(), 1);
    ASSERT_FALSE(zone.isEmpty());

    zone.appendLine(line2);
    ASSERT_EQ(zone.lineCount(), 2);

    return true;
}

// Test: ActiveZone insert
TEST(ActiveZone_Insert) {
    ActiveZone zone;

    // Insert some lines
    zone.insert(0, "Line 1");
    zone.insert(1, "Line 2");
    zone.insert(2, "Line 3");

    ASSERT_EQ(zone.lineCount(), 3);

    // Insert in middle
    zone.insert(2, "Line 2.5");
    ASSERT_EQ(zone.lineCount(), 4);

    Line* line = zone.getLine(2);
    ASSERT_NOT_NULL(line);
    ASSERT_STR_EQ(line->getText().c_str(), "Line 2.5");

    return true;
}

// Test: ActiveZone delete
TEST(ActiveZone_Delete) {
    ActiveZone zone;

    zone.insert(0, "Line 1");
    zone.insert(1, "Line 2");
    zone.insert(2, "Line 3");

    ASSERT_EQ(zone.lineCount(), 3);

    zone.deleteLine(2);  // Delete "Line 2"
    ASSERT_EQ(zone.lineCount(), 2);

    Line* line = zone.getLine(1);
    ASSERT_STR_EQ(line->getText().c_str(), "Line 3");

    return true;
}

// Test: ActiveZone delete range
TEST(ActiveZone_DeleteRange) {
    ActiveZone zone;

    for (int i = 1; i <= 5; i++) {
        std::string line = "Line " + std::to_string(i);
        zone.insert(i - 1, line.c_str());
    }

    ASSERT_EQ(zone.lineCount(), 5);

    zone.deleteRange(2, 4);  // Delete lines 2, 3, 4
    ASSERT_EQ(zone.lineCount(), 2);

    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(zone.getLine(1)->getText().c_str(), "Line 5");

    return true;
}

// Test: ActiveZone getLine
TEST(ActiveZone_GetLine) {
    ActiveZone zone;

    zone.insert(0, "Line 1");
    zone.insert(1, "Line 2");
    zone.insert(2, "Line 3");

    Line* line0 = zone.getLine(0);
    Line* line1 = zone.getLine(1);
    Line* line2 = zone.getLine(2);

    ASSERT_NOT_NULL(line0);
    ASSERT_NOT_NULL(line1);
    ASSERT_NOT_NULL(line2);

    ASSERT_STR_EQ(line0->getText().c_str(), "Line 1");
    ASSERT_STR_EQ(line1->getText().c_str(), "Line 2");
    ASSERT_STR_EQ(line2->getText().c_str(), "Line 3");

    // Out of range
    ASSERT_NULL(zone.getLine(10));

    return true;
}

// Test: ActiveZone findPattern
TEST(ActiveZone_FindPattern) {
    ActiveZone zone;

    zone.insert(0, "Hello World");
    zone.insert(1, "Goodbye World");
    zone.insert(2, "Hello Again");

    auto matches = zone.findPattern("Hello");
    ASSERT_EQ(matches.size(), 2);

    matches = zone.findPattern("World");
    ASSERT_EQ(matches.size(), 2);

    matches = zone.findPattern("NotFound");
    ASSERT_EQ(matches.size(), 0);

    return true;
}

// Test: ActiveZone replaceInLine
TEST(ActiveZone_ReplaceInLine) {
    ActiveZone zone;

    zone.insert(0, "Hello World");
    zone.insert(1, "Goodbye World");

    bool replaced = zone.replaceInLine(1, "World", "There");
    ASSERT_TRUE(replaced);

    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Hello There");

    replaced = zone.replaceInLine(2, "NotFound", "Found");
    ASSERT_FALSE(replaced);

    return true;
}

// Test: ActiveZone clear
TEST(ActiveZone_Clear) {
    ActiveZone zone;

    zone.insert(0, "Line 1");
    zone.insert(1, "Line 2");

    ASSERT_EQ(zone.lineCount(), 2);

    zone.clear();
    ASSERT_EQ(zone.lineCount(), 0);
    ASSERT_TRUE(zone.isEmpty());

    return true;
}

// Test: ActiveZone max lines
TEST(ActiveZone_MaxLines) {
    ActiveZone zone(5);  // Small zone for testing

    for (int i = 0; i < 10; i++) {
        std::string line = "Line " + std::to_string(i);
        zone.insert(i, line.c_str());
    }

    // Should not exceed max
    ASSERT_EQ(zone.lineCount(), 5);

    // First line should have been pushed out
    ASSERT_STR_EQ(zone.getLine(0)->getText().c_str(), "Line 5");

    return true;
}

// Register tests
REGISTER_TEST(ActiveZone, ActiveZone_Create);
REGISTER_TEST(ActiveZone, ActiveZone_AppendLine);
REGISTER_TEST(ActiveZone, ActiveZone_Insert);
REGISTER_TEST(ActiveZone, ActiveZone_Delete);
REGISTER_TEST(ActiveZone, ActiveZone_DeleteRange);
REGISTER_TEST(ActiveZone, ActiveZone_GetLine);
REGISTER_TEST(ActiveZone, ActiveZone_FindPattern);
REGISTER_TEST(ActiveZone, ActiveZone_ReplaceInLine);
REGISTER_TEST(ActiveZone, ActiveZone_Clear);
REGISTER_TEST(ActiveZone, ActiveZone_MaxLines);
