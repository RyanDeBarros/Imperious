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
        int c_;

    public:
        constexpr codepoint()
            : c_(0)
        {
        }
        
        constexpr explicit codepoint(int c)
            : c_(c)
        {
        }
        
        constexpr operator int() const
        {
            return c_;
        }

        constexpr bool operator==(const codepoint& o) const
        {
            return c_ == o.c_;
        }
        
        constexpr bool operator==(int x) const
        {
            return c_ == x;
        }

        constexpr bool operator==(char x) const
        {
            return c_ == x;
        }
    };

    constexpr bool is_n_or_r(codepoint codepoint)
    {
        return codepoint == '\n' || codepoint == '\r';
    }

    constexpr bool is_rn(codepoint r, codepoint n)
    {
        return r == '\r' && n == '\n';
    }

    class string
    {
        struct iterator;

    private:
        std::u8string str_ = u8"";

    public:
        string() = default;

        string(const char8_t* str);
        string(std::u8string str);
        string(const std::string_view str);
        string(const char* str);
        string(const std::string& str);
        string(const char16_t* str);
        string(const std::u16string& str);
        string(const char32_t* str);
        string(const std::u32string& str);
        string(const iterator& begin, const iterator& end);

        string(const string&) = default;
        string(string&&) noexcept = default;
        string& operator=(const string&) = default;
        string& operator=(string&&) noexcept = default;

        string operator+(const string& rhs) const;
        string& operator+=(const string& rhs);
        string operator*(size_t n) const;
        string& operator*=(size_t n);
        bool operator==(const string& other) const;
        bool operator!=(const string& other) const;

        size_t hash() const;
        size_t size() const;
        bool empty() const;
        void clear();

        std::u8string& encoding();
        const std::u8string& encoding() const;
        
        std::string str() const;
        bool begins_with(const utf::string& other) const;

        void push_back(codepoint codepoint);

        string substr(size_t begin, size_t end) const;

        struct iterator
        {
            friend class string;

        private:
            const string& string_;
            size_t i_;

        public:
            iterator(const string& string, size_t i);
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

        iterator begin() const;
        iterator end() const;
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
