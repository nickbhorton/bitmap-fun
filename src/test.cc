#include <iostream>

#include "test_images.h"
#include "Png.h"

int main() {
    std::vector<std::string> test_pngs = get_files_in_directory("test_images");
    Png test_png{test_pngs[0]};
    test_png.print_data_hex(10);
}