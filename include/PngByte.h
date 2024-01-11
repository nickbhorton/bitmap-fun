#ifndef PNG_BYTE_HEADER
#define PNG_BYTE_HEADER

#include <ostream>
#include <cassert>

struct PngByte {
    unsigned char data;

    PngByte(unsigned char c);

    char get_upper_nibble_hex() const;
    char get_lower_nibble_hex() const;

    friend std::ostream& operator<<(std::ostream& os, const PngByte& b);
};

#endif
