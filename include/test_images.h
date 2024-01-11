/**
 * There are several test images in the test_images folder this header 
 * will provide an interface to run though the test images and attempt
 * to decode them
*/

#ifndef TEST_IMAGES_HEADER
#define TEST_IMAGES_HEADER

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

std::vector<std::string> get_files_in_directory(const std::string& path);

#endif