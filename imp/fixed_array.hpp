#pragma once

#include "imp/dynamic_array.h"

#include <string>

namespace imp
{
	template<typename ty>
	class fixed_array
	{
		const size_t _length = 0;
		ty* _ptr = nullptr;

	public:
		// TODO imp error ?
		struct length_mismatch_error : public std::runtime_error
		{
			size_t expected_length, given_length;

			length_mismatch_error(size_t expected_length, size_t given_length)
				: expected_length(expected_length), given_length(given_length), std::runtime_error("provided length " + std::to_string(given_length) + " does not match array length " + std::to_string(expected_length))
			{
			}
		};

		// TODO imp error ?
		struct out_of_range_error : public std::runtime_error
		{
			size_t length, index;

			out_of_range_error(size_t length, size_t index)
				: index(index), length(length), std::runtime_error("index " + std::to_string(index) + " out of range for array length " + std::to_string(length))
			{
			}
		};

		explicit fixed_array(size_t length)
			: _length(length), _ptr(new ty[_length])
		{
		}

		fixed_array(size_t length, const ty& value)
			: _length(length), _ptr(new ty[_length])
		{
			for (size_t i = 0; i < length; ++i)
				_ptr[i] = value;
		}

		fixed_array(ty* raw_array, size_t length)
			: _length(length), _ptr(raw_array)
		{
		}

		template<resize_strategy resize_strategy>
		fixed_array(dynamic_array<ty, resize_strategy>&& array)
			: _length(array.size())
		{
			size_t sz = _length;
			_ptr = array.release(sz);

			if (_length == 0 || sz != _length)
			{
				delete[] _ptr;
				_ptr = nullptr;
			}

			if (sz != _length)
				throw length_mismatch_error(_length, sz);
		}

		fixed_array(std::initializer_list<ty> init)
			: _length(init.size()), _ptr(new ty[_length])
		{
			size_t i = 0;
			for (const ty& obj : init)
				_ptr[i++] = obj;
		}

		fixed_array(const fixed_array& o)
			: _length(o._length), _ptr(new ty[o._length])
		{
			for (size_t i = 0; i < _length; ++i)
				_ptr[i] = o._ptr[i];
		}

		fixed_array(fixed_array&& o) noexcept
			: _length(o._length), _ptr(o._ptr)
		{
			o._ptr = nullptr;
		}

		~fixed_array()
		{
			delete[] _ptr;
		}

		fixed_array& operator=(const fixed_array& o)
		{
			if (this != &o)
			{
				if (_length == o._length)
				{
					for (size_t i = 0; i < _length; ++i)
						_ptr[i] = o._ptr[i];
				}
				else
					throw length_mismatch_error(_length, o._length);
			}

			return *this;
		}

		fixed_array& operator=(fixed_array&& o)
		{
			if (this != &o)
			{
				if (_length == o._length)
				{
					delete[] _ptr;
					_ptr = o._ptr;
					o._ptr = nullptr;
				}
				else
					throw length_mismatch_error(_length, o._length);
			}

			return *this;
		}

		const ty* data() const noexcept
		{
			return _ptr;
		}

		ty* data() noexcept
		{
			return _ptr;
		}

		size_t length() const noexcept
		{
			return _length;
		}

		const ty& operator[](size_t i) const
		{
			if (i < _length)
				return _ptr[i];
			else
				throw out_of_range_error(_length, i);
		}

		ty& operator[](size_t i)
		{
			if (i < _length)
				return _ptr[i];
			else
				throw out_of_range_error(_length, i);
		}

		ty* begin() noexcept
		{
			return _ptr;
		}

		ty* end() noexcept
		{
			return _ptr + _length;
		}

		const ty* begin() const noexcept
		{
			return _ptr;
		}

		const ty* end() const noexcept
		{
			return _ptr + _length;
		}

		const ty* cbegin() const noexcept
		{
			return _ptr;
		}

		const ty* cend() const noexcept
		{
			return _ptr + _length;
		}

		operator std::span<ty>() noexcept
		{
			return std::span<ty>(_ptr, _length);
		}

		operator std::span<const ty>() const noexcept
		{
			return std::span<const ty>(_ptr, _length);
		}
	};
}
