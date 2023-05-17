#include "CompareFiles.h"

#include <fstream>
#include <streambuf>
#include <iterator>

namespace util {

// https://stackoverflow.com/a/15118786
bool FilesAreEqual(const std::string& filename1, const std::string& filename2) {
    std::ifstream stream{filename1};
    std::string file1{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};

    stream = std::ifstream{filename2};
    std::string file2{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};

    return file1 == file2;
}

}  // namespace util
