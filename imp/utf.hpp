#pragma once

#include <string>

namespace imp::utf
{
    extern std::u8string encode(const std::u16string& utf16, bool ignore_invalid_chars = false);
    extern std::u16string decode_utf16(const std::u8string& utf8);
    extern std::u8string encode(const std::u32string& utf32, bool ignore_invalid_chars = false);
    extern std::u32string decode_utf32(const std::u8string& utf8);
    extern std::u8string convert(const std::string_view str);
    extern std::string convert(const std::u8string& utf8);

    class codepoint
    {
        int _c;

    public:
        constexpr codepoint() : _c(0) {}
        constexpr explicit codepoint(int c) : _c(c) {}
        operator int() const { return _c; }

        constexpr bool operator==(const codepoint&) const = default;
        constexpr bool operator==(int x) const { return _c == x; }
        constexpr bool operator==(char x) const { return _c == x; }
    };

    constexpr bool is_n_or_r(codepoint codepoint) { return codepoint == '\n' || codepoint == '\r'; }
    constexpr bool is_rn(codepoint r, codepoint n) { return r == '\r' && n == '\n'; }

    class string
    {
        friend class iterator;
        std::u8string _str = u8"";

    public:
        string() = default;

        string(const char8_t* str)         : _str(str) {}
        string(std::u8string str)          : _str(std::move(str)) {}
        string(const std::string_view str) : _str(convert(str)) {}
        string(const char* str)            : _str(convert(str)) {}
        string(const std::string& str)     : _str(convert(str)) {}
        string(const char16_t* str)        : _str(encode(std::u16string(str))) {}
        string(const std::u16string& str)  : _str(encode(str)) {}
        string(const char32_t* str)        : _str(encode(std::u32string(str))) {}
        string(const std::u32string& str)  : _str(encode(str)) {}

        string(const string&) = default;
        string(string&&) noexcept = default;
        string& operator=(const string&) = default;
        string& operator=(string&&) noexcept = default;

        struct iterator
        {
        private:
            friend class string;
            const string& _string;
            size_t _i;

        public:
            iterator(const string& string, size_t i) : _string(string), _i(i) {}
            iterator(const iterator&) = default;
            iterator(iterator&&) noexcept = default;
            iterator& operator=(const iterator&) = default;
            iterator& operator=(iterator&&) noexcept = default;

            codepoint operator*() const;
            iterator& operator++();
            iterator operator++(int);
            iterator& operator--();
            iterator operator--(int);
            bool operator==(const iterator& other) const;
            bool operator!=(const iterator& other) const;
            char num_bytes() const;
            operator bool() const;
            codepoint advance();
        };

        iterator begin() const { return iterator(*this, 0); }
        iterator end() const { return iterator(*this, _str.size()); }
        size_t size() const { return _str.size(); }
        bool empty() const { return _str.empty(); }
        void clear() { *this = ""; }
        std::u8string& encoding() { return _str; }
        const std::u8string& encoding() const { return _str; }
        std::string str() const;
        bool begins_with(const utf::string& other) const;

        void push_back(codepoint codepoint);

        string operator+(const string& rhs) const;
        string& operator+=(const string& rhs);
        string operator*(size_t n) const;
        string& operator*=(size_t n);
        bool operator==(const string& other) const { return _str == other._str; }
        bool operator!=(const string& other) const { return _str != other._str; }
        size_t hash() const { return std::hash<std::u8string>{}(_str); }

        string(const iterator& begin_, const iterator& end_);
        string substr(size_t begin_, size_t end_) const;
    };
}

template<>
struct std::hash<imp::utf::codepoint>
{
    size_t operator()(const imp::utf::codepoint& c) const
    {
        return std::hash<int>{}(c);
    }
};

template<>
struct std::hash<imp::utf::string>
{
    size_t operator()(const imp::utf::string& string) const
    {
        return string.hash();
    }
};
