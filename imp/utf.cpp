#include "utf.hpp"

#include <imp/dependent_false.hpp>

#include <stdexcept>

/*
 * Implementation of UTF encoding/decoding comes from https://wiki.ubc.ca/images/9/9a/Layout_of_UTF-8_byte_sequences.png
 */
namespace imp::utf
{
    static constexpr int B1_CAP = 0b1000'0000; // 0x80
    static constexpr int B1_ERASURE = 0b0000'0000; // 0x00
    static constexpr int B2_CAP = 0b1110'0000; // 0xE0
    static constexpr int B2_MASK = 0b0001'1111; // 0x1F
    static constexpr int B2_ERASURE = 0b1100'0000; // 0xC0
    static constexpr int B3_CAP = 0b1111'0000; // 0xF0
    static constexpr int B3_MASK = 0b0000'1111; // 0x0F
    static constexpr int B3_ERASURE = 0b1110'0000; // 0xE0
    static constexpr int B4_CAP = 0b1111'1000; // 0xF8
    static constexpr int B4_MASK = 0b0000'0111; // 0x07
    static constexpr int B4_ERASURE = 0b1111'0000; // 0xF0
    static constexpr int CONT_MASK = 0b0011'1111; // 0x3F
    static constexpr int CONT_HEAD = 0b1000'0000; // 0x80
    static constexpr int CONT_CAPTURE = 0b1100'0000; // 0xC0
    static constexpr int SURR_OFFSET = 0x0001'0000; // 0x10000 = 0b00000000'00000001'00000000'00000000
    static constexpr int SURR_SHIFT = 10;
    static constexpr int SURR_HIGH_OFFSET = 0xD800;      // 0xD800   = 0b11011000'00000000
    static constexpr int SURR_HIGH_MAX = 0xDBFF;      // 0xDBFF   = 0b11011011'11111111
    static constexpr int SURR_LOW_OFFSET = 0xDC00;      // 0xDC00   = 0b11011100'00000000
    static constexpr int SURR_LOW_MAX = 0xDFFF;      // 0xDFFF   = 0b11011111'11111111
    static constexpr int SURR_LOW_MASK = 0x03FF;      // 0x3FF    = 0b00000011'11111111
    static constexpr int CODEPOINT_B1_MAX = 0x0000'007F; // 0x7F     = 0b00000000'00000000'00000000'01111111
    static constexpr int CODEPOINT_B2_MAX = 0x0000'07FF; // 0x7FF    = 0b00000000'00000000'00000111'11111111
    static constexpr int CODEPOINT_B3_MAX = 0x0000'FFFF; // 0xFFFF   = 0b00000000'00000000'11111111'11111111
    static constexpr int CODEPOINT_B4_MAX = 0x0010'FFFF; // 0x10FFFF = 0b00000000'00010000'11111111'11111111

    template<int bits> struct bytes { static_assert(imp::dependent_false_v<bits>); };
    template<>         struct bytes<8> { using type = char8_t; };
    template<>         struct bytes<16> { using type = char16_t; };
    template<>         struct bytes<32> { using type = char32_t; };
    template<int bits> using bytes_t = bytes<bits>::type;
    template<int bits> static constexpr bytes_t<bits> CH(auto c) { return static_cast<bytes_t<bits>>(c); }

#define UC(c) static_cast<unsigned char>(c)

    // TODO utf_error enum code

    static void encode_into(char32_t codepoint, char8_t quad[4], std::u8string& utf8, bool ignore_invalid_chars)
    {
        if (codepoint <= CODEPOINT_B1_MAX)
        {
            // 1-byte (ASCII)
            quad[0] = CH<8>(codepoint);
            utf8.append(quad, 1);
        }
        else if (codepoint <= CODEPOINT_B2_MAX)
        {
            // 2-byte
            quad[0] = CH<8>((codepoint >> 6) | B2_ERASURE);
            quad[1] = CH<8>((codepoint & CONT_MASK) | CONT_HEAD);
            utf8.append(quad, 2);
        }
        else if (codepoint <= CODEPOINT_B3_MAX)
        {
            // 3-byte
            quad[0] = CH<8>((codepoint >> 12) | B3_ERASURE);
            quad[1] = CH<8>(((codepoint >> 6) & CONT_MASK) | CONT_HEAD);
            quad[2] = CH<8>((codepoint & CONT_MASK) | CONT_HEAD);
            utf8.append(quad, 3);
        }
        else if (codepoint <= CODEPOINT_B4_MAX)
        {
            // 4-byte
            quad[0] = CH<8>((codepoint >> 18) | B4_ERASURE);
            quad[1] = CH<8>(((codepoint >> 12) & CONT_MASK) | CONT_HEAD);
            quad[2] = CH<8>(((codepoint >> 6) & CONT_MASK) | CONT_HEAD);
            quad[3] = CH<8>((codepoint & CONT_MASK) | CONT_HEAD);
            utf8.append(quad, 4);
        }
        else if (!ignore_invalid_chars)
        {
            quad[0] = quad[1] = quad[2] = quad[3] = 0;
            throw std::exception("Invalid Unicode codepoint"); // TODO use imp::error ?
        }
        quad[0] = quad[1] = quad[2] = quad[3] = 0;
    }

    std::u8string encode(const std::u16string& utf16, bool ignore_invalid_chars)
    {
        std::u8string utf8;
        char8_t quad[4]{ 0, 0, 0, 0 };
        char32_t codepoint = 0;

        for (size_t i = 0; i < utf16.size(); ++i)
        {
            codepoint = CH<32>(utf16[i]);

            if (codepoint >= SURR_HIGH_OFFSET && codepoint <= SURR_HIGH_MAX)
            {
                // Surrogate pair (4-byte UTF-8)
                if (i + 1 >= utf16.size())
                {
                    if (!ignore_invalid_chars)
                        throw std::exception("Invalid UTF-16 sequence"); // TODO use imp::error ?
                    continue;
                }
                char32_t low_surrogate = CH<32>(utf16[i + 1]);

                if (low_surrogate < SURR_LOW_OFFSET || low_surrogate > SURR_LOW_MAX) {
                    if (!ignore_invalid_chars)
                        throw std::exception("Invalid UTF-16: unpaired high surrogate"); // TODO use imp::error ?
                    continue;
                }
                ++i;
                codepoint = ((codepoint - SURR_HIGH_OFFSET) << SURR_SHIFT) + (low_surrogate - SURR_LOW_OFFSET) + SURR_OFFSET;
            }
            else if (codepoint >= SURR_LOW_OFFSET && codepoint <= SURR_LOW_MAX)
            {
                if (!ignore_invalid_chars)
                    throw std::exception("Invalid UTF-16: unpaired low surrogate"); // TODO use imp::error ?
                continue;
            }

            encode_into(codepoint, quad, utf8, ignore_invalid_chars);
        }

        return utf8;
    }

    std::u16string decode_utf16(const std::u8string& utf8)
    {
        std::u16string utf16;

        for (size_t i = 0; i < utf8.size();)
        {
            unsigned char byte = UC(utf8[i]);
            char32_t character = 0;

            if ((byte & B1_CAP) == B1_ERASURE) {
                // 1-byte (ASCII)
                character |= byte;
                i += 1;
            }
            else if ((byte & B2_CAP) == B2_ERASURE) {
                // 2-byte
                if (i + 1 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B2_MASK) << 6;
                character |= UC(utf8[i + 1]) & CONT_MASK;
                i += 2;
            }
            else if ((byte & B3_CAP) == B3_ERASURE) {
                // 3-byte
                if (i + 2 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B3_MASK) << 12;
                character |= (UC(utf8[i + 1]) & CONT_MASK) << 6;
                character |= UC(utf8[i + 2]) & CONT_MASK;
                i += 3;
            }
            else if ((byte & B4_CAP) == B4_ERASURE) {
                // 4-byte (surrogate pair)
                if (i + 3 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B4_MASK) << 18;
                character |= (UC(utf8[i + 1]) & CONT_MASK) << 12;
                character |= (UC(utf8[i + 2]) & CONT_MASK) << 6;
                character |= UC(utf8[i + 3]) & CONT_MASK;

                // Encode as UTF-16 surrogate pair
                character -= SURR_OFFSET;
                utf16.push_back(CH<16>((character >> SURR_SHIFT) + SURR_HIGH_OFFSET)); // high surrogate
                utf16.push_back(CH<16>((character & SURR_LOW_MASK) + SURR_LOW_OFFSET)); // low surrogate
                i += 4;
                continue;
            }
            else
                throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

            utf16.push_back(CH<16>(character));
        }

        return utf16;
    }

    std::u8string encode(const std::u32string& utf32, bool ignore_invalid_chars)
    {
        std::u8string utf8;
        char8_t quad[4]{ 0, 0, 0, 0 };
        for (char32_t codepoint : utf32)
            encode_into(codepoint, quad, utf8, ignore_invalid_chars);
        return utf8;
    }

    std::u32string decode_utf32(const std::u8string& utf8)
    {
        std::u32string utf32;
        size_t i = 0;
        while (i < utf8.size())
        {
            unsigned char byte = UC(utf8[i]);
            char32_t character = 0;

            if ((byte & B1_CAP) == B1_ERASURE)
            {
                // 1-byte (ASCII)
                character |= CH<32>(byte);
                i += 1;
            }
            else if ((byte & B2_CAP) == B2_ERASURE)
            {
                // 2-byte
                if (i + 1 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B2_MASK) << 6;
                character |= UC(utf8[i + 1]) & 0x3F;
                i += 2;
            }
            else if ((byte & B3_CAP) == B3_ERASURE)
            {
                // 3-byte
                if (i + 2 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B3_MASK) << 12;
                character |= (UC(utf8[i + 1]) & CONT_MASK) << 6;
                character |= UC(utf8[i + 2]) & CONT_MASK;
                i += 3;
            }
            else if ((byte & B4_CAP) == B4_ERASURE)
            {
                // 4-byte
                if (i + 3 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?
                character |= (byte & B4_MASK) << 18;
                character |= (UC(utf8[i + 1]) & CONT_MASK) << 12;
                character |= (UC(utf8[i + 2]) & CONT_MASK) << 6;
                character |= UC(utf8[i + 3]) & CONT_MASK;
                i += 4;
            }
            else
                throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

            utf32.push_back(character);
        }

        return utf32;
    }

    std::u8string convert(const std::string_view _str)
    {
        return std::u8string(_str.begin(), _str.end());
    }

    std::string convert(const std::u8string& utf8)
    {
        return std::string(utf8.begin(), utf8.end());
    }

    codepoint string::iterator::operator*() const
    {
        if (_i >= _string._str.size())
            throw std::out_of_range("End of _string"); // TODO use imp::error ?
        unsigned char first = UC(_string._str[_i]);
        char32_t cdpt = 0;

        if (first < B1_CAP)
        {
            cdpt |= first;
        }
        else if (first < B2_CAP)
        {
            if (_i + 1 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B2_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK);
        }
        else if (first < B3_CAP)
        {
            if (_i + 2 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B3_MASK) << 12;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 2]) & CONT_MASK);
        }
        else if (first < B4_CAP)
        {
            if (_i + 3 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B4_MASK) << 18;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK) << 12;
            cdpt |= (UC(_string._str[_i + 2]) & CONT_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 3]) & CONT_MASK);
        }

        return codepoint(static_cast<int>(cdpt));
    }

    string::iterator& string::iterator::operator++()
    {
        _i += num_bytes();
        return *this;
    }

    string::iterator string::iterator::operator++(int)
    {
        iterator iter(_string, _i);
        _i += num_bytes();
        return iter;
    }

    string::iterator& string::iterator::operator--()
    {
        if (_i == 0)
            throw std::out_of_range("Start of string"); // TODO use imp::error ?
        --_i;
        while ((UC(_string._str[_i]) & CONT_CAPTURE) == CONT_HEAD)
        {
            if (_i == 0)
                throw std::exception("UTF-8 invalid starting byte"); // TODO use imp::error ?
            --_i;
        }
        return *this;
    }

    string::iterator string::iterator::operator--(int)
    {
        iterator it(_string, _i);
        --*this;
        return it;
    }

    bool string::iterator::operator==(const iterator& other) const
    {
        return &_string == &other._string && _i == other._i;
    }

    bool string::iterator::operator!=(const iterator& other) const
    {
        return &_string != &other._string || _i != other._i;
    }

    char string::iterator::num_bytes() const
    {
        unsigned char first = UC(_string._str[_i]);
        if (first < B1_CAP)
            return 1;
        else if (first < B2_CAP)
            return 2;
        else if (first < B3_CAP)
            return 3;
        else if (first < B4_CAP)
            return 4;
        else
            throw std::exception("Invalid UTF-8"); // TODO use imp::error ?
    }

    string::iterator::operator bool() const
    {
        return _i < _string._str.size();
    }

    codepoint string::iterator::advance()
    {
        if (_i >= _string._str.size())
            throw std::out_of_range("End of _string"); // TODO use imp::error ?
        unsigned char first = UC(_string._str[_i]);
        char32_t cdpt = 0;

        if (first < B1_CAP)
        {
            cdpt |= first;
            _i += 1;
        }
        else if (first < B2_CAP)
        {
            if (_i + 1 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B2_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK);
            _i += 2;
        }
        else if (first < B3_CAP)
        {
            if (_i + 2 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B3_MASK) << 12;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 2]) & CONT_MASK);
            _i += 3;
        }
        else if (first < B4_CAP)
        {
            if (_i + 3 >= _string._str.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?
            cdpt |= (first & B4_MASK) << 18;
            cdpt |= (UC(_string._str[_i + 1]) & CONT_MASK) << 12;
            cdpt |= (UC(_string._str[_i + 2]) & CONT_MASK) << 6;
            cdpt |= (UC(_string._str[_i + 3]) & CONT_MASK);
            _i += 4;
        }
        else
            throw std::exception("Invalid UTF-8"); // TODO use imp::error ?

        return codepoint(static_cast<int>(cdpt));
    }

    std::string string::str() const
    {
        std::string s;
        s.reserve(size());
        for (auto it = begin(); it; ++it)
            s.push_back(*it);
        return s;
    }

    bool string::begins_with(const utf::string& other) const
    {
        auto it1 = begin();
        auto it2 = other.begin();
        while (true)
        {
            if (it1)
            {
                if (it2)
                {
                    if (*it1 == *it2)
                    {
                        ++it1;
                        ++it2;
                    }
                    else
                        return false;
                }
                else
                    return true;
            }
            else
                return !it2;
        }
    }

    void string::push_back(codepoint cdpnt)
    {
        int codepoint = cdpnt;
        char8_t quad[4]{ 0, 0, 0, 0 };
        if (codepoint <= CODEPOINT_B1_MAX)
        {
            quad[0] = CH<8>(codepoint);
            _str.append(quad, 1);
        }
        else if (codepoint <= CODEPOINT_B2_MAX)
        {
            quad[0] = CH<8>(B2_ERASURE | (codepoint >> 6));
            quad[1] = CH<8>(CONT_HEAD | (codepoint & CONT_MASK));
            _str.append(quad, 2);
        }
        else if (codepoint <= CODEPOINT_B3_MAX)
        {
            quad[0] = CH<8>(B3_ERASURE | (codepoint >> 12));
            quad[1] = CH<8>(CONT_HEAD | ((codepoint >> 6) & CONT_MASK));
            quad[2] = CH<8>(CONT_HEAD | (codepoint & CONT_MASK));
            _str.append(quad, 3);
        }
        else if (codepoint <= CODEPOINT_B4_MAX)
        {
            quad[0] = CH<8>(B4_ERASURE | (codepoint >> 18));
            quad[1] = CH<8>(CONT_HEAD | ((codepoint >> 12) & CONT_MASK));
            quad[2] = CH<8>(CONT_HEAD | ((codepoint >> 6) & CONT_MASK));
            quad[3] = CH<8>(CONT_HEAD | (codepoint & CONT_MASK));
            _str.append(quad, 4);
        }
        else
            throw std::exception("codepoint is out of valid UTF-8 range"); // TODO use imp::error ?
    }

    string string::operator+(const string& rhs) const
    {
        return utf::string(_str + rhs._str);
    }

    string& string::operator+=(const string& rhs)
    {
        _str += rhs._str;
        return *this;
    }

    string string::operator*(size_t n) const
    {
        std::u8string temp;
        for (size_t i = 0; i < n; ++i)
            temp += _str;
        return string(std::move(temp));
    }

    string& string::operator*=(size_t n)
    {
        std::u8string temp;
        for (size_t i = 0; i < n; ++i)
            temp += _str;
        _str.swap(temp);
        return *this;
    }

    string::string(const iterator& begin_, const iterator& end_)
    {
        if (&begin_._string == &end_._string)
            _str = begin_._string._str.substr(begin_._i, end_._i - begin_._i);
        else
            throw std::exception("imp::utf::string iterators are not compatible."); // TODO use imp::error ?
    }

    string string::substr(size_t begin_, size_t end_) const
    {
        iterator index = begin();
        size_t i = 0;
        while (i < begin_)
        {
            ++index;
            ++i;
        }
        iterator b = index;
        while (i < end_)
        {
            ++index;
            ++i;
        }
        return string(b, index);
    }

#undef UC
}
