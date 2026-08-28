#pragma once

#include <array>
#include <concepts>
#include <span>
#include <stdexcept>

namespace imp
{
	template<typename ty>
	class group
	{
		template<typename _>
		friend class group;

		ty* _data = nullptr;
		size_t _count = 0;

	public:
		group(ty* array, size_t count)
			: _data(array), _count(count)
		{
		}

		template<typename other_ty> requires std::convertible_to<other_ty*, ty*>
		group(std::span<other_ty> span)
			: _data(span.data()), _count(span.size())
		{
		}
		
		template<typename other_ty, size_t n> requires std::convertible_to<other_ty*, ty*>
		group(std::array<other_ty, n>& array)
			: _data(array.data()), _count(n)
		{
		}
		
		template<typename other_ty, size_t n> requires std::convertible_to<const other_ty*, ty*>
		group(const std::array<other_ty, n>& array)
			: _data(array.data()), _count(n)
		{
		}

		group<ty>& operator=(const group<const ty>& o)
		{
			if (_data != o._data)
			{
				if (_count != o._count)
					throw std::logic_error("imp::group mismatching size in operator=()"); // TODO imp error ?

				// TODO safety checks/workarounds if arrays overlap
				for (size_t i = 0; i < _count; ++i)
					_data[i] = o._data[i];
			}

			return *this;
		}

		operator group<const ty>() const
		{
			return group<const ty>(_data, _count);
		}

		ty* data() const
		{
			return _data;
		}

		size_t size() const
		{
			return _count;
		}

		bool operator==(const group<const ty>& o) const
		{
			if (_count != o._count)
				return false;

			for (size_t i = 0; i < _count; ++i)
			{
				if (_data[i] != o._data[i])
					return false;
			}

			return true;
		}

		bool operator!=(const group<const ty>& o) const
		{
			if (_count != o._count)
				return true;

			for (size_t i = 0; i < _count; ++i)
			{
				if (_data[i] != o._data[i])
					return true;
			}

			return false;
		}
	};
}
