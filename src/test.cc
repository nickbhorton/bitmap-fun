#include <iostream>

#include "test_images.h"
#include "Png.h"

int main() {
    std::vector<std::string> test_pngs = get_files_in_directory("test_images");
    // for (auto const& e : test_pngs) {
    //     Png test_png{e};
    // }
    Png test_png{test_pngs[0]};

}