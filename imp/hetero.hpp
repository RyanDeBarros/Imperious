#pragma once

#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace imp::hetero
{
	struct string_hash
	{
		using is_transparent = void;

        size_t operator()(std::string_view s) const noexcept
        {
            return std::hash<std::string_view>{}(s);
        }

        size_t operator()(std::span<const char> s) const noexcept
        {
            return (*this)(std::string_view{ s.data(), s.size() });
        }
	};

	struct string_equal
	{
        using is_transparent = void;

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
        {
            return lhs == rhs;
        }

        bool operator()(std::span<const char> lhs, std::string_view rhs) const noexcept
        {
            return std::string_view{ lhs.data(), lhs.size() } == rhs;
        }

        bool operator()(std::string_view lhs, std::span<const char> rhs) const noexcept
        {
            return lhs == std::string_view{ rhs.data(), rhs.size() };
        }

        bool operator()(std::span<const char> lhs, std::span<const char> rhs) const noexcept
        {
            return std::string_view{ lhs.data(), lhs.size() } == std::string_view{ rhs.data(), rhs.size() };
        }
	};

    struct string_compare
    {
        using is_transparent = void;

        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
        {
            return lhs < rhs;
        }

        bool operator()(std::span<const char> lhs, std::string_view rhs) const noexcept
        {
            return std::string_view{ lhs.data(), lhs.size() } < rhs;
        }

        bool operator()(std::string_view lhs, std::span<const char> rhs) const noexcept
        {
            return lhs < std::string_view{ rhs.data(), rhs.size() };
        }

        bool operator()(std::span<const char> lhs, std::span<const char> rhs) const noexcept
        {
            return std::string_view{ lhs.data(), lhs.size() } < std::string_view{ rhs.data(), rhs.size() };
        }
    };

	template<typename value>
	using string_umap = std::unordered_map<std::string, value, string_hash, string_equal>;
	
	template<typename value>
	using string_map = std::map<std::string, value, string_compare>;

	using string_uset = std::unordered_set<std::string, string_hash, string_equal>;

	using string_set = std::set<std::string, string_compare>;
}
