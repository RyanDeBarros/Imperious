#pragma 

#include <concepts>

namespace imp
{
    template<typename ty>
    constexpr ty max(const ty& first, const ty& second)
    {
        return std::max(first, second);
    }

    template<typename ty, typename... Rest>
    inline ty max(const ty& first, const Rest&... rest)
    {
        return max(first, max(rest...));
    }

    template<typename ty>
    constexpr ty min(const ty& first, const ty& second)
    {
        return std::min(first, second);
    }

    template<typename ty, typename... Rest>
    inline ty min(const ty& first, const Rest&... rest)
    {
        return min(first, min(rest...));
    }

    namespace internal
    {
        template<typename tuple, size_t... i>
        constexpr size_t max_of_impl(const tuple& tup, std::index_sequence<i...>)
        {
            size_t max_index = 0;
            auto max_value = std::get<0>(tup);
            ((std::get<i>(tup) > max_value ? (max_value = std::get<i>(tup), max_index = i) : void(0)), ...);
            return max_index;
        }

        template<typename tuple, size_t... i>
        constexpr size_t min_of_impl(const tuple& tup, std::index_sequence<i...>)
        {
            size_t min_index = 0;
            auto min_value = std::get<0>(tup);
            ((std::get<i>(tup) < min_value ? (min_value = std::get<i>(tup), min_index = i) : void(0)), ...);
            return min_index;
        }
    }

    template<typename... args>
    constexpr size_t max_of(const args&... args_)
    {
        static_assert(sizeof...(args) > 0, "At least one argument required for imp::max_of");
        return internal::max_of_impl(std::tie(args_...), std::make_index_sequence<sizeof...(args)>{});
    }

    template<typename... args>
    constexpr size_t min_of(const args&... args_)
    {
        static_assert(sizeof...(args) > 0, "At least one argument required for imp::min_of");
        return internal::min_of_impl(std::tie(args_...), std::make_index_sequence<sizeof...(args)>{});
    }

    template<typename ty>
    inline ty dupl(const ty& obj)
    {
        return obj;
    }

    template<typename ty>
    inline ty dupl(ty&& obj)
    {
        return std::move(obj);
    }

    template<typename struct_ty, typename member_ty>
    constexpr size_t member_offset(member_ty struct_ty::* member)
    {
        return reinterpret_cast<std::size_t>(&(reinterpret_cast<struct_ty*>(0)->*member));
    }
}
