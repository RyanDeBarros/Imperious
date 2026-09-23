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
        using copy_assign_ty = void(*)(void*, const void*);
        using move_assign_ty = void(*)(void*, void*);

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
            [](void* to, const void* from) { *static_cast<ty*>(to) = *static_cast<const ty*>(from); },
            [](void* to, void* from) { *static_cast<ty*>(to) = std::move(*static_cast<const ty*>(from)); }
        };

		void* _raw = nullptr;
        type_erasure _type;
        const ops* _ops = nullptr;

        dtor_ty dtor() const
        {
            return _ops ? _ops->dtor : nullptr;
        }

        void try_dtor(void* ptr) const
        {
            if (auto d = dtor())
                d(ptr);
        }

        copy_ty copy() const
        {
            return _ops ? _ops->copy : nullptr;
        }

        void* try_copy(const void* ptr) const
        {
            if (auto c = copy())
                return c(ptr);
            else
                return nullptr;
        }

        copy_assign_ty copy_assign() const
        {
            return _ops ? _ops->copy_assign : nullptr;
        }

        bool try_copy_assign(void* to, const void* from)
        {
            if (to && from)
            {
                if (auto ca = copy_assign())
                {
                    ca(to, from);
                    return true;
                }
            }

            return false;
        }

        move_assign_ty move_assign() const
        {
            return _ops ? _ops->move_assign : nullptr;
        }

        bool try_move_assign(void* to, void* from)
        {
            if (to && from)
            {
                if (auto ca = move_assign())
                {
                    ca(to, from);
                    return true;
                }
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
			if (!copy() && o._raw)
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

				if (!copy() && o._raw)
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
        return make_box<ty>(std::forward<ty>(obj));
    }

    template<typename ty>
    void forward_into_box(box& b, ty&& obj)
    {
        copy_box<ty>(b, std::forward<ty>(obj));
    }
}
