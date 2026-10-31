#pragma once

#include "imp/dynamic_array.hpp"

namespace imp
{
    template<typename ty>
    class fixed_array
    {
    public:
        // TODO imp error ?
        struct length_mismatch_error
            : public std::runtime_error
        {
            size_t expected_length;
            size_t given_length;

            length_mismatch_error(size_t expected_length, size_t given_length)
                : expected_length(expected_length)
                , given_length(given_length)
                , std::runtime_error(
                    "provided length " + std::to_string(given_length)
                    + " does not match array length " + std::to_string(expected_length))
            {
            }
        };

        // TODO imp error ?
        struct out_of_range_error
            : public std::runtime_error
        {
            size_t length;
            size_t index;

            out_of_range_error(size_t length, size_t index)
                : index(index)
                , length(length)
                , std::runtime_error(
                    "index " + std::to_string(index) + " out of range for array length " + std::to_string(length))
            {
            }
        };

    private:
        const size_t length_ = 0;
        ty* ptr_ = nullptr;

    public:
        explicit fixed_array(size_t length)
            : length_(length)
            , ptr_(new ty[length_])
        {
        }

        fixed_array(size_t length, const ty& value)
            : length_(length)
            , ptr_(new ty[length_])
        {
            for (size_t i = 0; i < length; ++i)
                ptr_[i] = value;
        }

        fixed_array(ty* raw_array, size_t length)
            : length_(length)
            , ptr_(raw_array)
        {
        }

        template<resize_strategy resize_strategy>
        fixed_array(dynamic_array<ty, resize_strategy>&& array)
            : length_(array.size())
        {
            size_t sz = length_;
            ptr_ = array.release(sz);

            if (length_ == 0 || sz != length_)
            {
                delete[] ptr_;
                ptr_ = nullptr;
            }

            if (sz != length_)
                throw length_mismatch_error(length_, sz);
        }

        fixed_array(std::initializer_list<ty> init)
            : length_(init.size())
            , ptr_(new ty[length_])
        {
            size_t i = 0;
            for (const ty& obj : init)
                ptr_[i++] = obj;
        }

        fixed_array(const fixed_array& o)
            : length_(o.length_)
            , ptr_(new ty[o.length_])
        {
            for (size_t i = 0; i < length_; ++i)
                ptr_[i] = o.ptr_[i];
        }

        fixed_array(fixed_array&& o) noexcept
            : length_(o.length_)
            , ptr_(o.ptr_)
        {
            o.ptr_ = nullptr;
        }

        ~fixed_array()
        {
            delete[] ptr_;
        }

        fixed_array& operator=(const fixed_array& o)
        {
            if (this != &o)
            {
                if (length_ == o.length_)
                {
                    for (size_t i = 0; i < length_; ++i)
                        ptr_[i] = o.ptr_[i];
                }
                else
                    throw length_mismatch_error(length_, o.length_);
            }

            return *this;
        }

        fixed_array& operator=(fixed_array&& o)
        {
            if (this != &o)
            {
                if (length_ == o.length_)
                {
                    delete[] ptr_;
                    ptr_ = o.ptr_;
                    o.ptr_ = nullptr;
                }
                else
                    throw length_mismatch_error(length_, o.length_);
            }

            return *this;
        }

        const ty* data() const noexcept
        {
            return ptr_;
        }

        ty* data() noexcept
        {
            return ptr_;
        }

        size_t length() const noexcept
        {
            return length_;
        }

        const ty& operator[](size_t i) const
        {
            if (i < length_)
                return ptr_[i];
            else
                throw out_of_range_error(length_, i);
        }

        ty& operator[](size_t i)
        {
            if (i < length_)
                return ptr_[i];
            else
                throw out_of_range_error(length_, i);
        }

        operator std::span<ty>() noexcept
        {
            return std::span<ty>(ptr_, length_);
        }

        operator std::span<const ty>() const noexcept
        {
            return std::span<const ty>(ptr_, length_);
        }

        class iterator
        {
            friend class fixed_array<ty>;

        private:
            fixed_array<ty>& arr_;
            size_t idx_ = 0;

        private:
            iterator(fixed_array<ty>& vec, size_t idx)
                : arr_(vec)
                , idx_(idx)
            {
            }

        public:
            bool operator==(const iterator& o) const
            {
                return &arr_ == &o.arr_ && idx_ == o.idx_;
            }

            bool operator!=(const iterator& o) const
            {
                return &arr_ != &o.arr_ || idx_ != o.idx_;
            }
            
            const ty& operator*() const
            {
                return arr_[idx()];
            }
            
            ty& operator*()
            {
                return arr_[idx()];
            }
            
            const ty* operator->() const
            {
                return &arr_[idx()];
            }
            
            ty* operator->()
            {
                return &arr_[idx()];
            }

            iterator& operator++()
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                ++idx_;
                return *this;
            }

            iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                return idx_;
            }
        };

        class const_iterator
        {
            friend class fixed_array<ty>;

        private:
            const fixed_array<ty>& arr_;
            size_t idx_ = 0;

        private:
            const_iterator(const fixed_array<ty>& vec, size_t idx)
                : arr_(vec)
                , idx_(idx)
            {
            }

        public:
            bool operator==(const const_iterator& o) const
            {
                return &arr_ == &o.arr_ && idx_ == o.idx_;
            }

            bool operator!=(const const_iterator& o) const
            {
                return &arr_ != &o.arr_ || idx_ != o.idx_;
            }

            const ty& operator*() const
            {
                return arr_[idx()];
            }
            
            const ty* operator->() const
            {
                return &arr_[idx()];
            }

            const_iterator& operator++()
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                ++idx_;
                return *this;
            }

            const_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                const_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                return idx_;
            }
        };

        class reverse_iterator
        {
            friend class fixed_array<ty>;

        private:
            fixed_array<ty>& arr_;
            size_t idx_ = 0;

        private:
            reverse_iterator(fixed_array<ty>& vec, size_t idx)
                : arr_(vec)
                , idx_(idx)
            {
            }

        public:
            bool operator==(const reverse_iterator& o) const
            {
                return &arr_ == &o.arr_ && idx_ == o.idx_;
            }
            
            bool operator!=(const reverse_iterator& o) const
            {
                return &arr_ != &o.arr_ || idx_ != o.idx_;
            }

            const ty& operator*() const
            {
                return arr_[idx()];
            }
            
            ty& operator*()
            {
                return arr_[idx()];
            }
            
            const ty* operator->() const
            {
                return &arr_[idx()];
            }
            
            ty* operator->()
            {
                return &arr_[idx()];
            }

            reverse_iterator& operator++()
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                ++idx_;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                reverse_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                return arr_.size() - 1 - idx_;
            }
        };

        class const_reverse_iterator
        {
            friend class fixed_array<ty>;

        private:
            const fixed_array<ty>& arr_;
            size_t idx_ = 0;

        private:
            const_reverse_iterator(const fixed_array<ty>& vec, size_t idx)
                : arr_(vec)
                , idx_(idx)
            {
            }

        public:
            bool operator==(const const_reverse_iterator& o) const
            {
                return &arr_ == &o.arr_ && idx_ == o.idx_;
            }
            
            bool operator!=(const const_reverse_iterator& o) const
            {
                return &arr_ != &o.arr_ || idx_ != o.idx_;
            }

            const ty& operator*() const
            {
                return arr_[idx()];
            }
            
            const ty* operator->() const
            {
                return &arr_[idx()];
            }

            const_reverse_iterator& operator++()
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                ++idx_;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                const_reverse_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw out_of_range_error(arr_.length_, idx_);

                return arr_.size() - 1 - idx_;
            }
        };

        iterator begin()
        {
            return iterator(*this, 0);
        }

        iterator end()
        {
            return iterator(*this, length_);
        }

        const_iterator begin() const
        {
            return const_iterator(*this, 0);
        }
        
        const_iterator end() const
        {
            return const_iterator(*this, length_);
        }
        
        const_iterator cbegin() const
        {
            return const_iterator(*this, 0);
        }
        
        const_iterator cend() const
        {
            return const_iterator(*this, length_);
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(*this, 0);
        }

        reverse_iterator rend()
        {
            return reverse_iterator(*this, length_);
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(*this, 0);
        }
        
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(*this, length_);
        }
        
        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(*this, 0);
        }
        
        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(*this, length_);
        }
    };
}
