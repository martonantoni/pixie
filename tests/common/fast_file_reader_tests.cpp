#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

namespace FastFileReaderTests
{



const std::filesystem::path testFilePath_crlf = "test_fast_file_reader_crlf.txt";
const std::filesystem::path testFilePath_lf = "test_fast_file_reader_lf.txt";
bool testFilesCreated = false;
int linesWritten = 0;

std::pair<std::string, bool> lineText(int lineIndex) // returns text, isEOF
{
    // the first 10000 lines is "Line X", where X is the line number (1-based)
    if (lineIndex >= 1 && lineIndex <= 10000)
    {
        return { "Line " + std::to_string(lineIndex), false };
    }
    // the next 10000 lines is "Line X", but every second line is empty (still counting lines)
    else if (lineIndex >= 10001 && lineIndex <= 20000)
    {
        if (lineIndex % 2 == 0)
            return { "", false };
        else
            return { "Line " + std::to_string(lineIndex), false };
    }
    // the next 100'000 lines are each 31 characters long, starting with "Line X - " followed by the lower case alphabet, truncated so line length
    // is exactly 31 characters
    else if (lineIndex >= 20001 && lineIndex <= 120000)
    {
        std::string line = "Line " + std::to_string(lineIndex) + " - abcdefghijklmnopqrstuvwxyz";
        line = line.substr(0, 31); // truncate to 31 characters
        return { line, false };
    }
    // then comes 100000 empty lines
    else if (lineIndex >= 120001 && lineIndex <= 220000)
    {
        return { "", false };
    }
    // then 5000 lines with varying length, but at least 1000 characters
    // text is warping around "the quick brown fox jumps over the lazy dog "
    else if (lineIndex >= 220001 && lineIndex <= 225000)
    {
        std::string baseText = "the quick brown fox jumps over the lazy dog ";
        std::string line;
        int targetLength = 1000 + (lineIndex % 500); // length between 1000 and 1499
        while (static_cast<int>(line.size()) < targetLength)
        {
            line += baseText;
        }
        line = line.substr(0, targetLength); // truncate to target length
        return { line, false };
    }

    return { "", true }; // EOF

}

void createTestFile(const std::string& lineEndMarker, const std::filesystem::path& path)
{
    cFile out;
    out.Open(path, cFile::Open_Truncate | cFile::Open_Write);

    int lineIndex = 1;
    for (;;)
    {
        auto [line, isEOF] = lineText(lineIndex);
        if (isEOF)
            break;
        out.Write(line.c_str(), line.size());
        out.Write(lineEndMarker.c_str(), lineEndMarker.size());
        ++lineIndex;
    }
    linesWritten = lineIndex - 1;
    testFilesCreated = true;
}

void createTestFiles()
{
    if (testFilesCreated)
        return;
    createTestFile("\r\n"s, testFilePath_crlf);
    createTestFile("\n"s, testFilePath_lf);
}

void testReadLines(const std::filesystem::path& path)
{
    cFastFileReader reader(path);
    int lineNumber = 1;
    for (auto line : reader)
    {
        auto [expectedText, isEOF] = lineText(lineNumber);
        EXPECT_EQ(line, expectedText) << " at line " << lineNumber;
        ++lineNumber;
    }
    ASSERT_EQ(lineNumber - 1, linesWritten);
}

TEST(fast_file_reader, read_lines_crlf)
{
    createTestFiles();
    testReadLines(testFilePath_crlf);
}

TEST(fast_file_reader, read_lines_lf)
{
    createTestFiles();
    testReadLines(testFilePath_lf);
}


} // namespace FastFileReaderTests

