#pragma once

#include <concepts>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace imp::hetero
{
    template<typename ty>
    concept string_like =
        std::same_as<std::remove_cvref_t<ty>, std::string> ||
        std::same_as<std::remove_cvref_t<ty>, std::string_view> ||
        std::same_as<std::remove_cvref_t<ty>, std::span<const char>>;

    constexpr std::string_view to_string_view(std::string_view value) noexcept
    {
        return value;
    }

    constexpr std::string_view to_string_view(const std::string& value) noexcept
    {
        return value;
    }

    constexpr std::string_view to_string_view(std::span<const char> value) noexcept
    {
        return { value.data(), value.size() };
    }

	struct string_hash
	{
		using is_transparent = void;

        template<string_like ty>
        size_t operator()(const ty& value) const noexcept
        {
            return std::hash<std::string_view>{}(to_string_view(value));
        }
	};

	struct string_equal
	{
        using is_transparent = void;

        template<string_like lty, string_like rty>
        bool operator()(const lty& lhs, const rty& rhs) const noexcept
        {
            return to_string_view(lhs) == to_string_view(rhs);
        }
	};

    struct string_compare
    {
        using is_transparent = void;

        template<string_like lty, string_like rty>
        bool operator()(const lty& lhs, const rty& rhs) const noexcept
        {
            return to_string_view(lhs) < to_string_view(rhs);
        }
    };

	template<typename value>
	using string_umap = std::unordered_map<std::string, value, string_hash, string_equal>;
	
	template<typename value>
	using string_map = std::map<std::string, value, string_compare>;

	using string_uset = std::unordered_set<std::string, string_hash, string_equal>;

	using string_set = std::set<std::string, string_compare>;
}
