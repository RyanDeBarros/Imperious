#pragma once

#include "imp/hash.hpp"

#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace imp
{
	template<typename ty>
	class internship
	{
		std::vector<ty> _data;

	public:
		struct handle
		{
			size_t id = 0;

			handle() = default;
			explicit handle_handletype(size_t id) : id(id) {}
		};

	private:
		std::unordered_multimap<size_t, handle_type> _lut;

	public:
		template<typename view_ty, typename view_hash = std::hash<view_ty>, typename view_equals = std::equal_to<view_ty>, typename converter = void>
		handle intern(const view_ty& view)
		{
			size_t hash = hasher().with<view_hash>(view);
			auto range = _lut.equal_range(hash);

			for (auto it = range.first; it != range.second; ++it)
			{
				auto handle_ = it->second;
				if (view_equals{}(get(handle_), view))
					return handle_;
			}

			handle handle_ = handle_type(_data.size() + 1);

			if constexpr (std::is_same_v<converter, void>)
				_data.emplace_back(view);
			else
				_data.emplace_back(converter{}(view));

			_lut.emplace(hash, handle_);
			return handle_;
		}

		const ty& get(const handle handle_) const
		{
			if (auto ptr = try_get(handle_))
				return *ptr;
			else
				throw std::logic_error("Invalid handle"); // TODO imp::error ?
		}

		const ty* try_get(const handle handle_) const
		{
			if (handle_ > 0 && handle_ <= _data.size())
				return _data.data() + handle_ - 1;
			else
				return nullptr;
		}
	};
}
