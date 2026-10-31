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
        template<typename>
        friend class group;

    private:
        ty* data_ = nullptr;
        size_t count_ = 0;

    public:
        group(ty* array, size_t count)
            : data_(array)
            , count_(count)
        {
        }

        template<typename other_ty>
            requires std::convertible_to<other_ty*, ty*>
        group(std::span<other_ty> span)
            : data_(span.data())
            , count_(span.size())
        {
        }

        template<typename other_ty, size_t n>
            requires std::convertible_to<other_ty*, ty*>
        group(std::array<other_ty, n>& array)
            : data_(array.data())
            , count_(n)
        {
        }

        template<typename other_ty, size_t n>
            requires std::convertible_to<const other_ty*, ty*>
        group(const std::array<other_ty, n>& array)
            : data_(array.data())
            , count_(n)
        {
        }

        group<ty>& operator=(const group<const ty>& o)
        {
            if (data_ != o.data_)
            {
                if (count_ != o.count_)
                    throw std::logic_error("imp::group mismatching size in operator=()"); // TODO imp error ?

                // TODO safety checks/workarounds if arrays overlap
                for (size_t i = 0; i < count_; ++i)
                    data_[i] = o.data_[i];
            }

            return *this;
        }

        operator group<const ty>() const
        {
            return group<const ty>(data_, count_);
        }

        ty* data() const
        {
            return data_;
        }

        size_t size() const
        {
            return count_;
        }

        bool operator==(const group<const ty>& o) const
        {
            if (count_ != o.count_)
                return false;

            for (size_t i = 0; i < count_; ++i)
            {
                if (data_[i] != o.data_[i])
                    return false;
            }

            return true;
        }

        bool operator!=(const group<const ty>& o) const
        {
            if (count_ != o.count_)
                return true;

            for (size_t i = 0; i < count_; ++i)
            {
                if (data_[i] != o.data_[i])
                    return true;
            }

            return false;
        }
    };
}
