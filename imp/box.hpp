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

    private:
        template<typename ty>
        static constexpr ops kOps = {
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
                    *static_cast<ty*>(to) = std::move(*static_cast<ty*>(from));
                    return true;
                }
                else
                    return false;
            }
        };

        void* raw_ = nullptr;
        type_erasure type_;
        const ops* ops_ = nullptr;

    public:
        box() = default;

        template<typename ty>
        box(ty* raw)
            : raw_(raw)
            , type_(erase_type<ty>())
            , ops_(&kOps<ty>)
        {
        }

        box(const box& o)
            : raw_(o.try_copy(o.raw_))
            , type_(o.type_)
            , ops_(o.ops_)
        {
            if (o.raw_ && !raw_)
                throw std::logic_error("Attempted to copy a non-copyable imp::box"); // TODO imp::error ?
        }

        box(box&& o) noexcept
            : raw_(o.raw_)
            , type_(o.type_)
            , ops_(o.ops_)
        {
            o.raw_ = nullptr;
            o.type_ = type_erasure();
            o.ops_ = nullptr;
        }

        ~box()
        {
            try_dtor(raw_);
        }

        box& operator=(const box& o)
        {
            if (this != &o)
            {
                if (type_ != o.type_ || !try_copy_assign(raw_, o.raw_))
                {
                    try_dtor(raw_);
                    raw_ = o.try_copy(o.raw_);
                }

                type_ = o.type_;
                ops_ = o.ops_;

                if (o.raw_ && !raw_)
                    throw std::logic_error("Attempted to copy a non-copyable imp::box"); // TODO imp::error ?
            }

            return *this;
        }

        box& operator=(box&& o) noexcept
        {
            if (this != &o)
            {
                if (type_ != o.type_ || !try_move_assign(raw_, o.raw_))
                {
                    try_dtor(raw_);
                    raw_ = o.raw_;
                }

                type_ = o.type_;
                ops_ = o.ops_;

                o.raw_ = nullptr;
                o.type_ = type_erasure();
                o.ops_ = nullptr;
            }

            return *this;
        }

    public:
        template<typename ty>
        bool holds() const
        {
            return type_ == erase_type<ty>();
        }

        bool holds(type_erasure type) const
        {
            return type_ == type;
        }

        type_erasure type() const
        {
            return type_;
        }

        const void* unsafe_raw() const
        {
            return raw_;
        }

        void* unsafe_raw()
        {
            return raw_;
        }

        template<typename ty>
        const ty* as() const
        {
            if (holds<ty>())
                return static_cast<const ty*>(raw_);
            else
                return nullptr;
        }

        template<typename ty>
        ty* as()
        {
            if (holds<ty>())
                return static_cast<ty*>(raw_);
            else
                return nullptr;
        }

        template<typename ty>
        std::unique_ptr<ty> consume()
        {
            if (holds<ty>())
            {
                std::unique_ptr<ty> ptr(static_cast<ty*>(raw_));
                raw_ = nullptr;
                return ptr;
            }
            else
                return nullptr;
        }

    private:
        void try_dtor(void* ptr) const
        {
            if (ops_)
                ops_->dtor(ptr);
        }

        void* try_copy(const void* ptr) const
        {
            if (ops_)
                return ops_->copy(ptr);
            else
                return nullptr;
        }

        bool try_copy_assign(void* to, const void* from)
        {
            if (to && from)
            {
                if (ops_)
                    return ops_->copy_assign(to, from);
            }

            return false;
        }

        bool try_move_assign(void* to, void* from)
        {
            if (to && from)
            {
                if (ops_)
                    return ops_->move_assign(to, from);
            }

            return false;
        }
    };

    template<typename ty, typename... args>
        requires (!std::is_void_v<ty>)
    box make_box(args&&... args_)
    {
        return box(new ty(std::forward<args>(args_)...));
    }

    template<typename ty, typename... args>
        requires (!std::is_void_v<ty>)
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
