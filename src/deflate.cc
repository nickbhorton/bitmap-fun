// Most algorithms here are heavily inspired if not outright copied from puff.c

#include "deflate.h"

constexpr int MaxBitsInACode = 15;
// LL indicates literal bytes and length codes (which share the same huffman tree)
constexpr int  MaxCodesForLL = 286;
// Dist indicate the distance huffman tree
constexpr int  MaxCodesForDist = 30;
constexpr int  MaxCodesTotal = MaxCodesForLL + MaxCodesForDist;
constexpr int FixedCodesForLL = 288;

namespace deflate {

HuffmanTree calculate_huffman_tree(const std::vector<int>& bit_lengths, int n) {
    // Index into vector signifies the bit length
    std::vector<int> codes_per_bit_length{};
    // +1 because zero is included even though its not possible
    //     This is in an effort to make the code more readable
    codes_per_bit_length.resize(MaxBitsInACode + 1);
    for (auto& e : codes_per_bit_length) {
        e = 0;
    }
    for (unsigned int i = 0; i < n; i++){
        ++codes_per_bit_length[bit_lengths[i]];
    }
    // print the bit_length_counts
    std::cout << "calculate_huffman_tree\ncodes_per_bit_length: ";
    for (const auto& e : codes_per_bit_length){
        std::cout << e << " ";
    }
    std::cout << "\n";

    // sanity check: make sure that number of symbols makes sense for
    // how many bits we have available

    // a bit length of zero can only signify one code
    int number_of_codes_left = 1;
    for (int current_bit_length = 1; current_bit_length < MaxBitsInACode; current_bit_length++){
        number_of_codes_left <<= 1; // adding a bit allows for the code to specify 2 times as many codes
        number_of_codes_left -= codes_per_bit_length[current_bit_length]; // remove number of codes at that bit length
        if (number_of_codes_left < 0){
            std::cout << "Error: ran out of possible codes with our limited amount of bits. indicates calculating huffman tree is impossible\n";
            std::exit(EXIT_FAILURE);
        }
    }

    std::vector<int> offsets_into_symbol_array_for_each_length{};
    offsets_into_symbol_array_for_each_length.resize(MaxBitsInACode + 1);
    offsets_into_symbol_array_for_each_length[1] = 0;
    for (int current_bit_length = 1; current_bit_length < MaxBitsInACode; current_bit_length++) {
        offsets_into_symbol_array_for_each_length[current_bit_length + 1] = 
            offsets_into_symbol_array_for_each_length[current_bit_length] + codes_per_bit_length[current_bit_length];
    }
    // print the offsets_into_symbol_array_for_each_length
    std::cout << "offsets_into_symbol_array_for_each_length: ";
    for (const auto& e : offsets_into_symbol_array_for_each_length){
        std::cout << e << " ";
    }
    std::cout << "\n";


    std::vector<int> symbols{};
    symbols.resize(bit_lengths.size());

    for (int symbol = 0; symbol < n; symbol++) {
        if (bit_lengths[symbol] != 0) {
            symbols[offsets_into_symbol_array_for_each_length[bit_lengths[symbol]]++] = symbol;
        }
    }   
    // print the symbols

    std::cout << "symbols for computed tree: ";
    for (const auto& e : symbols){
        std::cout << e << " ";
    }
    std::cout << "\n";
    return HuffmanTree{std::move(codes_per_bit_length), std::move(symbols)};
}

static bool get_next_bit(const std::vector<unsigned char>& encoded_bytes) {
    static int bit_position = 0;
    assert(bit_position >= 0);
    assert(bit_position <= 8*encoded_bytes.size());
    const std::bitset<8> bits {encoded_bytes.at(bit_position/8)};
    return bits[bit_position++%8];
}

static int get_next_n_bits(const std::vector<unsigned char>& encoded_bytes, int n) {
    int result = 0;
    assert(n <= MaxBitsInACode);
    for (int i = 0; i < n; i++){
        result |= get_next_bit(encoded_bytes) << i;
    }
    return result;
}

int decode_symbol(const std::vector<unsigned char>& encoded_bytes, const HuffmanTree& tree) {
    int code{};
    int number_of_codes_for_current_bit_length{};
    // Index into tree.symbols for the first symbol at current bit length
    int index{};
    int first{};
    for (int number_of_bits_in_code = 1; number_of_bits_in_code < MaxBitsInACode; number_of_bits_in_code++){
        code |= get_next_bit(encoded_bytes);
        number_of_codes_for_current_bit_length = tree.count[number_of_bits_in_code];
        // printf("code: %d  count: %d  first: %d index: %d\n", code, number_of_codes_for_current_bit_length, first, index);
        if (code - number_of_codes_for_current_bit_length < first) {
            // printf("symbol: %d\n", tree.symbol[index + (code - first)]);
            return tree.symbol[index + (code - first)];
        }
        index += number_of_codes_for_current_bit_length;
        first += number_of_codes_for_current_bit_length;
        first <<= 1;
        code <<= 1;
    }
    std::cout << "not enough bits in maximum bit length to decode the current code\n";
    std::exit(EXIT_FAILURE);
}

void decode_symbols(
    const std::vector<unsigned char>& encoded_bytes, 
    const HuffmanTree& ll_tree, 
    const HuffmanTree& d_tree, 
    std::size_t size_of_decoded_bytes
) {
    // direct from puff.c
    static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    std::cout << "decode_symbols() called\nll_tree:\n";
    // std::cout << ll_tree << "\n";
    std::vector<unsigned char> decoded_bytes{};
    // std::cout << "decoding:\n";
    while (decoded_bytes.size() < size_of_decoded_bytes){
        int decoded_byte = decode_symbol(encoded_bytes, ll_tree);
        // std::cout << (int) decoded_byte << "\n";
        if (decoded_byte < 256){
            decoded_bytes.push_back((unsigned char) decoded_byte);
        }
        else if (decoded_byte > 256){
            decoded_byte -= 257;
            if (decoded_byte >= 29) {
                std::cout << "not a valid length\n";
                std::exit(EXIT_FAILURE);
            }
            int len = lens[decoded_byte] + get_next_n_bits(encoded_bytes, lext[decoded_byte]);
            // std::cout << "len: " << len << "\n";

            decoded_byte = decode_symbol(encoded_bytes, d_tree);
            // std::cout << "decoded_byte (d_tree): " << decoded_byte << "\n";

            int distance = dists[decoded_byte] + get_next_n_bits(encoded_bytes, dext[decoded_byte]);
            // std::cout << "distance: " << distance << "\n";
            while (len--) {
                // std::cout << (int) decoded_bytes[decoded_bytes.size() - distance] << " ";
                decoded_bytes.push_back(decoded_bytes[decoded_bytes.size() - distance]);
            }
            // std::cout << "\ndecoded bytes vector size: " << decoded_bytes.size() << "\n";
        }
        else {
            std::cout << "not a direct byte lol\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < decoded_bytes.size(); i += 4){
        std::cout << (int) decoded_bytes[i + 0] << " ";
        std::cout << (int) decoded_bytes[i + 1] << " ";
        std::cout << (int) decoded_bytes[i + 2] << " ";
        std::cout << (int) decoded_bytes[i + 3] << "\n";
    }
}

void decode_fixed(const std::vector<unsigned char>& encoded_bytes, std::size_t size_of_decoded_bytes) {
    static bool first_call = true;
    static HuffmanTree fixed_tree{};
    if (first_call) {
        std::vector<int> lengths{};
        int symbol{};
        for (symbol = 0; symbol < 144; symbol++) {
            lengths.push_back(8);
        }
        for (; symbol < 256; symbol++) {
            lengths.push_back(9);
        }
        for (; symbol < 280; symbol++) {
            lengths.push_back(7);
        }
        for (; symbol < FixedCodesForLL; symbol++) {
            lengths.push_back(8);
        }
        fixed_tree = calculate_huffman_tree(lengths, FixedCodesForLL);
        first_call = false;
    }
    // TODO: fix this for d_tree
    decode_symbols(encoded_bytes, fixed_tree, fixed_tree, size_of_decoded_bytes);
}

void decode_dynamic(const std::vector<unsigned char>& encoded_bytes, std::size_t size_of_decoded_bytes) {
    // RFC 1951
    int number_of_ll_codes = get_next_n_bits(encoded_bytes, 5) + 257;
    std::cout << "number_of_LL_codes: " << number_of_ll_codes << "\n";
    int number_of_distance_codes = get_next_n_bits(encoded_bytes, 5) + 1;
    std::cout << "number_of_distance_codes: " << number_of_distance_codes << "\n";
    int number_of_code_length_codes = get_next_n_bits(encoded_bytes, 4) + 4;
    std::cout << "number_of_code_length_codes: " << number_of_code_length_codes << "\n";
    if (number_of_ll_codes > MaxCodesForLL) {
        std::cout << "Error: the dynamic huffman tree specified has too many literal/length codes\n";
        std::exit(EXIT_FAILURE);
    }
    if (number_of_distance_codes > MaxCodesForDist) {
        std::cout << "Error: the dynamic huffman tree specified has too many distance codes\n";
        std::exit(EXIT_FAILURE);
    }
    const std::vector<int> order {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    std::vector<int> lengths {};
    lengths.resize(order.size());
    for (int i = 0; i < order.size(); i++){
        if (i < number_of_code_length_codes) {
            const int current_length = get_next_n_bits(encoded_bytes, 3);
            lengths[order[i]] = current_length;
        }
        else {
            lengths[order[i]] = 0;
        }
    }

    HuffmanTree treetree = calculate_huffman_tree(lengths, 19);

    lengths.clear();
    lengths.resize(MaxCodesTotal);
    int index {0};
    while (index < number_of_ll_codes + number_of_distance_codes) {
        int symbol = decode_symbol(encoded_bytes, treetree);
        if (symbol < 16) {
            lengths[index++] = symbol;
        }
        else {
            int len{0};
            if (symbol == 16) {
                if (index == 0) {
                    std::cout << "Error: no tree specified\n";
                    std:exit(EXIT_FAILURE);
                }
                len = lengths[index - 1];
                symbol = 3 + get_next_n_bits(encoded_bytes, 2);
            }
            else if (symbol == 17){
                symbol = 3 + get_next_n_bits(encoded_bytes, 3);
            }
            else {
                symbol = 11 + get_next_n_bits(encoded_bytes, 7);
            }
            if (index + symbol > number_of_ll_codes + number_of_distance_codes) {
                std::cout << "Error: to many lengths specified in the tree\n";
                std::exit(EXIT_FAILURE);
            }
            while (symbol--) {
                lengths[index++] = len;
            }
        }
    }

    /* check for end-of-block code -- there better be one! */
    if (lengths[256] == 0) {
        std::cout << "Error: no end-of-block code\n";
        std::exit(EXIT_FAILURE);
    }

    HuffmanTree ll_tree = calculate_huffman_tree(lengths, number_of_ll_codes);
    std::cout << ll_tree << "\n";
    HuffmanTree distance_tree = calculate_huffman_tree(std::vector<int>(lengths.begin() + number_of_ll_codes, lengths.end()), number_of_distance_codes);
    std::cout << distance_tree << "\n";

    decode_symbols(encoded_bytes, ll_tree, distance_tree, size_of_decoded_bytes);
}

enum BTypeCompression {
    NoCompression,
    FixedHuffmanCodes,
    DynamicHuffmanCodes,
    Reserved
};

void inflate(const std::vector<unsigned char>& encoded_bytes, std::size_t size_of_decoded_bytes){
    bool is_final_block = get_next_bit(encoded_bytes);
    int compression_type = get_next_n_bits(encoded_bytes, 2);

    if (is_final_block) {
        std::cout << "this is the final block\n";
    }

    if (compression_type == NoCompression){
        std::cout << "compression_type: no compression\n";
        std::cout << "this is unsupported for now\n";
        std::exit(EXIT_FAILURE);
    }
    else if (compression_type == FixedHuffmanCodes){
        std::cout << "compression_type: fixed huffman codes\n";
        decode_fixed(encoded_bytes, size_of_decoded_bytes);
    }
    else if (compression_type == DynamicHuffmanCodes){
        std::cout << "compression_type: dynamic huffman codes\n";
        decode_dynamic(encoded_bytes, size_of_decoded_bytes);
    }
    else if (compression_type == Reserved){
        std::cout << "compression_type: reserved, probably an error\n";
        std::exit(EXIT_FAILURE);
    }
}

} // namespace deflate