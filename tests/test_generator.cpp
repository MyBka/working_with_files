#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
    #define EXECUTABLE_EXTENSION ".exe"
#else
    #define EXECUTABLE_EXTENSION ""
    #include <sys/wait.h>
#endif

namespace fs = std::filesystem;


namespace {

std::string Sanitize(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (const char c : value) {
        result += std::isalnum(static_cast<unsigned char>(c)) ? c : '_';
    }

    return result;
}

std::string TrimRight(std::string value) {
    const std::size_t pos = value.find_last_not_of(" \n\r\t");
    value.erase(pos + 1);
    return value;
}

std::string ReadFirstLine(const fs::path& path) {
    std::ifstream file(path);
    std::string line;
    std::getline(file, line);
    return TrimRight(line);
}

std::string ReadWholeFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> SplitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(TrimRight(line));
    }

    while (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }

    return lines;
}

std::string SubstituteDir(std::string value, const std::string& dir) {
    const std::string placeholder = "{dir}";
    std::size_t pos = 0;

    while ((pos = value.find(placeholder, pos)) != std::string::npos) {
        value.replace(pos, placeholder.size(), dir);
        pos += dir.size();
    }

    return value;
}

std::string Quoted(const std::string& value) {
    return "\"" + value + "\"";
}

int DecodeExitCode(int status) {
#ifdef _WIN32
    return status;
#else
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return -WTERMSIG(status);
    }

    return status;
#endif
}

}  // namespace

struct TestCase {
    std::string dir_name;
    std::string display_name;
    std::string args;
    std::vector<std::string> expected;
    std::string error;

    TestCase(std::string DirName, std::string DisplayName, std::string Args,
             std::vector<std::string> Expected, std::string Error)
        : dir_name(std::move(DirName)),
          display_name(std::move(DisplayName)),
          args(std::move(Args)),
          expected(std::move(Expected)),
          error(std::move(Error)) {}

    std::string GtestName() const {
        if (display_name.empty()) {
            return Sanitize(dir_name);
        }

        return Sanitize(dir_name) + "_" + display_name;
    }
};

std::ostream& operator<<(std::ostream& os, const TestCase& test_case) {
    return os << std::format("TestCase(name: {}, args: {}, expected_lines: {})",
                             test_case.GtestName(), test_case.args, test_case.expected.size());
}

class WordCountTest : public ::testing::TestWithParam<TestCase> {
protected:
    void TearDown() override {
        if (!temp_output_.empty()) {
            std::remove(temp_output_.c_str());
        }
    }

    std::vector<std::string> GetActualOutput(const TestCase& test, int& exit_code) {
        temp_output_ = "temp_output_" + Sanitize(test.dir_name) + ".txt";

        const std::string executable =
            (fs::path("..") / ("WordCount" + std::string(EXECUTABLE_EXTENSION))).string();

        const std::string cmd =
            Quoted(executable) + " " + test.args + " > " + Quoted(temp_output_);

        exit_code = DecodeExitCode(std::system(cmd.c_str()));

        return SplitLines(ReadWholeFile(temp_output_));
    }

    static std::string BuildContext(const TestCase& test, int exit_code) {
        return "Command: WordCount " + test.args + "\nExit code: " + std::to_string(exit_code);
    }

private:
    std::string temp_output_;
};

std::vector<TestCase> DiscoverTests() {
    std::vector<TestCase> cases;

    if (!fs::exists("test_cases")) {
        return cases;
    }

    for (const auto& entry : fs::directory_iterator("test_cases")) {
        if (!entry.is_directory()) {
            continue;
        }

        const std::string dir_name = entry.path().filename().string();

        if (dir_name.empty() || dir_name.front() == '.') {
            continue;
        }

        const std::string case_dir = (fs::path("test_cases") / dir_name).generic_string();

        std::string display_name;
        if (fs::exists(entry.path() / "name.txt")) {
            display_name = Sanitize(ReadFirstLine(entry.path() / "name.txt"));
        }

        std::string args;
        if (fs::exists(entry.path() / "args.txt")) {
            args = SubstituteDir(ReadFirstLine(entry.path() / "args.txt"), case_dir);
        } else {
            std::string flags;
            if (fs::exists(entry.path() / "flags.txt")) {
                flags = ReadFirstLine(entry.path() / "flags.txt");
            }

            const std::string input = case_dir + "/input.txt";
            args = flags.empty() ? input : flags + " " + input;
        }

        std::vector<std::string> expected;
        for (const std::string& line : SplitLines(ReadWholeFile(entry.path() / "expected.txt"))) {
            expected.push_back(SubstituteDir(line, case_dir));
        }

        std::string error;
        if (!fs::exists(entry.path() / "expected.txt")) {
            error = "Test case " + case_dir + " has no expected.txt";
        } else if (expected.empty()) {
            error = "File expected.txt in test case " + case_dir + " is empty";
        }

        cases.emplace_back(dir_name, display_name, args, std::move(expected), std::move(error));
    }

    std::sort(cases.begin(), cases.end(),
              [](const TestCase& a, const TestCase& b) { return a.dir_name < b.dir_name; });

    return cases;
}

TEST_P(WordCountTest, MatchesExpected) {
    const TestCase& tc = GetParam();

    ASSERT_TRUE(tc.error.empty()) << tc.error;

    int exit_code = 0;
    const std::vector<std::string> actual = GetActualOutput(tc, exit_code);
    const std::string context = BuildContext(tc, exit_code);

    ASSERT_EQ(actual.size(), tc.expected.size())
        << context << "\nExpected lines: " << tc.expected.size()
        << ", got: " << actual.size();

    for (std::size_t i = 0; i < tc.expected.size(); ++i) {
        EXPECT_EQ(actual[i], tc.expected[i]) << context << "\nMismatch at line " << i + 1;
    }
}

INSTANTIATE_TEST_SUITE_P(FullSuite, WordCountTest, ::testing::ValuesIn(DiscoverTests()),
                         [](const ::testing::TestParamInfo<TestCase>& info) {
                             return info.param.GtestName();
                         });

TEST(WordCountSuite, TestCasesDiscovered) {
    ASSERT_TRUE(fs::exists("test_cases"))
        << "Directory test_cases not found in " << fs::current_path().string();
    EXPECT_FALSE(DiscoverTests().empty())
        << "Directory test_cases contains no test case directories";
}