#pragma once

#include <typeindex>

namespace imp
{
	using type_erasure = size_t;

	extern type_erasure erase_type(std::type_index ti);

	template<typename ty>
	type_erasure erase_type()
	{
		return erase_type(typeid(ty));
	}

	template<typename ty>
	ty* resolve_type(type_erasure erasure, void* ptr)
	{
		if (erase_type<ty>() == erasure)
			return static_cast<ty*>(ptr);
		else
			return nullptr;
	}

	template<typename ty>
	const ty* resolve_type(type_erasure erasure, const void* ptr)
	{
		if (erase_type<ty>() == erasure)
			return static_cast<const ty*>(ptr);
		else
			return nullptr;
	}
}
