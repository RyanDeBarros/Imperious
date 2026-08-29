#pragma once

#include <span>
#include <stdexcept>
#include <string>

namespace imp
{
	struct resize_strategy
	{
		float mult = 1.5f;
		size_t increment = 1;

		constexpr bool valid() const
		{
			return mult >= 1.f && increment > 0;
		}

		constexpr size_t new_capacity(size_t capacity) const
		{
			return static_cast<size_t>(capacity * mult) + increment;
		}
	};

	template<typename ty, resize_strategy resize_strategy = resize_strategy()>
	class dynamic_array
	{
		static_assert(resize_strategy.valid(), "invalid resize strategy");

		size_t _size = 0;
		size_t _capacity = 0;
		ty* _arr = nullptr;

	public:
		// TODO imp error ?
		struct out_of_range_error : public std::runtime_error
		{
			size_t size, index;

			out_of_range_error(size_t size, size_t index)
				: index(index), size(size), std::runtime_error("index " + std::to_string(index) + " out of range for array size " + std::to_string(size))
			{
			}
		};

		dynamic_array() = default;

		dynamic_array(const dynamic_array& o)
			: _size(o._size), _capacity(o._capacity), _arr(new ty[_capacity])
		{
			for (size_t i = 0; i < _size; ++i)
				_arr[i] = o._arr[i];
		}

		dynamic_array(dynamic_array&& o) noexcept
			: _size(o._size), _capacity(o._capacity), _arr(o._arr)
		{
			o._arr = nullptr;
			o._capacity = 0;
			o._size = 0;
		}

		~dynamic_array()
		{
			delete[] _arr;
		}

		dynamic_array& operator=(const dynamic_array& o)
		{
			if (this != &o)
			{
				delete[] _arr;
				_size = o._size;
				_capacity = o._capacity;
				_arr = new ty[_capacity];
				for (size_t i = 0; i < _size; ++i)
					_arr[i] = o._arr[i];
			}

			return *this;
		}

		dynamic_array& operator=(dynamic_array&& o) noexcept
		{
			if (this != &o)
			{
				delete[] _arr;
				_size = o._size;
				_capacity = o._capacity;
				_arr = o._arr;
				o._arr = nullptr;
				o._capacity = 0;
				o._size = 0;
			}

			return *this;
		}

		const ty& operator[](size_t i) const
		{
			if (i < _size)
				return _arr[i];
			else
				throw out_of_range_error(_size, i);
		}

		ty& operator[](size_t i)
		{
			if (i < _size)
				return _arr[i];
			else
				throw out_of_range_error(_size, i);
		}

		const ty* data() const noexcept
		{
			return _arr;
		}

		ty* data() noexcept
		{
			return _arr;
		}

		size_t size() const noexcept
		{
			return _size;
		}

		size_t capacity() const noexcept
		{
			return _capacity;
		}

		ty* begin() noexcept
		{
			return _arr;
		}

		ty* end() noexcept
		{
			return _arr + _size;
		}

		const ty* begin() const noexcept
		{
			return _arr;
		}

		const ty* end() const noexcept
		{
			return _arr + _size;
		}

		const ty* cbegin() const noexcept
		{
			return _arr;
		}

		const ty* cend() const noexcept
		{
			return _arr + _size;
		}

		operator std::span<ty>() noexcept
		{
			return std::span<ty>(_arr, _size);
		}

		operator std::span<const ty>() const noexcept
		{
			return std::span<const ty>(_arr, _size);
		}

		void push_back(ty obj)
		{
			if (_size == _capacity)
				reallocate();

			_arr[_size++] = std::move(obj);
		}

		void insert(size_t pos, ty obj)
		{
			if (pos > _size)
				throw out_of_range_error(_size, pos);

			if (_size == _capacity)
				reallocate();

			for (long i = _size - 1; i >= pos; --i)
				_arr[i + 1] = std::move(_arr[i]);
			_arr[pos] = std::move(obj);
			++_size;
		}

		void erase(size_t pos)
		{
			if (pos >= _size)
				throw out_of_range_error(_size, pos);

			for (long i = pos; i < _size - 1; ++i)
				_arr[i] = std::move(_arr[i + 1]);
			--_size;
		}

		void clear()
		{
			delete[] _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
		}

		ty* release(size_t& out_size)
		{
			if (_size == 0)
			{
				out_size = 0;
				return nullptr;
			}

			if (_size != _capacity)
			{
				ty* new_arr = new ty[_size];
				for (size_t i = 0; i < _size; ++i)
					new_arr[i] = std::move(_arr[i]);

				delete[] _arr;
				_arr = new_arr;
				_capacity = _size;
			}

			out_size = _size;
			ty* ptr = _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
			return ptr;
		}

		ty* release(size_t& out_size, size_t& out_capacity)
		{
			out_size = _size;
			out_capacity = _capacity;
			ty* ptr = _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
			return ptr;
		}

	private:
		void reallocate()
		{
			reserve(resize_strategy.new_capacity(_capacity));
		}

	public:
		void reserve(size_t new_capacity)
		{
			if (new_capacity > _capacity)
			{
				ty* new_arr = new ty[new_capacity];
				for (size_t i = 0; i < _size; ++i)
					new_arr[i] = std::move(_arr[i]);

				delete[] _arr;
				_arr = new_arr;
				_capacity = new_capacity;
			}
		}
	};
}
