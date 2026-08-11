#pragma once

#include <array>

namespace imp
{
	struct hasher
	{
		size_t h = 0;

		static void hash_combine(size_t& hash, size_t with);

		template<typename ty>
		hasher& with(const ty& o)
		{
			return with<std::hash<ty>>(o);
		}

		template<typename Hash, typename ty>
		hasher& with(const ty& o)
		{
			hash_combine(h, Hash{}(o));
			return *this;
		}

		operator size_t () const
		{
			return h;
		}
	};

	template<typename ty, typename hash = std::hash<ty>>
	struct stl_hash
	{
		template<size_t n>
		size_t operator()(const std::array<ty, n>& a) const
		{
			hasher h;
			for (size_t i = 0; i < N; ++i)
				h.with<hash>(a[i]);
			return h;
		}
	};
}
