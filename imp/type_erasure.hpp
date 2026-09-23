#pragma once

#include <string>
#include <string_view>

namespace imp
{
    namespace internal
    {
        constexpr std::uint64_t fnv1a_hash(std::string_view str)
        {
            std::uint64_t hash = 0xcbf29ce484222325ULL;
            for (char c : str)
            {
                hash ^= static_cast<std::uint64_t>(c);
                hash *= 0x00000100000001B3ULL;
            }
            return hash;
        }

    }

    template<typename ty>
    constexpr std::string_view compiler_type_name()
    {
#if defined(_MSC_VER)
        return __FUNCSIG__;
#else
        return __PRETTY_FUNCTION__;
#endif
    }

    template<typename ty>
    constexpr std::uint64_t type_id()
    {
        return internal::fnv1a_hash(compiler_type_name<ty>());
    }

    struct type_erasure
    {
    private:
        std::uint64_t _index;

    public:
        template<typename ty>
        type_erasure()
            : _index(type_id<ty>())
        {
        }

        std::string repr() const
        {
            return std::to_string(_index);
        }

        bool operator==(const type_erasure&) const = default;
        bool operator!=(const type_erasure&) const = default;

        size_t hash() const
        {
            return static_cast<size_t>(_index);
        }
    };

	template<typename ty>
	const ty* resolve_type(type_erasure erasure, const void* ptr)
	{
		if (erase_type<ty>() == erasure)
			return static_cast<const ty*>(ptr);
		else
			return nullptr;
	}

	template<typename ty>
	ty* resolve_type(type_erasure erasure, void* ptr)
	{
		if (erase_type<ty>() == erasure)
			return static_cast<ty*>(ptr);
		else
			return nullptr;
	}

	template<typename ty>
	const ty* resolve_type(type_erasure erasure, const ty* ptr)
	{
		return resolve_type<ty>(erasure, static_cast<const void*>(ptr));
	}

	template<typename ty>
	ty* resolve_type(type_erasure erasure, ty* ptr)
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
