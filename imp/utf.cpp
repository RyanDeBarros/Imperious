#include "utf.hpp"

#include <imp/dependent_false.hpp>

#include <stdexcept>

/*
 * Implementation of UTF encoding/decoding comes from
 * https://wiki.ubc.ca/images/9/9a/Layout_of_UTF-8_byte_sequences.png
 */
namespace imp::utf
{
    static constexpr int kB1Cap = 0b1000'0000;     // 0x80
    static constexpr int kB1Erasure = 0b0000'0000; // 0x00

    static constexpr int kB2Cap = 0b1110'0000;     // 0xE0
    static constexpr int kB2Mask = 0b0001'1111;    // 0x1F
    static constexpr int kB2Erasure = 0b1100'0000; // 0xC0
    
    static constexpr int kB3Cap = 0b1111'0000;     // 0xF0
    static constexpr int kB3Mask = 0b0000'1111;    // 0x0F
    static constexpr int kB3Erasure = 0b1110'0000; // 0xE0
    
    static constexpr int kB4Cap = 0b1111'1000;     // 0xF8
    static constexpr int kB4Mask = 0b0000'0111;    // 0x07
    static constexpr int kB4Erasure = 0b1111'0000; // 0xF0

    static constexpr int kContMask = 0b0011'1111; // 0x3F
    static constexpr int kContHead = 0b1000'0000; // 0x80
    static constexpr int kContCapture = 0b1100'0000; // 0xC0

    static constexpr int kSurrOffset = 0x0001'0000; // 0x10000 = 0b00000000'00000001'00000000'00000000
    static constexpr int kSurrShift = 10;

    static constexpr int kSurrHighOffset = 0xD800;  // 0xD800   = 0b11011000'00000000
    static constexpr int kSurrHighMax = 0xDBFF;     // 0xDBFF   = 0b11011011'11111111

    static constexpr int kSurrLowOffset = 0xDC00;   // 0xDC00   = 0b11011100'00000000
    static constexpr int kSurrLowMax = 0xDFFF;      // 0xDFFF   = 0b11011111'11111111
    static constexpr int kSurrLowMask = 0x03FF;     // 0x3FF    = 0b00000011'11111111

    static constexpr int kCodepointB1Max = 0x0000'007F; // 0x7F     = 0b00000000'00000000'00000000'01111111
    static constexpr int kCodepointB2Max = 0x0000'07FF; // 0x7FF    = 0b00000000'00000000'00000111'11111111
    static constexpr int kCodepointB3Max = 0x0000'FFFF; // 0xFFFF   = 0b00000000'00000000'11111111'11111111
    static constexpr int kCodepointB4Max = 0x0010'FFFF; // 0x10FFFF = 0b00000000'00010000'11111111'11111111

    template<int bits>
    struct bytes
    {
        static_assert(imp::kDependentFalse<bits>);
    };
    
    template<>
    struct bytes<8>
    {
        using type = char8_t;
    };
    
    template<>
    struct bytes<16>
    {
        using type = char16_t;
    };
    
    template<>
    struct bytes<32>
    {
        using type = char32_t;
    };
    
    template<int bits>
    using bytes_t = bytes<bits>::type;
    
    template<int bits>
    static constexpr bytes_t<bits> CH(auto c)
    {
        return static_cast<bytes_t<bits>>(c);
    }

#define UC(c) static_cast<unsigned char>(c)

    // TODO utf_error enum code

    static void encode_into(char32_t codepoint, char8_t quad[4], std::u8string& utf8, bool ignore_invalid_chars)
    {
        if (codepoint <= kCodepointB1Max)
        {
            // 1-byte (ASCII)
            quad[0] = CH<8>(codepoint);
            utf8.append(quad, 1);
        }
        else if (codepoint <= kCodepointB2Max)
        {
            // 2-byte
            quad[0] = CH<8>((codepoint >> 6) | kB2Erasure);
            quad[1] = CH<8>((codepoint & kContMask) | kContHead);
            utf8.append(quad, 2);
        }
        else if (codepoint <= kCodepointB3Max)
        {
            // 3-byte
            quad[0] = CH<8>((codepoint >> 12) | kB3Erasure);
            quad[1] = CH<8>(((codepoint >> 6) & kContMask) | kContHead);
            quad[2] = CH<8>((codepoint & kContMask) | kContHead);
            utf8.append(quad, 3);
        }
        else if (codepoint <= kCodepointB4Max)
        {
            // 4-byte
            quad[0] = CH<8>((codepoint >> 18) | kB4Erasure);
            quad[1] = CH<8>(((codepoint >> 12) & kContMask) | kContHead);
            quad[2] = CH<8>(((codepoint >> 6) & kContMask) | kContHead);
            quad[3] = CH<8>((codepoint & kContMask) | kContHead);
            utf8.append(quad, 4);
        }
        else if (!ignore_invalid_chars)
        {
            quad[0] = quad[1] = quad[2] = quad[3] = 0;
            throw std::exception("Invalid Unicode codepoint"); // TODO use imp::error ?
        }

        quad[0] = 0;
        quad[1] = 0;
        quad[2] = 0;
        quad[3] = 0;
    }

    std::u8string encode(const std::u16string& utf16, bool ignore_invalid_chars)
    {
        std::u8string utf8;
        char8_t quad[4]{ 0, 0, 0, 0 };
        char32_t codepoint = 0;

        for (size_t i = 0; i < utf16.size(); ++i)
        {
            codepoint = CH<32>(utf16[i]);

            if (codepoint >= kSurrHighOffset && codepoint <= kSurrHighMax)
            {
                // Surrogate pair (4-byte UTF-8)
                if (i + 1 >= utf16.size())
                {
                    if (!ignore_invalid_chars)
                        throw std::exception("Invalid UTF-16 sequence"); // TODO use imp::error ?

                    continue;
                }

                const char32_t low_surrogate = CH<32>(utf16[i + 1]);
                if (low_surrogate < kSurrLowOffset || low_surrogate > kSurrLowMax)
                {
                    if (!ignore_invalid_chars)
                        throw std::exception("Invalid UTF-16: unpaired high surrogate"); // TODO use imp::error ?
                
                    continue;
                }

                ++i;
                codepoint = ((codepoint - kSurrHighOffset) << kSurrShift)
                    + (low_surrogate - kSurrLowOffset) + kSurrOffset;
            }
            else if (codepoint >= kSurrLowOffset && codepoint <= kSurrLowMax)
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

            if ((byte & kB1Cap) == kB1Erasure) {
                // 1-byte (ASCII)
                character |= byte;
                i += 1;
            }
            else if ((byte & kB2Cap) == kB2Erasure) {
                // 2-byte
                if (i + 1 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB2Mask) << 6;
                character |= UC(utf8[i + 1]) & kContMask;
                i += 2;
            }
            else if ((byte & kB3Cap) == kB3Erasure) {
                // 3-byte
                if (i + 2 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB3Mask) << 12;
                character |= (UC(utf8[i + 1]) & kContMask) << 6;
                character |= UC(utf8[i + 2]) & kContMask;
                i += 3;
            }
            else if ((byte & kB4Cap) == kB4Erasure) {
                // 4-byte (surrogate pair)
                if (i + 3 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB4Mask) << 18;
                character |= (UC(utf8[i + 1]) & kContMask) << 12;
                character |= (UC(utf8[i + 2]) & kContMask) << 6;
                character |= UC(utf8[i + 3]) & kContMask;

                // Encode as UTF-16 surrogate pair
                character -= kSurrOffset;
                utf16.push_back(CH<16>((character >> kSurrShift) + kSurrHighOffset)); // high surrogate
                utf16.push_back(CH<16>((character & kSurrLowMask) + kSurrLowOffset)); // low surrogate
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

            if ((byte & kB1Cap) == kB1Erasure)
            {
                // 1-byte (ASCII)
                character |= CH<32>(byte);
                i += 1;
            }
            else if ((byte & kB2Cap) == kB2Erasure)
            {
                // 2-byte
                if (i + 1 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB2Mask) << 6;
                character |= UC(utf8[i + 1]) & 0x3F;
                i += 2;
            }
            else if ((byte & kB3Cap) == kB3Erasure)
            {
                // 3-byte
                if (i + 2 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB3Mask) << 12;
                character |= (UC(utf8[i + 1]) & kContMask) << 6;
                character |= UC(utf8[i + 2]) & kContMask;
                i += 3;
            }
            else if ((byte & kB4Cap) == kB4Erasure)
            {
                // 4-byte
                if (i + 3 >= utf8.size())
                    throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

                character |= (byte & kB4Mask) << 18;
                character |= (UC(utf8[i + 1]) & kContMask) << 12;
                character |= (UC(utf8[i + 2]) & kContMask) << 6;
                character |= UC(utf8[i + 3]) & kContMask;
                i += 4;
            }
            else
                throw std::exception("Invalid UTF-8 sequence"); // TODO use imp::error ?

            utf32.push_back(character);
        }

        return utf32;
    }

    std::u8string convert(const std::string_view str_)
    {
        return std::u8string(str_.begin(), str_.end());
    }

    std::string convert(const std::u8string& utf8)
    {
        return std::string(utf8.begin(), utf8.end());
    }

    string::string(const char8_t* str)
        : str_(str)
    {
    }

    string::string(std::u8string str)
        : str_(std::move(str))
    {
    }

    string::string(const std::string_view str)
        : str_(convert(str))
    {
    }

    string::string(const char* str)
        : str_(convert(str))
    {
    }

    string::string(const std::string& str)
        : str_(convert(str))
    {
    }

    string::string(const char16_t* str)
        : str_(encode(std::u16string(str)))
    {
    }

    string::string(const std::u16string& str)
        : str_(encode(str))
    {
    }

    string::string(const char32_t* str)
        : str_(encode(std::u32string(str)))
    {
    }

    string::string(const std::u32string& str)
        : str_(encode(str))
    {
    }

    string::string(const string::iterator& begin_, const string::iterator& end_)
    {
        if (&begin_.string_ == &end_.string_)
            str_ = begin_.string_.str_.substr(begin_.i_, end_.i_ - begin_.i_);
        else
            throw std::exception("iterators are not compatible."); // TODO use imp::error ?
    }

    string string::operator+(const string& rhs) const
    {
        return utf::string(str_ + rhs.str_);
    }

    string& string::operator+=(const string& rhs)
    {
        str_ += rhs.str_;
        return *this;
    }

    string string::operator*(size_t n) const
    {
        std::u8string temp;
        for (size_t i = 0; i < n; ++i)
            temp += str_;
        return string(std::move(temp));
    }

    string& string::operator*=(size_t n)
    {
        std::u8string temp;
        for (size_t i = 0; i < n; ++i)
            temp += str_;
        str_.swap(temp);
        return *this;
    }

    bool string::operator==(const string& other) const
    {
        return str_ == other.str_;
    }

    bool string::operator!=(const string& other) const
    {
        return str_ != other.str_;
    }

    size_t string::hash() const
    {
        return std::hash<std::u8string>{}(str_);
    }

    size_t string::size() const
    {
        return str_.size();
    }

    bool string::empty() const
    {
        return str_.empty();
    }

    void string::clear()
    {
        *this = "";
    }

    std::u8string& string::encoding()
    {
        return str_;
    }

    const std::u8string& string::encoding() const
    {
        return str_;
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
        if (codepoint <= kCodepointB1Max)
        {
            quad[0] = CH<8>(codepoint);
            str_.append(quad, 1);
        }
        else if (codepoint <= kCodepointB2Max)
        {
            quad[0] = CH<8>(kB2Erasure | (codepoint >> 6));
            quad[1] = CH<8>(kContHead | (codepoint & kContMask));
            str_.append(quad, 2);
        }
        else if (codepoint <= kCodepointB3Max)
        {
            quad[0] = CH<8>(kB3Erasure | (codepoint >> 12));
            quad[1] = CH<8>(kContHead | ((codepoint >> 6) & kContMask));
            quad[2] = CH<8>(kContHead | (codepoint & kContMask));
            str_.append(quad, 3);
        }
        else if (codepoint <= kCodepointB4Max)
        {
            quad[0] = CH<8>(kB4Erasure | (codepoint >> 18));
            quad[1] = CH<8>(kContHead | ((codepoint >> 12) & kContMask));
            quad[2] = CH<8>(kContHead | ((codepoint >> 6) & kContMask));
            quad[3] = CH<8>(kContHead | (codepoint & kContMask));
            str_.append(quad, 4);
        }
        else
            throw std::exception("codepoint is out of valid UTF-8 range"); // TODO use imp::error ?
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

    string::iterator::iterator(const string& string, size_t i)
        : string_(string)
        , i_(i)
    {
    }

    codepoint string::iterator::operator*() const
    {
        if (i_ >= string_.str_.size())
            throw std::out_of_range("End of string_"); // TODO use imp::error ?

        unsigned char first = UC(string_.str_[i_]);
        char32_t cdpt = 0;

        if (first < kB1Cap)
        {
            cdpt |= first;
        }
        else if (first < kB2Cap)
        {
            if (i_ + 1 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB2Mask) << 6;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask);
        }
        else if (first < kB3Cap)
        {
            if (i_ + 2 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB3Mask) << 12;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask) << 6;
            cdpt |= (UC(string_.str_[i_ + 2]) & kContMask);
        }
        else if (first < kB4Cap)
        {
            if (i_ + 3 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB4Mask) << 18;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask) << 12;
            cdpt |= (UC(string_.str_[i_ + 2]) & kContMask) << 6;
            cdpt |= (UC(string_.str_[i_ + 3]) & kContMask);
        }

        return codepoint(static_cast<int>(cdpt));
    }

    string::iterator& string::iterator::operator++()
    {
        i_ += num_bytes();
        return *this;
    }

    string::iterator string::iterator::operator++(int)
    {
        iterator iter(string_, i_);
        i_ += num_bytes();
        return iter;
    }

    string::iterator& string::iterator::operator--()
    {
        if (i_ == 0)
            throw std::out_of_range("Start of string"); // TODO use imp::error ?

        --i_;
        while ((UC(string_.str_[i_]) & kContCapture) == kContHead)
        {
            if (i_ == 0)
                throw std::exception("UTF-8 invalid starting byte"); // TODO use imp::error ?

            --i_;
        }
        return *this;
    }

    string::iterator string::iterator::operator--(int)
    {
        iterator it(string_, i_);
        --*this;
        return it;
    }

    bool string::iterator::operator==(const iterator& other) const
    {
        return &string_ == &other.string_ && i_ == other.i_;
    }

    bool string::iterator::operator!=(const iterator& other) const
    {
        return &string_ != &other.string_ || i_ != other.i_;
    }

    char string::iterator::num_bytes() const
    {
        unsigned char first = UC(string_.str_[i_]);
        if (first < kB1Cap)
            return 1;
        else if (first < kB2Cap)
            return 2;
        else if (first < kB3Cap)
            return 3;
        else if (first < kB4Cap)
            return 4;
        else
            throw std::exception("Invalid UTF-8"); // TODO use imp::error ?
    }

    string::iterator::operator bool() const
    {
        return i_ < string_.str_.size();
    }

    codepoint string::iterator::advance()
    {
        if (i_ >= string_.str_.size())
            throw std::out_of_range("End of string_"); // TODO use imp::error ?

        unsigned char first = UC(string_.str_[i_]);
        char32_t cdpt = 0;

        if (first < kB1Cap)
        {
            cdpt |= first;
            i_ += 1;
        }
        else if (first < kB2Cap)
        {
            if (i_ + 1 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB2Mask) << 6;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask);
            i_ += 2;
        }
        else if (first < kB3Cap)
        {
            if (i_ + 2 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB3Mask) << 12;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask) << 6;
            cdpt |= (UC(string_.str_[i_ + 2]) & kContMask);
            i_ += 3;
        }
        else if (first < kB4Cap)
        {
            if (i_ + 3 >= string_.str_.size())
                throw std::out_of_range("Invalid UTF-8"); // TODO use imp::error ?

            cdpt |= (first & kB4Mask) << 18;
            cdpt |= (UC(string_.str_[i_ + 1]) & kContMask) << 12;
            cdpt |= (UC(string_.str_[i_ + 2]) & kContMask) << 6;
            cdpt |= (UC(string_.str_[i_ + 3]) & kContMask);
            i_ += 4;
        }
        else
            throw std::exception("Invalid UTF-8"); // TODO use imp::error ?

        return codepoint(static_cast<int>(cdpt));
    }

    string::iterator string::begin() const
    {
        return iterator(*this, 0);
    }

    string::iterator string::end() const
    {
        return iterator(*this, str_.size());
    }

#undef UC
}
