#ifndef PNG_HEADER
#define PNG_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

#include "PngByte.h"

struct Chunk {
    uint32_t length;
    unsigned char type[4];
    std::size_t chunk_data_start;
    uint32_t crc;
};

struct IHDR {
    uint32_t width;
    uint32_t height;
    unsigned char bit_depth;
    unsigned char color_type;
    unsigned char compression_method;
    unsigned char filter_method;
    unsigned char interlace_method;
};

class Png {
    /**
     * @details This vector is the only place that should host data from a file.
     * Design other methods and data structures around this vector of bytes
     * but try no to copy bytes to other structures.
    */
    std::vector<PngByte> data_;
    std::vector<Chunk> chunks_;
    IHDR header_;
    /**
     * Indexes into the chunks_ vector. The size of this vector indicates how many
     * IDAT chunks there are.
    */
    std::vector<int> IDAT_chunk_indexes;

    std::string file_path;

    bool parsing_success;

    /**
     * @brief copies file to data_
    */
    void load_data_from_file_path();
    bool validate_png_signature();
    /**
     * @brief IHDR chunk has to be present and first! Assumes that 
     * vanadate_png_signature() is true and populate_chunks() has been
     * called.
    */
    bool validate_IHDR();
    bool validate_IDAT();

    /** 
     * @brief assumes load_data_from_file_path() has been called.
    */
    void populate_chunks();
    /**
     * @brief assumes that validate_IHDR() is true.
    */
    void populate_header();

public:
    Png(const std::string& path_to_image);
    ~Png();
    void print_data_hex(int width = 16) const;
    uint32_t get_uint32_t_h(std::size_t index_into_data) const;


    Png() = delete;
    Png(const Png& other) = delete;
    Png(Png&& other) = delete;
    Png& operator=(const Png& other) = delete;
    Png& operator=(Png&& other) = delete;
};

#endif