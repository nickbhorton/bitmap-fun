#include "Png.h"

static constexpr bool verbose_construction = true;

static constexpr unsigned char png_signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

static const char* critical_chunk_names[] = {
    "IHDR", "IDAT", "PLTE", "IEND",
};

void Png::load_data_from_file_path() {
    std::ifstream file;
    try {
        file.open(file_path, std::ios::in | std::ios::binary);
        while (!file.eof()) {
            char c{};
            file.get(c);
            if (!file.eof()){
                data_.push_back(PngByte(static_cast<unsigned char>(c)));
            }
            
        }
        file.close();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "Exception opening/reading/closing file\n";
    }
}

bool Png::validate_png_signature()
{
    for (unsigned long i = 0; i < sizeof(png_signature); i++) {
        if (data_[i].data != png_signature[i]){
            return false;
        }
    }
    return true;
}

bool Png::validate_IHDR() {
    for (int i = 0; i < 4; i++) {
        if (static_cast<char>(chunks_[0].type[i]) != critical_chunk_names[0][i]) {
            return false;
        }
    }
    return true;
}

bool Png::validate_IDAT()
{
    int number_of_IDAT_chunks = 0;
    bool previous_chunk_was_IDAT = false;
    for (std::size_t i = 0; i < chunks_.size(); ++i) {
        bool is_IDAT_chunk = true;
        for (int i = 0; i < 4; i++) {
            if (static_cast<char>(chunks_[i].type[i]) != critical_chunk_names[1][i]) {
                is_IDAT_chunk = false;
                previous_chunk_was_IDAT = false;
                break;
            }
        }
        if (is_IDAT_chunk) {
            previous_chunk_was_IDAT = true;
            number_of_IDAT_chunks++;
        }
    }
}


Png::Png(const std::string& path_to_image) :
    data_{},
    chunks_{},
    header_{},
    IDAT_chunk_indexes{},
    file_path{path_to_image},
    parsing_success{false}
{
    load_data_from_file_path();
    bool valid_png_signature_found = validate_png_signature();
    if constexpr (verbose_construction) {
        if (valid_png_signature_found) {
            std::cout << file_path << ": valid png signature\n";
        }
        else {
            std::cout << file_path << ": invalid png signature\n";
        }
    }
    if (!valid_png_signature_found) return;
    populate_chunks();
    bool valid_IHDR = validate_IHDR();
    if constexpr (verbose_construction) {
        if (valid_IHDR) {
            std::cout << file_path << ": valid IHDR position\n";
        }
        else {
            std::cout << file_path << ": invalid IHDR position\n";
        }
    }
    if (!valid_IHDR) return;
    populate_header();
}

Png::~Png() {
    if constexpr (verbose_construction) {
        std::cout << file_path << ": PNG destructor called\n";
    }
}

void Png::print_data_hex(int width) const {
    int line_width = 0;
    std::cout << "file size: " << data_.size() << "\n";
    for (auto const& c : data_){
        std::cout << c << " ";
        ++line_width;
        if (line_width == width) {
            std::cout << "\n";
            line_width = 0;
        }
    }
    std::cout << "\n";
}

uint32_t Png::get_uint32_t_h(std::size_t index_into_data) const {
    uint32_t result = 0;
    result += static_cast<uint32_t>(data_[index_into_data + 0].data) << 24;
    result += static_cast<uint32_t>(data_[index_into_data + 1].data) << 16;
    result += static_cast<uint32_t>(data_[index_into_data + 2].data) << 8;
    result += static_cast<uint32_t>(data_[index_into_data + 3].data) << 0;
    return result;
}

void Png::populate_chunks() {
    std::size_t current_index = sizeof(png_signature);
    while (current_index < data_.size()) {
        uint32_t length = get_uint32_t_h(current_index);
        current_index += sizeof(uint32_t);
                        // length + crc int + name chars
        if (current_index + length + 4 + 4 > data_.size()) {
            if constexpr (verbose_construction) {
                std::cout << "Error: length found was to big for file size.\n";
            }
            break;
        }

        unsigned char type[4];
        for (int i = 0; i < 4; i++) {
            type[i] = data_[current_index].data;
            if (!((type[i] >= 0x41 && type[i] <= 0x5A) || (type[i] >= 0x61 && type[i] <= 0x7A))) {
                if constexpr (verbose_construction) {
                    std::cout << "Error: chunk name parsing found a non ascii character.\n";
                }
                break;
            }
            current_index++;
        }
        std::size_t chunk_data_start = current_index;
        current_index += length;
        uint32_t crc = get_uint32_t_h(current_index);
        current_index += sizeof(uint32_t);
        if constexpr (verbose_construction) {
            std::cout << "chunk " << type[0] << type[1] << type[2] << type[3] 
                      << " length: " << length << " start: " << chunk_data_start 
                      << " crc: " << crc << "\n";
        }
        chunks_.push_back(Chunk {
            .length = length,
            .type = {type[0], type[1], type[2], type[3]},
            .chunk_data_start = chunk_data_start,
            .crc = crc
        });
    }
}

void Png::populate_header() {
    constexpr int sizeof_length_field = 4;
    constexpr int sizeof_name_field = 4;
    std::size_t current_index = sizeof(png_signature) + sizeof_length_field + sizeof_name_field;
    header_.width = get_uint32_t_h(current_index);
    current_index += sizeof(uint32_t);
    if constexpr (verbose_construction) {
        std::cout << "IHDR width: " << header_.width << "\n";
    }
    header_.height = get_uint32_t_h(current_index);
    current_index += sizeof(uint32_t);
    if constexpr (verbose_construction) {
        std::cout << "IHDR height: " << header_.width << "\n";
    }
    header_.bit_depth = data_[current_index].data;
    current_index++;
    if constexpr (verbose_construction) {
        std::cout << "IHDR bit depth: " << (int) header_.bit_depth << "\n";
    }
    header_.color_type = data_[current_index].data;
    current_index++;
    if constexpr (verbose_construction) {
        std::cout << "IHDR color type: " << (int) header_.color_type << "\n";
    }
    header_.compression_method = data_[current_index].data;
    current_index++;
    if constexpr (verbose_construction) {
        std::cout << "IHDR compression method: " << (int) header_.compression_method << "\n";
    }
    header_.filter_method = data_[current_index].data;
    current_index++;
    if constexpr (verbose_construction) {
        std::cout << "IHDR filter method: " << (int) header_.filter_method << "\n";
    }
    header_.interlace_method = data_[current_index].data;
    current_index++;
    if constexpr (verbose_construction) {
        std::cout << "IHDR interlace method: " << (int) header_.interlace_method << "\n";
    }
}