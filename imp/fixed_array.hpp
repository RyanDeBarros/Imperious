#pragma once

#include "imp/dynamic_array.hpp"

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

		operator std::span<ty>() noexcept
		{
			return std::span<ty>(_ptr, _length);
		}

		operator std::span<const ty>() const noexcept
		{
			return std::span<const ty>(_ptr, _length);
		}

        class iterator
        {
            friend class fixed_array<ty>;
            fixed_array<ty>& _arr;
            size_t _idx = 0;

            iterator(fixed_array<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                return _idx;
            }

        public:
            bool operator==(const iterator& other) const { return &_arr == &other._arr && _idx == other._idx; }
            bool operator!=(const iterator&) const = default;

            const ty& operator*() const { return _arr[idx()]; }
            ty& operator*() { return _arr[idx()]; }
            const ty* operator->() const { return &_arr[idx()]; }
            ty* operator->() { return &_arr[idx()]; }

            iterator& operator++()
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                ++_idx;
                return *this;
            }

            iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class const_iterator
        {
            friend class fixed_array<ty>;
            const fixed_array<ty>& _arr;
            size_t _idx = 0;

            const_iterator(const fixed_array<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                return _idx;
            }

        public:
            bool operator==(const const_iterator& other) const { return &_arr == &other._arr && _idx == other._idx; }
            bool operator!=(const const_iterator&) const = default;

            const ty& operator*() const { return _arr[idx()]; }
            const ty* operator->() const { return &_arr[idx()]; }

            const_iterator& operator++()
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                ++_idx;
                return *this;
            }

            const_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                const_iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class reverse_iterator
        {
            friend class fixed_array<ty>;
            fixed_array<ty>& _arr;
            size_t _idx = 0;

            reverse_iterator(fixed_array<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                return _arr.size() - 1 - _idx;
            }

        public:
            bool operator==(const reverse_iterator& other) const { return &_arr == &other._arr && _idx == other._idx; }
            bool operator!=(const reverse_iterator&) const = default;

            const ty& operator*() const { return _arr[idx()]; }
            ty& operator*() { return _arr[idx()]; }
            const ty* operator->() const { return &_arr[idx()]; }
            ty* operator->() { return &_arr[idx()]; }

            reverse_iterator& operator++()
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                ++_idx;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                reverse_iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class const_reverse_iterator
        {
            friend class fixed_array<ty>;
            const fixed_array<ty>& _arr;
            size_t _idx = 0;

            const_reverse_iterator(const fixed_array<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                return _arr.size() - 1 - _idx;
            }

        public:
            bool operator==(const const_reverse_iterator& other) const { return &_arr == &other._arr && _idx == other._idx; }
            bool operator!=(const const_reverse_iterator&) const = default;

            const ty& operator*() const { return _arr[idx()]; }
            const ty* operator->() const { return &_arr[idx()]; }

            const_reverse_iterator& operator++()
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                ++_idx;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw out_of_range_error(_arr._length, _idx);

                const_reverse_iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        iterator begin() { return iterator(*this, 0); }
        iterator end() { return iterator(*this, _length); }

        const_iterator begin() const { return const_iterator(*this, 0); }
        const_iterator end() const { return const_iterator(*this, _length); }
        const_iterator cbegin() const { return const_iterator(*this, 0); }
        const_iterator cend() const { return const_iterator(*this, _length); }

        reverse_iterator rbegin() { return reverse_iterator(*this, 0); }
        reverse_iterator rend() { return reverse_iterator(*this, _length); }

        const_reverse_iterator rbegin() const { return const_reverse_iterator(*this, 0); }
        const_reverse_iterator rend() const { return const_reverse_iterator(*this, _length); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(*this, 0); }
        const_reverse_iterator crend() const { return const_reverse_iterator(*this, _length); }
	};
}
