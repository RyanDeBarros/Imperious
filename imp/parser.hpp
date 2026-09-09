#pragma once

#include <optional>
#include <string_view>

namespace imp
{
	extern std::optional<int> stoi_direct(const std::string_view str, const int base);
	extern std::optional<int> stoi(const std::string_view str);
	extern std::optional<int> stocdpt(const std::string_view str);

	struct cdpt_hash
	{
		size_t operator()(const std::string_view value) const noexcept;
	};

	struct cdpt_equal
	{
		bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept;
	};

	struct cdpt_less
	{
		bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept;
	};
}
