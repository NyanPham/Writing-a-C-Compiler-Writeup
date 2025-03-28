// filepath: /d:/Self-study/C-Compiler/Sandler-Nora-Writing-a-C-Compiler--writeup/code/tests/TestFramework.h
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

class TestFramework
{
public:
    using TestFunc = std::function<void()>;

    struct Test
    {
        std::string name;
        std::string chapter;
        std::string stage;
        TestFunc func;
    };

    static void addTest(const std::string &name, const std::string &chapter, const std::string &stage, TestFunc func)
    {
        getTests().emplace_back(Test{name, chapter, stage, func});
    }

    static void runTests(const std::string &chapterFilter = "", const std::string &stageFilter = "")
    {
        int passed = 0;
        int failed = 0;
        std::cout << "Running tests with filters - Chapter: " << chapterFilter << ", Stage: " << stageFilter << '\n';
        for (const auto &test : getTests())
        {
            if ((!chapterFilter.empty() && test.chapter != chapterFilter) ||
                (!stageFilter.empty() && test.stage != stageFilter))
            {
                continue;
            }
            try
            {
                test.func();
                std::cout << "\033[32m[PASSED]\033[0m " << test.name << '\n'; // Green color for passed tests
                ++passed;
            }
            catch (const std::exception &e)
            {
                std::cout << "\033[31m[FAILED]\033[0m " << test.name << ": " << e.what() << '\n'; // Red color for failed tests
                ++failed;
            }
            catch (...)
            {
                std::cout << "\033[31m[FAILED]\033 " << test.name << ": Unknown exception\n"; // Red color for failed tests 
                ++failed;
            }   
        }
        std::cout << "\nTotal: " << passed + failed << ", \033[32mPassed: " << passed << "\033[0m, \033[31mFailed: " << failed << "\033[0m\n";
    }

private:
    static std::vector<Test> &getTests()
    {
        static std::vector<Test> tests;
        return tests;
    }
};

#define TEST_CASE(name, chapter, stage)                                            \
    void name();                                                                   \
    struct name##_Register                                                         \
    {                                                                              \
        name##_Register() { TestFramework::addTest(#name, chapter, stage, name); } \
    };                                                                             \
    static name##_Register name##_register;                                        \
    void name()

#define ASSERT_TRUE(condition) \
    if (!(condition))          \
    throw std::runtime_error("Assertion failed: " #condition " at " __FILE__ ":" + std::to_string(__LINE__))

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual))     \
    throw std::runtime_error("Assertion failed: " #expected " == " #actual " at " __FILE__ ":" + std::to_string(__LINE__))

#endif // TEST_FRAMEWORK_H