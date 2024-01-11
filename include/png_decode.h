#ifndef PNG_DECODE_HEADER
#define PNG_DECODE_HEADER

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <bitset>
#include <cmath>

#include "deflate.h"

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

class Image {
    std::vector<unsigned char> data_;
    std::vector<Color> pixel_array;
    int width_;
    int height_;
    unsigned char bit_depth_;
    unsigned char color_type_;
    unsigned char compression_method_;
    unsigned char filter_method_;
    unsigned char interlace_method_;

    bool constexpr has_alpha_channel() {
        return color_type_ & 0b00000100;
    }

    void get_data_from_file(const std::string& filename) {
        std::ifstream file;
        file.open(filename, std::ios::binary | std::ios::out);
        if (!file.good()){
            std::cout << "file path was bad\n";
            std::exit(EXIT_FAILURE);
        }
        while (!file.eof()){
            char c{0};
            file.read(&c, 1);
            const unsigned char current_byte = static_cast<unsigned char>(c);
            data_.push_back(current_byte);
        }
        file.close();
    }

    bool contains_png_header() {
        return (data_.at(0) == 0x89 &&
                data_.at(1) == 0x50 &&
                data_.at(2) == 0x4e &&
                data_.at(3) == 0x47 &&
                data_.at(4) == 0x0d &&
                data_.at(5) == 0x0a &&
                data_.at(6) == 0x1a &&
                data_.at(7) == 0x0a
        );
    }

    int32_t get_i32_big_endian(int buffer_offset){
        union intc32
        {
            unsigned char c[4];
            int32_t v;
        };
        
        const intc32 result = {{
            data_.at(buffer_offset + 3), 
            data_.at(buffer_offset + 2), 
            data_.at(buffer_offset + 1), 
            data_.at(buffer_offset + 0)
        }};
        return result.v;
    }
    
    public:
    Image(const std::string& filename) {
        get_data_from_file(filename);
        if (!contains_png_header()){
            std::cout << "file not a png\n";
            std::exit(EXIT_FAILURE);
        }
        constexpr int size_of_png_header = 8;
        constexpr int size_of_length_field = 4;
        constexpr int size_of_ascii_chunk_name = 4;
        constexpr int size_of_check_sum = 4;
        bool found_IHDR_chunk = false;
        int buffer_offset = size_of_png_header;
        while (buffer_offset < data_.size() - size_of_length_field) {
            int chunk_size = get_i32_big_endian(buffer_offset);
            buffer_offset += size_of_length_field;
            if (data_.at(buffer_offset + 0) == 'I' &&
                data_.at(buffer_offset + 1) == 'H' &&
                data_.at(buffer_offset + 2) == 'D' &&
                data_.at(buffer_offset + 3) == 'R') 
            {
                width_ = get_i32_big_endian(buffer_offset + 4);
                height_ = get_i32_big_endian(buffer_offset + 8);
                bit_depth_ = data_.at(buffer_offset + 12);
                color_type_ = data_.at(buffer_offset + 13);
                compression_method_ = data_.at(buffer_offset + 14);
                filter_method_ = data_.at(buffer_offset + 15);
                interlace_method_ = data_.at(buffer_offset + 16);
                found_IHDR_chunk = true;
                std::cout << "width: " << width_ << "\n";
                std::cout << "height: " << height_ << "\n";
                // std::cout << "bit_depth: " << (int) bit_depth_ << "\n";
                std::cout << "color_type: " << (int) color_type_ << "\n";
                // std::cout << "compression_method: "  << (int) compression_method_ << "\n";
                // std::cout << "filter_method: " << (int) filter_method_ << "\n";
                // std::cout << "interlace_method: " << (int) interlace_method_ << "\n";
                if (color_type_ & 0b00000001) {
                    std::cout << "Pallet bit was set. Pallet format is not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
                if (!color_type_ & 0b00000010) {
                    std::cout << "Color bit was not set. Grey scale images are not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
                if (bit_depth_ != 8){
                    std::cout << "Bit depth was not 8. Other bit depths are not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
                if (compression_method_) {
                    std::cout << "A compression method was specified, compression is not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
                if (filter_method_) {
                    std::cout << "A filter method was specified, filtering is not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
                if (interlace_method_) {
                    std::cout << "A interlace method was specified, interlacing is not currently supported.\n";
                    std::exit(EXIT_FAILURE);
                }
            }
            else if (data_.at(buffer_offset + 0) == 'I' &&
                data_.at(buffer_offset + 1) == 'D' &&
                data_.at(buffer_offset + 2) == 'A' &&
                data_.at(buffer_offset + 3) == 'T') 
            {
                if (!found_IHDR_chunk) {
                    std::cout << "Error, IDAT chunk found before IHDR chunk\n";
                    std::exit(EXIT_FAILURE);
                }
                std::cout << "IDAT:\n"; 
                const std::bitset<8> compression_method_and_flag_byte = data_.at(buffer_offset + 4);
                if (!compression_method_and_flag_byte[3]){
                    std::cout << "Error. bits 3 of 'Compression method' is not set. This bit indicates the 'deflate' compression method.";
                    std::exit(EXIT_FAILURE);
                }
                if (compression_method_and_flag_byte[0] || compression_method_and_flag_byte[1] || compression_method_and_flag_byte[2]) {
                    std::cout << "Error. bits 0 -> 2 of 'Compression method' are not all zero.";
                    std::exit(EXIT_FAILURE);
                }
                constexpr int number_of_bits_in_compression_method_mask = 4;
                const std::bitset<8> compression_info = compression_method_and_flag_byte >> number_of_bits_in_compression_method_mask;
                const int compression_info_base_2_minus_8 = compression_info.to_ulong();
                const int LZ77_window_size = std::pow(2, compression_info_base_2_minus_8 + 8);

                const std::bitset<8> flags = data_.at(buffer_offset + 5);
                const int fcheck = ((flags << 3) >> 3).to_ulong();
                // RFC 1950 page 5 goes crazy idk.
                const int check_value = (compression_method_and_flag_byte.to_ulong() * 256) + flags.to_ulong();
                if (check_value % 31 != 0){
                    std::cout << "Error. The FCHECK value must be such that CMF and FLG, when viewed as a 16-bit unsigned integer stored in MSB order (CMF*256 + FLG), is a multiple of 31.\n";
                    std::exit(EXIT_FAILURE);
                }
                const bool fdict = flags[5];
                // Not needed for decompression but indicates the speed of the compression algorithm.
                const int flevel = (flags >> 6).to_ulong();
                if (fdict){
                    std::cout << "The fdict bit was set, preset dictionary is not currently supported\n";
                    std::exit(EXIT_FAILURE);
                }
                constexpr int size_of_cmf_flg_bytes = 2;
                const int offset_to_compressed_data = buffer_offset + size_of_ascii_chunk_name + size_of_cmf_flg_bytes;
                constexpr int size_of_ADLER32_check_sum = 4;
                const int size_of_compressed_data = chunk_size - size_of_ADLER32_check_sum - size_of_cmf_flg_bytes;
                std::vector<unsigned char> compressed_data{};
                for (int i = 0; i < size_of_compressed_data; i++){
                    compressed_data.push_back(data_.at(offset_to_compressed_data + i));
                }
                constexpr int number_of_bytes_per_pixel = 4; // assuming argb
                deflate::inflate(compressed_data, width_ * height_ * number_of_bits_in_compression_method_mask);
            }
            else {
                std::cout << chunk_size << ":" 
                    << data_.at(buffer_offset + 0)
                    << data_.at(buffer_offset + 1)
                    << data_.at(buffer_offset + 2)
                    << data_.at(buffer_offset + 3) << "\n";
            }
            buffer_offset += size_of_ascii_chunk_name + chunk_size + size_of_check_sum;
        }
    }
    ~Image() {
        std::cout << "Image destructor called\n";
    }

    Image() = delete;
    Image(const Image& other) = delete;
    Image(Image&& other) = delete;
    Image& operator=(const Image& other) = delete;
    Image& operator=(Image&& other) = delete;
};

#endif