#pragma once

#include <utility>

namespace imp
{
    template<typename ty>
    struct swapper
    {
        ty front, back;

        swapper& swap()
        {
            std::swap(front, back);
            return *this;
        }
    };

    template<typename ty>
    class mut_swapper
    {
        ty front_, back_;
        mutable bool switch_ = true;

    public:
        mut_swapper() = default;

        explicit mut_swapper(ty element)
            : front_(element), back_(std::move(element))
        {
        }

        mut_swapper(ty front, ty back)
            : front_(std::move(front)), back_(std::move(back))
        {
        }

        const ty& front() const
        {
            return switch_ ? front_ : back_;
        }

        ty& front()
        {
            return switch_ ? front_ : back_;
        }

        const ty& back() const
        {
            return switch_ ? back_ : front_;
        }

        ty& back()
        {
            return switch_ ? back_ : front_;
        }

        mut_swapper& swap() const
        {
            switch_ = !switch_;
        }
    };
}
