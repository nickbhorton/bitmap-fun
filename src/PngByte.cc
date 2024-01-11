#include "PngByte.h"

static char nibble_to_hex(unsigned char c) {
    switch (c & 0b00001111)
    {
        case 0: return '0'; break;
        case 1: return '1'; break;
        case 2: return '2'; break;
        case 3: return '3'; break;
        case 4: return '4'; break;
        case 5: return '5'; break;
        case 6: return '6'; break;
        case 7: return '7'; break;
        case 8: return '8'; break;
        case 9: return '9'; break;
        case 10: return 'A'; break;
        case 11: return 'B'; break;
        case 12: return 'C'; break;
        case 13: return 'D'; break;
        case 14: return 'E'; break;
        case 15: return 'F'; break;
    }
    return 0;
}


PngByte::PngByte(unsigned char c) : data{c} {}

char PngByte::get_upper_nibble_hex() const {
    return nibble_to_hex(data >> 4);
}
char PngByte::get_lower_nibble_hex() const {
    return nibble_to_hex(data);
}

std::ostream& operator<<(std::ostream& os, const PngByte& b) {
    os << b.get_upper_nibble_hex() << b.get_lower_nibble_hex();
    return os;
}