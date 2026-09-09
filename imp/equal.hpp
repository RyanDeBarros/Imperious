#pragma once

#include <array>

namespace imp
{
	template<typename equal>
	struct stl_equal
	{
		template<typename ty, size_t n>
		bool operator()(const std::array<ty, n>& a, const std::array<ty, n>& b) const noexcept
		{
			for (size_t i = 0; i < n; ++i)
			{
				if (!equal{}(a[i], b[i]))
					return false;
			}

			return true;
		}
	};
}
