#pragma once

#include <concepts>

namespace imp
{
    template<typename ty>
    concept numeric = std::integral<ty> || std::floating_point<ty>;

    template <typename ty, bool = std::is_enum_v<ty>>
    struct underlying_or_self { using type = ty; };

    template <typename ty>
    struct underlying_or_self<ty, true> { using type = std::underlying_type_t<ty>; };

    template <typename ty>
    using underlying_or_self_t = typename underlying_or_self<ty>::type;

    template<typename ty, typename... types>
    concept one_of = (std::same_as<ty, types> || ...);

    template<typename ty, typename... types>
    concept decays_one_of = (std::same_as<std::decay_t<ty>, types> || ...);
}
