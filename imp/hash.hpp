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
			return with_hash<std::hash<ty>>(o);
		}

		template<typename hash, typename ty>
		hasher& with_hash(const ty& o)
		{
			hash_combine(h, hash{}(o));
			return *this;
		}

		operator size_t () const
		{
			return h;
		}
	};

	template<typename hash>
	struct stl_hash
	{
		template<typename ty, size_t n>
		size_t operator()(const std::array<ty, n>& a) const noexcept
		{
			hasher h;
			for (size_t i = 0; i < n; ++i)
				h.with_hash<hash>(a[i]);
			return h;
		}

        template<typename ty>
        size_t operator()(const std::pair<ty, ty>& p) const noexcept
        {
            return hasher().with_hash<hash>(p.first).with_hash<hash>(p.second);
        }
	};
}
