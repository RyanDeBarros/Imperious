#pragma once

#include <imp/macros.hpp>

#include <memory>

namespace imp
{
    struct polymorphic
    {
        virtual std::unique_ptr<polymorphic> clone() const = 0;
    };

    // TODO in documentation, mention that klass must be passed as a paren-delimited parameter
#define IMP_POLYMORPHIC_IMPL(klass) \
	public: \
		virtual std::unique_ptr<polymorphic> clone() const override \
		{ \
            return std::make_unique<IMP_UNPAREN(klass)>(*this); \
		}

    struct poly_default_impl
    {
    };

    constexpr poly_default_impl poly_default;

    template<typename ty>
    class poly
    {
        static_assert(std::derived_from<ty, polymorphic>, "imp::poly type must derive from imp::polymorphic");

        template<typename>
        friend class poly;

        std::unique_ptr<ty> _ptr;

        static std::unique_ptr<ty> clone(const polymorphic* p)
        {
            if (p == nullptr)
                return nullptr;

            auto cloned = p->clone();
            ty* casted = dynamic_cast<ty*>(cloned.get());
            if (!casted)
                throw std::bad_cast();

            cloned.release();
            return std::unique_ptr<ty>(casted);
        }

    public:
        poly()
        {
        }

        poly(std::nullptr_t)
        {
        }

        poly(poly_default_impl)
            : _ptr(std::make_unique<ty>())
        {
        }

        poly(ty* raw)
            : _ptr(raw)
        {
        }

        poly(std::unique_ptr<ty>&& ptr)
            : _ptr(std::move(ptr))
        {
        }

        poly(const poly& o)
            : _ptr(clone(o.get()))
        {
        }

        poly(poly&& o) noexcept
            : _ptr(std::move(o._ptr))
        {
        }

        template<std::derived_from<ty> uy>
        poly(const poly<uy>& o)
            : _ptr(clone(o.get()))
        {
        }

        template<std::derived_from<ty> uy>
        poly(poly<uy>&& o) noexcept
            : _ptr(std::move(o._ptr))
        {
        }

        poly& operator=(const poly& o)
        {
            if (this != &o)
                _ptr = clone(o.get());

            return *this;
        }

        poly& operator=(poly&& o) noexcept
        {
            if (this != &o)
                _ptr = std::move(o._ptr);

            return *this;
        }

        template<std::derived_from<ty> uy>
        poly& operator=(const poly<uy>& o)
        {
            _ptr = clone(o.get());
            return *this;
        }

        template<std::derived_from<ty> uy>
        poly& operator=(poly<uy>&& o) noexcept
        {
            _ptr = std::move(o._ptr);
            return *this;
        }

        ty& operator*()
        {
            return *_ptr;
        }

        const ty& operator*() const
        {
            return *_ptr;
        }

        ty* operator->()
        {
            return get();
        }

        const ty* operator->() const
        {
            return get();
        }

        ty* get()
        {
            return _ptr.get();
        }

        const ty* get() const
        {
            return _ptr.get();
        }

        template<typename uy>
        uy* as()
        {
            return dynamic_cast<uy*>(get());
        }

        template<typename uy>
        const uy* as() const
        {
            return dynamic_cast<const uy*>(get());
        }

        template<typename uy>
        bool castable() const
        {
            return as<uy>() != nullptr;
        }

        operator bool() const
        {
            return _ptr.get() != nullptr;
        }
    };

    template<typename ty, typename... args>
    inline poly<ty> make_poly(args&&... args_)
    {
        return poly<ty>(std::make_unique<ty>(std::forward<args>(args_)...));
    }
}
