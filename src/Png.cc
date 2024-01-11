#include "Png.h"

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


Png::Png(const std::string& path_to_image) :
    file_path{path_to_image}
{
    Png::load_data_from_file_path();
}
Png::~Png() {
    std::cout << "PNG destructor for " << file_path << " called\n";
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