#pragma once

#include <stdexcept>

// TODO clearer distinction between fixed_array and fixed_vector names - only difference besides (de)allocation
// algorithm is behaviour of assignment operator - fixed_array must match sizes while fixed_vector doesn't.

namespace imp
{
    template<typename ty>
    class fixed_vector
    {
        size_t length_ = 0;
        ty* ptr_ = nullptr;

    public:
        fixed_vector(size_t size)
            : length_(size)
            , ptr_(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t i = 0; i < size; ++i)
            {
                try
                {
                    new (ptr_ + i) ty();
                }
                catch (...)
                {
                    for (size_t j = 0; j < i; ++j)
                        ptr_[j].~ty();

                    operator delete[](ptr_);
                    throw;
                }
            }
        }

        fixed_vector(size_t size, const ty& default_value)
            : length_(size)
            , ptr_(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t i = 0; i < size; ++i)
            {
                try
                {
                    new (ptr_ + i) ty(default_value);
                }
                catch (...)
                {
                    for (size_t j = 0; j < i; ++j)
                        ptr_[j].~ty();

                    operator delete[](ptr_);
                    throw;
                }
            }
        }

        template<typename... args_ty>
        fixed_vector(size_t size, const args_ty&... args)
            : length_(size)
            , ptr_(static_cast<ty*>(operator new[](size * sizeof(ty))))
        {
            for (size_t i = 0; i < size; ++i)
            {
                try
                {
                    new (ptr_ + i) ty(args...);
                }
                catch (...)
                {
                    for (size_t j = 0; j < i; ++j)
                        ptr_[j].~ty();

                    operator delete[](ptr_);
                    throw;
                }
            }
        }

        fixed_vector(const fixed_vector<ty>& o)
            : length_(o.length_)
            , ptr_(static_cast<ty*>(operator new[](o.length_ * sizeof(ty))))
        {
            for (size_t i = 0; i < length_; ++i)
            {
                try
                {
                    new (ptr_ + i) ty(o.ptr_[i]);
                }
                catch (...)
                {
                    for (size_t j = 0; j < i; ++j)
                        ptr_[j].~ty();

                    operator delete[](ptr_);
                    throw;
                }
            }
        }

        fixed_vector(fixed_vector<ty>&& o) noexcept
            : length_(o.length_)
            , ptr_(o.ptr_)
        {
            o.length_ = 0;
            o.ptr_ = nullptr;
        }

        ~fixed_vector()
        {
            for (size_t i = 0; i < length_; ++i)
                ptr_[i].~ty();

            operator delete[](ptr_);
        }

        fixed_vector<ty>& operator=(const fixed_vector<ty>& o)
        {
            if (this != &o)
            {
                for (size_t i = 0; i < length_; ++i)
                    ptr_[i].~ty();

                operator delete[](ptr_);

                length_ = o.length_;
                ptr_ = static_cast<ty*>(operator new[](o.length_ * sizeof(ty)));

                for (size_t i = 0; i < length_; ++i)
                {
                    try
                    {
                        new (ptr_ + i) ty(o.ptr_[i]);
                    }
                    catch (...)
                    {
                        for (size_t j = 0; j < i; ++j)
                            ptr_[j].~ty();

                        operator delete[](ptr_);
                        throw;
                    }
                }
            }
            return *this;
        }

        fixed_vector<ty>& operator=(fixed_vector<ty>&& o) noexcept
        {
            if (this != &o)
            {
                for (size_t i = 0; i < length_; ++i)
                    ptr_[i].~ty();

                operator delete[](ptr_);

                length_ = o.length_;
                ptr_ = o.ptr_;
                o.length_ = 0;
                o.ptr_ = nullptr;
            }
            return *this;
        }

        const ty& operator[](size_t i) const
        {
            if (i >= length_)
                throw std::out_of_range(""); // TODO imp error ?

            return ptr_[i];
        }

        ty& operator[](size_t i)
        {
            if (i >= length_)
                throw std::out_of_range(""); // TODO imp error ?

            return ptr_[i];
        }

        size_t size() const
        {
            return length_;
        }

        const ty* data() const
        {
            return ptr_;
        }

        ty* data()
        {
            return ptr_;
        }

        class iterator
        {
            friend class fixed_vector<ty>;

        private:
            fixed_vector<ty>& arr_;
            size_t idx_ = 0;

        private:
            iterator(fixed_vector<ty>& vec, size_t idx)
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
                    throw std::out_of_range(""); // TODO imp error ?

                ++idx_;
                return *this;
            }

            iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                return idx_;
            }
        };

        class const_iterator
        {
            friend class fixed_vector<ty>;

        private:
            const fixed_vector<ty>& arr_;
            size_t idx_ = 0;

        private:
            const_iterator(const fixed_vector<ty>& vec, size_t idx)
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
                    throw std::out_of_range(""); // TODO imp error ?

                ++idx_;
                return *this;
            }

            const_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                const_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                return idx_;
            }
        };

        class reverse_iterator
        {
            friend class fixed_vector<ty>;

        private:
            fixed_vector<ty>& arr_;
            size_t idx_ = 0;

        private:
            reverse_iterator(fixed_vector<ty>& vec, size_t idx)
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
                    throw std::out_of_range(""); // TODO imp error ?

                ++idx_;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                reverse_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                return arr_.size() - 1 - idx_;
            }
        };

        class const_reverse_iterator
        {
            friend class fixed_vector<ty>;

        private:
            const fixed_vector<ty>& arr_;
            size_t idx_ = 0;

        private:
            const_reverse_iterator(const fixed_vector<ty>& vec, size_t idx)
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
                    throw std::out_of_range(""); // TODO imp error ?

                ++idx_;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

                const_reverse_iterator copy = *this;
                ++idx_;
                return copy;
            }

        private:
            size_t idx() const
            {
                if (idx_ >= arr_.length_)
                    throw std::out_of_range(""); // TODO imp error ?

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
