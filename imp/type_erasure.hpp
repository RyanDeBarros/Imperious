#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace imp
{
    namespace internal
    {
        constexpr uint64_t fnv1a_hash(std::string_view str) noexcept
        {
            uint64_t hash = 0xcbf29ce484222325ULL;
            for (char c : str)
            {
                hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
                hash *= 0x00000100000001B3ULL;
            }
            return hash;
        }
    }

    template<typename ty>
    constexpr std::string_view compiler_type_name() noexcept
    {
#if defined(_MSC_VER)
        return __FUNCSIG__;
#else
        return __PRETTY_FUNCTION__;
#endif
    }

    template<typename ty>
    constexpr uint64_t type_id() noexcept
    {
        return internal::fnv1a_hash(compiler_type_name<std::remove_cvref_t<ty>>());
    }

    struct type_erasure
    {
        template<typename ty>
        friend constexpr type_erasure erase_type() noexcept;

    private:
        uint64_t uid_;

    private:
        constexpr explicit type_erasure(uint64_t uid) noexcept
            : uid_(uid)
        {
        }

    public:
        constexpr type_erasure() noexcept
            : uid_(type_id<void>())
        {
        }

        constexpr bool operator==(const type_erasure&) const noexcept = default;
        constexpr bool operator!=(const type_erasure&) const noexcept = default;

        [[nodiscard]]
        constexpr size_t hash() const noexcept
        {
            return static_cast<size_t>(uid_);
        }

        [[nodiscard]]
        constexpr uint64_t uid() const
        {
            return uid_;
        }
    };

    template<typename ty>
    constexpr type_erasure erase_type() noexcept
    {
        return type_erasure(type_id<ty>());
    }

    template<typename ty, typename uy>
    constexpr bool erases_to() noexcept
    {
        return erase_type<ty>() == erase_type<uy>();
    }

    template<typename ty>
    constexpr const ty* resolve_type(type_erasure erasure, const void* ptr) noexcept
    {
        return erase_type<ty>() == erasure ? static_cast<const ty*>(ptr) : nullptr;
    }

    template<typename ty>
    constexpr ty* resolve_type(type_erasure erasure, void* ptr) noexcept
    {
        return erase_type<ty>() == erasure ? static_cast<ty*>(ptr) : nullptr;
    }

    template<typename ty>
    constexpr const ty* resolve_type(type_erasure erasure, const ty* ptr) noexcept
    {
        return resolve_type<ty>(erasure, static_cast<const void*>(ptr));
    }

    template<typename ty>
    constexpr ty* resolve_type(type_erasure erasure, ty* ptr) noexcept
    {
        return resolve_type<ty>(erasure, static_cast<void*>(ptr));
    }
}

template<>
struct std::hash<imp::type_erasure>
{
    size_t operator()(imp::type_erasure te) const
    {
        return te.hash();
    }
};
