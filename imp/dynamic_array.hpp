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

    template<
        typename ty,
        resize_strategy resize_strategy = resize_strategy()
    >
    class dynamic_array
    {
        static_assert(resize_strategy.valid(), "invalid resize strategy");

    public:
        // TODO imp error ?
        struct out_of_range_error
            : public std::runtime_error
        {
            size_t size;
            size_t index;

            out_of_range_error(size_t size, size_t index)
                : index(index)
                , size(size)
                , std::runtime_error(
                    "index " + std::to_string(index) + " out of range for array size " + std::to_string(size))
            {
            }
        };

    private:
        size_t size_ = 0;
        size_t capacity_ = 0;
        ty* arr_ = nullptr;

    public:
        dynamic_array() = default;

        dynamic_array(const dynamic_array& o)
            : size_(o.size_)
            , capacity_(o.capacity_)
            , arr_(new ty[capacity_])
        {
            for (size_t i = 0; i < size_; ++i)
                arr_[i] = o.arr_[i];
        }

        dynamic_array(dynamic_array&& o) noexcept
            : size_(o.size_)
            , capacity_(o.capacity_)
            , arr_(o.arr_)
        {
            o.arr_ = nullptr;
            o.capacity_ = 0;
            o.size_ = 0;
        }

        ~dynamic_array()
        {
            delete[] arr_;
        }

        dynamic_array& operator=(const dynamic_array& o)
        {
            if (this != &o)
            {
                delete[] arr_;
                size_ = o.size_;
                capacity_ = o.capacity_;
                arr_ = new ty[capacity_];
                for (size_t i = 0; i < size_; ++i)
                    arr_[i] = o.arr_[i];
            }

            return *this;
        }

        dynamic_array& operator=(dynamic_array&& o) noexcept
        {
            if (this != &o)
            {
                delete[] arr_;
                size_ = o.size_;
                capacity_ = o.capacity_;
                arr_ = o.arr_;
                o.arr_ = nullptr;
                o.capacity_ = 0;
                o.size_ = 0;
            }

            return *this;
        }

        const ty& operator[](size_t i) const
        {
            if (i < size_)
                return arr_[i];
            else
                throw out_of_range_error(size_, i);
        }

        ty& operator[](size_t i)
        {
            if (i < size_)
                return arr_[i];
            else
                throw out_of_range_error(size_, i);
        }

        operator std::span<ty>() noexcept
        {
            return std::span<ty>(arr_, size_);
        }

        operator std::span<const ty>() const noexcept
        {
            return std::span<const ty>(arr_, size_);
        }

        const ty* data() const noexcept
        {
            return arr_;
        }

        ty* data() noexcept
        {
            return arr_;
        }

        size_t size() const noexcept
        {
            return size_;
        }

        size_t capacity() const noexcept
        {
            return capacity_;
        }

        ty* begin() noexcept
        {
            return arr_;
        }

        ty* end() noexcept
        {
            return arr_ + size_;
        }

        const ty* begin() const noexcept
        {
            return arr_;
        }

        const ty* end() const noexcept
        {
            return arr_ + size_;
        }

        const ty* cbegin() const noexcept
        {
            return arr_;
        }

        const ty* cend() const noexcept
        {
            return arr_ + size_;
        }

        void push_back(ty obj)
        {
            if (size_ == capacity_)
                reallocate();

            arr_[size_++] = std::move(obj);
        }

        void insert(size_t pos, ty obj)
        {
            if (pos > size_)
                throw out_of_range_error(size_, pos);

            if (size_ == capacity_)
                reallocate();

            for (long i = size_ - 1; i >= pos; --i)
                arr_[i + 1] = std::move(arr_[i]);

            arr_[pos] = std::move(obj);
            ++size_;
        }

        void erase(size_t pos)
        {
            if (pos >= size_)
                throw out_of_range_error(size_, pos);

            for (long i = pos; i < size_ - 1; ++i)
                arr_[i] = std::move(arr_[i + 1]);

            --size_;
        }

        void clear()
        {
            delete[] arr_;
            arr_ = nullptr;
            capacity_ = 0;
            size_ = 0;
        }

        ty* release(size_t& out_size)
        {
            if (size_ == 0)
            {
                out_size = 0;
                return nullptr;
            }

            if (size_ != capacity_)
            {
                ty* new_arr = new ty[size_];
                for (size_t i = 0; i < size_; ++i)
                    new_arr[i] = std::move(arr_[i]);

                delete[] arr_;
                arr_ = new_arr;
                capacity_ = size_;
            }

            out_size = size_;
            ty* ptr = arr_;
            arr_ = nullptr;
            capacity_ = 0;
            size_ = 0;
            return ptr;
        }

        ty* release(size_t& out_size, size_t& out_capacity)
        {
            out_size = size_;
            out_capacity = capacity_;
            ty* ptr = arr_;
            arr_ = nullptr;
            capacity_ = 0;
            size_ = 0;
            return ptr;
        }

    private:
        void reallocate()
        {
            reserve(resize_strategy.new_capacity(capacity_));
        }

    public:
        void reserve(size_t new_capacity)
        {
            if (new_capacity > capacity_)
            {
                ty* new_arr = new ty[new_capacity];
                for (size_t i = 0; i < size_; ++i)
                    new_arr[i] = std::move(arr_[i]);

                delete[] arr_;
                arr_ = new_arr;
                capacity_ = new_capacity;
            }
        }
    };
}
