#pragma once

#include <stdexcept>

// TODO clearer distinction between fixed_array and fixed_vector names - only difference besides (de)allocation algorithm is behaviour of assignment operator - fixed_array must match sizes while fixed_vector doesn't.

namespace imp
{
    template<typename ty>
    class fixed_vector
    {
        size_t _length = 0;
        ty* _ptr = nullptr;

    public:
        fixed_vector(size_t size)
            : _length(size), _ptr(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t _idx = 0; _idx < size; ++_idx)
            {
                try
                {
                    new (_ptr + _idx) ty();
                }
                catch (...)
                {
                    for (size_t j = 0; j < _idx; ++j)
                        _ptr[j].~ty();

                    operator delete[](_ptr);
                    throw;
                }
            }
        }

        fixed_vector(size_t size, const ty& default_value)
            : _length(size), _ptr(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t _idx = 0; _idx < size; ++_idx)
            {
                try
                {
                    new (_ptr + _idx) ty(default_value);
                }
                catch (...)
                {
                    for (size_t j = 0; j < _idx; ++j)
                        _ptr[j].~ty();

                    operator delete[](_ptr);
                    throw;
                }
            }
        }

        template<typename... args>
        fixed_vector(size_t size, const args&... args_)
            : _length(size), _ptr(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t _idx = 0; _idx < size; ++_idx)
            {
                try
                {
                    new (_ptr + _idx) ty(args_...);
                }
                catch (...)
                {
                    for (size_t j = 0; j < _idx; ++j)
                        _ptr[j].~ty();

                    operator delete[](_ptr);
                    throw;
                }
            }
        }

        fixed_vector(const fixed_vector<ty>& other)
            : _length(other._length), _ptr(static_cast<ty*>(operator new[](other._length * sizeof(ty))))
        {
            for (size_t _idx = 0; _idx < _length; ++_idx)
            {
                try
                {
                    new (_ptr + _idx) ty(other._ptr[_idx]);
                }
                catch (...)
                {
                    for (size_t j = 0; j < _idx; ++j)
                        _ptr[j].~ty();

                    operator delete[](_ptr);
                    throw;
                }
            }
        }

        fixed_vector(fixed_vector<ty>&& other) noexcept
            : _length(other._length), _ptr(other._ptr)
        {
            other._length = 0;
            other._ptr = nullptr;
        }

        ~fixed_vector()
        {
            for (size_t _idx = 0; _idx < _length; ++_idx)
                _ptr[_idx].~ty();

            operator delete[](_ptr);
        }

        fixed_vector<ty>& operator=(const fixed_vector<ty>& other)
        {
            if (this != &other)
            {
                for (size_t _idx = 0; _idx < _length; ++_idx)
                    _ptr[_idx].~ty();

                operator delete[](_ptr);

                _length = other._length;
                _ptr = static_cast<ty*>(operator new[](other._length * sizeof(ty)));

                for (size_t _idx = 0; _idx < _length; ++_idx)
                {
                    try
                    {
                        new (_ptr + _idx) ty(other._ptr[_idx]);
                    }
                    catch (...)
                    {
                        for (size_t j = 0; j < _idx; ++j)
                            _ptr[j].~ty();

                        operator delete[](_ptr);
                        throw;
                    }
                }
            }
            return *this;
        }

        fixed_vector<ty>& operator=(fixed_vector<ty>&& other) noexcept
        {
            if (this != &other)
            {
                for (size_t _idx = 0; _idx < _length; ++_idx)
                    _ptr[_idx].~ty();

                operator delete[](_ptr);

                _length = other._length;
                _ptr = other._ptr;
                other._length = 0;
                other._ptr = nullptr;
            }
            return *this;
        }

        const ty& operator[](size_t _idx) const
        {
            if (_idx >= _length)
                throw std::out_of_range(""); // TODO imp error ?

            return _ptr[_idx];
        }

        ty& operator[](size_t _idx)
        {
            if (_idx >= _length)
                throw std::out_of_range(""); // TODO imp error ?

            return _ptr[_idx];
        }

        size_t size() const
        {
            return _length;
        }

        const ty* data() const
        {
            return _ptr;
        }

        ty* data()
        {
            return _ptr;
        }

        class iterator
        {
            friend class fixed_vector<ty>;
            fixed_vector<ty>& _arr;
            size_t _idx = 0;

            iterator(fixed_vector<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

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
                    throw std::out_of_range(""); // TODO imp error ?

                ++_idx;
                return *this;
            }

            iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

                iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class const_iterator
        {
            friend class fixed_vector<ty>;
            const fixed_vector<ty>& _arr;
            size_t _idx = 0;

            const_iterator(const fixed_vector<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

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
                    throw std::out_of_range(""); // TODO imp error ?

                ++_idx;
                return *this;
            }

            const_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

                const_iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class reverse_iterator
        {
            friend class fixed_vector<ty>;
            fixed_vector<ty>& _arr;
            size_t _idx = 0;

            reverse_iterator(fixed_vector<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

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
                    throw std::out_of_range(""); // TODO imp error ?

                ++_idx;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

                reverse_iterator copy = *this;
                ++_idx;
                return copy;
            }
        };

        class const_reverse_iterator
        {
            friend class fixed_vector<ty>;
            const fixed_vector<ty>& _arr;
            size_t _idx = 0;

            const_reverse_iterator(const fixed_vector<ty>& vec, size_t idx) : _arr(vec), _idx(idx) {}

            size_t idx() const
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

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
                    throw std::out_of_range(""); // TODO imp error ?

                ++_idx;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                if (_idx >= _arr._length)
                    throw std::out_of_range(""); // TODO imp error ?

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
