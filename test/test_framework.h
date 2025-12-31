#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>

#define ASSERT_TRUE(x) do { \
    if (!(x)) { \
        std::cerr << "  失败: " << #x << " (第 " << __LINE__ << " 行)\n"; \
        return false; \
    } \
} while(0)

#define ASSERT_FALSE(x) ASSERT_TRUE(!(x))

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "  失败: " << #a << " == " << #b << " (第 " << __LINE__ << " 行)\n"; \
        std::cerr << "    期望: " << (b) << "\n"; \
        std::cerr << "    实际: " << (a) << "\n"; \
        return false; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    if ((a) == (b)) { \
        std::cerr << "  失败: " << #a << " != " << #b << " (第 " << __LINE__ << " 行)\n"; \
        return false; \
    } \
} while(0)

#define ASSERT_NULL(x) ASSERT_EQ((x), nullptr)
#define ASSERT_NOT_NULL(x) ASSERT_NE((x), nullptr)

#define ASSERT_STR_EQ(a, b) do { \
    if (std::string(a) != std::string(b)) { \
        std::cerr << "  失败: 字符串不匹配 (第 " << __LINE__ << " 行)\n"; \
        std::cerr << "    期望: \"" << (b) << "\"\n"; \
        std::cerr << "    实际: \"" << (a) << "\"\n"; \
        return false; \
    } \
} while(0)

typedef bool (*TestFunc)();

struct TestCase {
    const char* suite;
    const char* name;
    TestFunc func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void addTest(const char* suite, const char* name, TestFunc func) {
        tests_.push_back({suite, name, func});
    }

    int runAll() {
        int passed = 0;
        int failed = 0;

        std::cout << "\n========================================\n";
        std::cout << "正在运行测试\n";
        std::cout << "========================================\n\n";

        for (const auto& test : tests_) {
            std::cout << "[" << test.suite << "] " << test.name << "... ";
            if (test.func()) {
                std::cout << "通过\n";
                passed++;
            } else {
                std::cout << "失败\n";
                failed++;
            }
        }

        std::cout << "\n========================================\n";
        std::cout << "结果: " << passed << " 个通过, " << failed << " 个失败\n";
        std::cout << "========================================\n\n";

        return failed;
    }

private:
    std::vector<TestCase> tests_;
};

struct TestRegister {
    TestRegister(const char* suite, const char* name, TestFunc func) {
        TestRegistry::instance().addTest(suite, name, func);
    }
};

#define TEST(name) bool test_##name()

#define REGISTER_TEST(suite, name) \
    static TestRegister TEST_REG_##suite##_##name(#suite, #name, test_##name);

#endif // TEST_FRAMEWORK_H
