#ifndef PNG_HEADER
#define PNG_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "PngByte.h"

class Png {
    /**
     * @details This vector is the only place that should host data from a file.
     * Design other methods and data structures around this vector of bytes
     * but try no to copy bytes to other structures.
    */
    std::vector<PngByte> data_;
    std::string file_path;

    /**
     * @brief copies file to data_
    */
    void load_data_from_file_path();

public:
    Png(const std::string& path_to_image);
    ~Png();
    void print_data_hex(int width) const;

    Png() = delete;
    Png(const Png& other) = delete;
    Png(Png&& other) = delete;
    Png& operator=(const Png& other) = delete;
    Png& operator=(Png&& other) = delete;
};

#endif