#pragma once

#include "imp/hash.hpp"

#include <unordered_map>
#include <vector>

namespace imp
{
	template<typename ty>
	class internship
	{
		std::vector<ty> _data;

	public:
		using handle_type = size_t;

	private:
		std::unordered_multimap<size_t, handle_type> _lut;

	public:
		template<typename view_ty, typename view_hash = std::hash<view_ty>, typename view_equals = std::equal_to<view_ty>, typename converter = void>
		handle_type intern(const view_ty& view)
		{
			size_t hash = hasher().with<view_hash>(view);
			auto range = _lut.equal_range(hash);

			for (auto it = range.first; it != range.second; ++it)
			{
				auto handle = it->second;
				if (view_equals{}(_data[handle], view))
					return handle;
			}

			handle_type handle = _data.size();

			if constexpr (std::is_same_v<converter, void>)
				_data.emplace_back(view);
			else
				_data.emplace_back(converter{}(view));

			_lut.emplace(hash, handle);
			return handle;
		}

		const ty& get(const handle_type handle) const
		{
			return _data[handle];
		}
	};
}
