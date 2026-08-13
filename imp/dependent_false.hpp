#pragma once

namespace imp
{
	template<auto>
	inline constexpr bool dependent_false_v = false;
}
