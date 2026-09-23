#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace imp
{
    namespace internal
    {
        constexpr std::uint64_t fnv1a_hash(std::string_view str) noexcept
        {
            std::uint64_t hash = 0xcbf29ce484222325ULL;
            for (char c : str)
            {
                hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
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
    constexpr std::uint64_t type_id() noexcept
    {
        return internal::fnv1a_hash(compiler_type_name<std::remove_cvref_t<ty>>());
    }

    struct type_erasure
    {
    private:
        std::uint64_t _uid;

        template<typename ty>
        friend constexpr type_erasure erase_type() noexcept;

        constexpr explicit type_erasure(std::uint64_t uid) noexcept
            : _uid(uid)
        {
        }

    public:
        constexpr type_erasure() noexcept
            : _uid(type_id<void>())
        {
        }

        constexpr bool operator==(const type_erasure&) const noexcept = default;
        constexpr bool operator!=(const type_erasure&) const noexcept = default;

        [[nodiscard]]
        constexpr size_t hash() const noexcept
        {
            return static_cast<size_t>(_uid);
        }

        [[nodiscard]]
        constexpr std::uint64_t uid() const
        {
            return _uid;
        }
    };

    template<typename ty>
    constexpr type_erasure erase_type() noexcept
    {
        return type_erasure(type_id<ty>());
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
