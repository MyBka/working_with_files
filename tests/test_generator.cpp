#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef _WIN32
#include <stdio.h>
#endif

namespace fs = std::filesystem;

namespace {

std::string ReadFile(const fs::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open " + path.string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string ReplaceAll(const std::string& value, const std::string& from, const std::string& to) {
    std::string result = value;
    std::size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.size(), to);
        pos += to.size();
    }
    return result;
}

} // namespace

namespace exec {

#ifdef _WIN32

#define popen _popen
#define pclose _pclose

// Yes, really: https://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments
std::string ShellQuote(const std::string& value) {
    std::string result = "\"";
    std::size_t backslashes = 0;

    for (const char c : value) {
        if (c == '\\') {
            ++backslashes;
        } else if (c == '"') {
            result.append(backslashes * 2 + 1, '\\');
            result += c;
            backslashes = 0;
        } else {
            result.append(backslashes, '\\');
            result += c;
            backslashes = 0;
        }
    }

    result.append(backslashes * 2, '\\');
    return result + '"';
}

#else

std::string ShellQuote(const std::string& value) {
    return "'" + ReplaceAll(value, "'", "'\\''") + "'";
}

#endif

std::string BuildCommand(const std::vector<std::string>& args) {
    std::string command = ShellQuote(WORD_COUNT_EXECUTABLE);
    for (const std::string& arg : args) {
        command += " " + ShellQuote(arg);
    }
#ifdef _WIN32
    // Yes, really (x2)
    return '"' + command + '"';
#else
    return command;
#endif
}

std::string Run(const std::vector<std::string>& args, int& status) {
    const std::string command = BuildCommand(args);
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        status = -1;
        return {};
    }

    std::string output;
    char buffer[4096];
    while (const std::size_t size = std::fread(buffer, 1, sizeof(buffer), pipe)) {
        output.append(buffer, size);
    }

    status = pclose(pipe);
    return output;
}

}  // namespace exec

namespace {

struct TestCase {
    std::string name;
    std::vector<std::string> args;
    std::string expected;
};

std::ostream& operator<<(std::ostream& output, const TestCase& test_case) {
    return output << test_case.name;
}

std::vector<std::string> ReadArguments(const fs::path& path, const std::string& case_dir) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open " + path.string());
    }

    std::vector<std::string> args;
    std::string arg;
    while (std::getline(file, arg)) {
        if (!arg.empty() && arg.back() == '\r') {
            arg.pop_back();
        }
        if (!arg.empty()) {
            args.push_back(ReplaceAll(arg, "{dir}", case_dir));
        }
    }
    return args;
}

std::vector<TestCase> DiscoverTests() {
    const fs::path testdata_dir = TEST_DATA_DIR;
    if (!fs::is_directory(testdata_dir)) {
        throw std::runtime_error("Test data directory not found: " + testdata_dir.string());
    }

    std::vector<TestCase> cases;
    for (const fs::directory_entry& entry : fs::directory_iterator(testdata_dir)) {
        if (!entry.is_directory()) {
            continue;
        }

        const std::string case_dir = entry.path().string();
        const std::string expected = ReplaceAll(ReadFile(entry.path() / "expected.txt"), "{dir}", case_dir);

        cases.push_back({
            entry.path().filename().string(),
            ReadArguments(entry.path() / "args.txt", case_dir),
            expected,
        });
    }

    if (cases.empty()) {
        throw std::runtime_error("No test cases found in " + testdata_dir.string());
    }

    std::sort(cases.begin(), cases.end(), [](const TestCase& left, const TestCase& right) {
        return left.name < right.name;
    });

    return cases;
}

}  // namespace

class WordCountTest : public ::testing::TestWithParam<TestCase> {};

TEST_P(WordCountTest, MatchesExpected) {
    const TestCase& test_case = GetParam();
    int status = 0;
    const std::string actual = exec::Run(test_case.args, status);

    EXPECT_EQ(status, 0) << "Command: " << exec::BuildCommand(test_case.args);
    EXPECT_EQ(actual, test_case.expected);
}

INSTANTIATE_TEST_SUITE_P(
    FullSuite,
    WordCountTest,
    ::testing::ValuesIn(DiscoverTests()),
    [](const ::testing::TestParamInfo<TestCase>& info) { return info.param.name; });
