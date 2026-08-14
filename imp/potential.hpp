#pragma once

#include <ostream>

namespace imp
{
	template<typename ty>
	struct potential
	{
		bool has_value;
		ty value;

		constexpr explicit potential()
			: has_value(false)
		{
		}

		constexpr explicit potential(ty value)
			: has_value(true), value(std::move(value))
		{
		}

		bool operator==(const potential<ty>& o) const
		{
			if (has_value)
				return o.has_value && value == o.value;
			else
				return !o.has_value;
		}

		bool operator!=(const potential<ty>& o) const
		{
			if (has_value)
				return !o.has_value || value != o.value;
			else
				return o.has_value;
		}
	};

	template<typename ty>
	std::ostream& operator<<(std::ostream& os, const potential<ty>& opt)
	{
		os << "imp::potential[has_value=" << opt.has_value;

		if (opt.has_value)
			os << ", value=" << opt.value << "]";
		else
			os << "]";

		return os;
	}
}
