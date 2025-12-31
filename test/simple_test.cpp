#include "test_framework.h"
#include "../include/line_block.h"

using namespace line_editor;

TEST(Simple_BlockCreate) {
    LineBlock block;
    ASSERT_EQ(block.used(), 0);
    return true;
}

REGISTER_TEST(Simple, Simple_BlockCreate);

int main() {
    return TestRegistry::instance().runAll();
}
