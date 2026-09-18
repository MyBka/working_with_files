#include <gtest/gtest.h>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <format>


#ifdef _WIN32
    #define EXECUTABLE_EXTENSION ".exe"
#else
    #define EXECUTABLE_EXTENSION ""
#endif

namespace fs = std::filesystem;

struct TestCase {
    std::string test_name;
    std::string test_file;
    std::string flags;
    std::string answer;

    TestCase(const std::string& TestName, const std::string& TestFile, const std::string& Flags, const std::string& ExpectedAnswer) :
        test_name(TestName), test_file(TestFile), flags(Flags), answer(ExpectedAnswer) {}
};

std::ostream& operator<<(std::ostream& os, const TestCase& test_case) {
    return os << std::format (
        "TestCase(test_name: {}, test_file: {}, flags: {}, answer: {})",
        test_case.test_name, test_case.test_file, test_case.flags, test_case.answer
    );
}

class WordCountTest : public ::testing::TestWithParam<TestCase> {
protected:
    void TearDown() override {
        std::remove("temp_output.txt");
    }

    std::string GetActualOutput(const TestCase& test) {
        std::string cmd = (fs::path("..") / ("WordCount" + std::string(EXECUTABLE_EXTENSION))).string();
        cmd += " " + test.flags + " " + test.test_file + " > temp_output.txt";
        
        std::system(cmd.c_str());

        std::ifstream res_file("temp_output.txt");
        std::string line;
        std::getline(res_file, line);
        return line;
    }
};


std::vector<TestCase> DiscoverTests() {
    std::vector<TestCase> cases;
    for (const auto& entry : fs::directory_iterator("test_cases")) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();

            std::ifstream exp_file(entry.path() / "expected.txt");
            std::string expected;
            std::getline(exp_file, expected);

            std::string flags;
            
            if (fs::exists(entry.path() / "flags.txt")) {
                std::ifstream flags_file(entry.path() / "flags.txt");
                std::getline(flags_file, flags);
            }
            
            cases.emplace_back(
                name, 
                (fs::path("test_cases") / name / "input.txt").string(), 
                flags, 
                expected
            );
        }
    }

    std::sort(cases.begin(), cases.end(), [](const TestCase& a, const TestCase& b) {
        return a.test_name < b.test_name;
    });

    return cases;
}


TEST_P(WordCountTest, MatchesExpected) {
    const auto& tc = GetParam();
    std::string actual = GetActualOutput(tc);
    
    // Удаляем возможные пробелы в конце, если они есть
    actual.erase(actual.find_last_not_of(" \n\r\t") + 1);
    
    EXPECT_EQ(actual, tc.answer);
}

INSTANTIATE_TEST_SUITE_P(
    FullSuite,
    WordCountTest,
    ::testing::ValuesIn(DiscoverTests())
);