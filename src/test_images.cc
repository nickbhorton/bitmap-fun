#include "test_images.h"

std::vector<std::string> get_files_in_directory(const std::string& path) {
    std::vector<std::string> test_files{};
    for (const auto& file : std::filesystem::directory_iterator(path)) {
        test_files.push_back(file.path().string());
    }
    return test_files;
}