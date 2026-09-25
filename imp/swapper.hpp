#pragma once

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
}
