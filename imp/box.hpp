#pragma once

#include "imp/type_erasure.hpp"

#include <concepts>
#include <memory>
#include <stdexcept>

namespace imp
{
	class box
	{
        using dtor_ty = void (*)(void*);
        using copy_ty = void* (*)(const void*);
        using copy_assign_ty = bool(*)(void*, const void*);
        using move_assign_ty = bool(*)(void*, void*);

        struct ops
        {
            dtor_ty dtor = nullptr;
            copy_ty copy = nullptr;
            copy_assign_ty copy_assign = nullptr;
            move_assign_ty move_assign = nullptr;
        };

        template<typename ty>
        static constexpr ops ops_impl
        {
            [](void* ptr) { delete static_cast<ty*>(ptr); },
            [](const void* ptr) -> void*
            {
                if constexpr (std::is_copy_constructible_v<ty>)
                    return ptr ? new ty(*static_cast<const ty*>(ptr)) : nullptr;
                else
                    return nullptr;
            },
            [](void* to, const void* from)
            {
                if constexpr (std::is_copy_assignable_v<ty>)
                {
                    *static_cast<ty*>(to) = *static_cast<const ty*>(from);
                    return true;
                }
                else
                    return false;
            },
            [](void* to, void* from)
            {
                if constexpr (std::is_move_assignable_v<ty>)
                {
                    *static_cast<ty*>(to) = std::move(*static_cast<const ty*>(from));
                    return true;
                }
                else
                    return false;
            }
        };

		void* _raw = nullptr;
        type_erasure _type;
        const ops* _ops = nullptr;

        void try_dtor(void* ptr) const
        {
            if (_ops)
                _ops->dtor(ptr);
        }

        void* try_copy(const void* ptr) const
        {
            if (_ops)
                return _ops->copy(ptr);
            else
                return nullptr;
        }

        bool try_copy_assign(void* to, const void* from)
        {
            if (to && from)
            {
                if (_ops)
                    return _ops->copy_assign(to, from);
            }

            return false;
        }

        bool try_move_assign(void* to, void* from)
        {
            if (to && from)
            {
                if (_ops)
                    return _ops->move_assign(to, from);
            }

            return false;
        }

	public:
        box() = default;

		template<typename ty>
		box(ty* raw)
			: _raw(raw), _type(erase_type<ty>()), _ops(&ops_impl<ty>)
		{
		}

		box(const box& o)
			: _raw(o.try_copy(o._raw)), _type(o._type), _ops(o._ops)
		{
			if (o._raw && !_raw)
				throw std::logic_error("Attempted to copy a non-copyable imp::box"); // TODO imp::error ?
		}

        box(box&& o) noexcept
            : _raw(o._raw), _type(o._type), _ops(o._ops)
        {
            o._raw = nullptr;
            o._type = type_erasure();
			o._ops = nullptr;
		}

		~box()
		{
            try_dtor(_raw);
		}

		box& operator=(const box& o)
		{
			if (this != &o)
			{
                if (_type != o._type || !try_copy_assign(_raw, o._raw))
                {
                    try_dtor(_raw);
                    _raw = o.try_copy(o._raw);
                }

				_type = o._type;
				_ops = o._ops;

				if (o._raw && !_raw)
					throw std::logic_error("Attempted to copy a non-copyable imp::box"); // TODO imp::error ?
			}

			return *this;
		}

		box& operator=(box&& o) noexcept
		{
			if (this != &o)
			{
                if (_type != o._type || !try_move_assign(_raw, o._raw))
                {
                    try_dtor(_raw);
	    			_raw = o._raw;
                }

				_type = o._type;
				_ops = o._ops;

				o._raw = nullptr;
				o._type = type_erasure();
				o._ops = nullptr;
			}

			return *this;
		}

		template<typename ty>
		bool holds() const
		{
			return _type == erase_type<ty>();
		}

		bool holds(type_erasure type) const
		{
			return _type == type;
		}

        type_erasure type() const
        {
            return _type;
        }

        const void* unsafe_raw() const
        {
            return _raw;
        }

        void* unsafe_raw()
        {
            return _raw;
        }

		template<typename ty>
		const ty* as() const
		{
			if (holds<ty>())
				return static_cast<const ty*>(_raw);
			else
				return nullptr;
		}

		template<typename ty>
		ty* as()
		{
			if (holds<ty>())
				return static_cast<ty*>(_raw);
			else
				return nullptr;
		}

		template<typename ty>
		std::unique_ptr<ty> consume()
		{
			if (holds<ty>())
			{
				std::unique_ptr<ty> ptr(static_cast<ty*>(_raw));
				_raw = nullptr;
				return ptr;
			}
			else
				return nullptr;
		}
	};

	template<typename ty, typename... args> requires (!std::is_void_v<ty>)
	box make_box(args&&... args_)
	{
		return box(new ty(std::forward<args>(args_)...));
	}

	template<typename ty, typename... args> requires (!std::is_void_v<ty>)
    void copy_box(box& b, args&&... args_)
    {
        if (ty* obj = b.as<ty>())
            *obj = ty(std::forward<args>(args_)...);
        else
            b = make_box<ty>(std::forward<args>(args_)...);
    }

    template<typename ty>
    box forward_to_box(ty&& obj)
    {
        return make_box<std::remove_cvref_t<ty>>(std::forward<ty>(obj));
    }

    template<typename ty>
    void forward_into_box(box& b, ty&& obj)
    {
        copy_box<std::remove_cvref_t<ty>>(b, std::forward<ty>(obj));
    }
}
