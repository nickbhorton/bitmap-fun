#ifndef DEFLATE_HEADER
#define DEFLATE_HEADER

#include <vector>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <cassert>
#include <array>

namespace deflate
{
struct HuffmanTree {
    std::vector<int> count;
    std::vector<int> symbol;
    ~HuffmanTree() {
        std::cout << "HuffmanTree deconstructor called\n";
    }
    friend std::ostream& operator<<(std::ostream& os, const HuffmanTree& ht) {
        os << "count: ";
        for (const auto& e : ht.count) {
            os << e << " ";
        }
        os << "\n";
        os << "symbol: ";
        for (const auto& e : ht.symbol) {
            os << e << " ";
        }
        return os;
    }
};

HuffmanTree calculate_huffman_tree(const std::vector<int>& bit_lengths);
void inflate(const std::vector<unsigned char>& encoded_bytes, std::size_t size_of_decoded_bytes);
} // namespace deflate

#endif
