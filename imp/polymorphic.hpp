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

    private:
        std::unique_ptr<ty> ptr_;

    public:
        poly()
        {
        }

        poly(std::nullptr_t)
        {
        }

        poly(poly_default_impl)
            : ptr_(std::make_unique<ty>())
        {
        }

        poly(ty* raw)
            : ptr_(raw)
        {
        }

        poly(std::unique_ptr<ty>&& ptr)
            : ptr_(std::move(ptr))
        {
        }

        poly(const poly& o)
            : ptr_(clone(o.get()))
        {
        }

        poly(poly&& o) noexcept
            : ptr_(std::move(o.ptr_))
        {
        }

        template<std::derived_from<ty> uy>
        poly(const poly<uy>& o)
            : ptr_(clone(o.get()))
        {
        }

        template<std::derived_from<ty> uy>
        poly(poly<uy>&& o) noexcept
            : ptr_(std::move(o.ptr_))
        {
        }

        poly& operator=(const poly& o)
        {
            if (this != &o)
                ptr_ = clone(o.get());

            return *this;
        }

        poly& operator=(poly&& o) noexcept
        {
            if (this != &o)
                ptr_ = std::move(o.ptr_);

            return *this;
        }

        template<std::derived_from<ty> uy>
        poly& operator=(const poly<uy>& o)
        {
            ptr_ = clone(o.get());
            return *this;
        }

        template<std::derived_from<ty> uy>
        poly& operator=(poly<uy>&& o) noexcept
        {
            ptr_ = std::move(o.ptr_);
            return *this;
        }

        ty& operator*()
        {
            return *ptr_;
        }

        const ty& operator*() const
        {
            return *ptr_;
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
            return ptr_.get();
        }

        const ty* get() const
        {
            return ptr_.get();
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
            return ptr_.get() != nullptr;
        }

    private:
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
    };

    template<typename ty, typename... args>
    inline poly<ty> make_poly(args&&... args_)
    {
        return poly<ty>(std::make_unique<ty>(std::forward<args>(args_)...));
    }
}
