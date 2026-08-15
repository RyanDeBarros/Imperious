#pragma once

#include <array>
#include <ostream>
#include <utility>

namespace imp
{
	template<typename ty>
	struct potential
	{
		bool has_value;
		ty value;

		constexpr potential()
			: has_value(false), value()
		{
		}

		constexpr potential(ty value)
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

	struct nullpotential_t
	{
		template<typename ty>
		constexpr operator potential<ty>() const
		{
			return potential<ty>();
		}
	};

	inline constexpr nullpotential_t nullpotential{};

	// TODO handle min/max values in incorrect order
	
	template<typename ty>
	bool clamp(ty& data, imp::potential<ty> min, imp::potential<ty> max)
	{
		const bool below_min = min.has_value && data < min.value;
		const bool above_max = max.has_value && data > min.value;
		
		if (below_min)
			data = min.value;
		else if (above_max)
			data = max.value;

		return below_min || above_max;
	}

	template<typename ty>
	bool clamp(ty* data, const size_t count, imp::potential<ty> min, imp::potential<ty> max)
	{
		bool changed = false;
		for (size_t i = 0; i < count; ++i)
			changed |= clamp(data[i], min, max);
		return changed;
	}

	template<typename ty, size_t n>
	bool clamp(std::array<ty, n>& data, imp::potential<ty> min, imp::potential<ty> max)
	{
		bool changed = false;
		for (size_t i = 0; i < n; ++i)
			changed |= clamp(data[i], min, max);
		return changed;
	}

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
