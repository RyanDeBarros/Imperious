#pragma once

#include <memory>
#include <stdexcept>

namespace imp
{
    template<typename derived>
    class issuer
        : public std::enable_shared_from_this<issuer<derived>>
    {
        using super = std::enable_shared_from_this<issuer<derived>>;

    public:
        class ticket;

    protected:
        issuer() = default;
        issuer(const issuer&) = default;
        issuer(issuer&&) noexcept = default;
        ~issuer() = default;
        issuer& operator=(const issuer&) = default;
        issuer& operator=(issuer&&) noexcept = default;

    public:
        template<typename... args_ty>
        static std::shared_ptr<derived> instantiate(args_ty&&... args)
        {
            return std::make_shared<derived>(std::forward<args_ty>(args)...);
        }

        ticket issue()
        {
            return ticket(super::shared_from_this());
        }

        class ticket
        {
            friend class issuer<derived>;

        private:
            std::weak_ptr<issuer<derived>> issuer_;

        protected:
            ticket(std::weak_ptr<issuer<derived>>&& issuer)
                : issuer_(std::move(issuer))
            {
            }

        public:
            ticket() = default;

            bool is_valid() const
            {
                return !issuer_.expired();
            }

        public:
            class accessor
            {
                friend class issuer<derived>::ticket;

            private:
                std::shared_ptr<issuer<derived>> issuer_;

            private:
                accessor(std::shared_ptr<issuer>&& issuer)
                    : issuer_(std::move(issuer))
                {
                }

            public:
                const derived* get() const
                {
                    if (issuer_)
                        return static_cast<const derived*>(issuer_.get());
                    else
                        return nullptr;
                }

                derived* get()
                {
                    if (issuer_)
                        return static_cast<derived*>(issuer_.get());
                    else
                        return nullptr;
                }

                const derived& operator*() const
                {
                    if (issuer_)
                        return static_cast<const derived&>(*issuer_);
                    else
                        throw std::runtime_error("null pointer"); // TODO imp error ?
                }

                derived& operator*()
                {
                    if (issuer_)
                        return static_cast<derived&>(*issuer_);
                    else
                        throw std::runtime_error("null pointer"); // TODO imp error ?
                }

                const derived* operator->() const
                {
                    return get();
                }

                derived* operator->()
                {
                    return get();
                }

                operator bool() const
                {
                    return issuer_ && issuer_.get();
                }

                void reset()
                {
                    issuer_.reset();
                }
            };

        protected:
            accessor lock() const
            {
                return accessor(issuer_.lock());
            }

            void reset()
            {
                issuer_.reset();
            }

            void reset(issuer<derived>& issuer_)
            {
                this->issuer_ = issuer_.weak_from_this();
            }
        };
    };

    template<typename derived>
        requires std::derived_from<derived, issuer<derived>>
    class issuer_instance
    {
        std::shared_ptr<derived> issuer_;
        derived& cache_;

    public:
        template<typename... args_ty>
        issuer_instance(args_ty&&... args)
            : issuer_(issuer<derived>::instantiate(std::forward<args_ty>(args)...))
            , cache_(*issuer_)
        {
        }

        issuer_instance(const issuer_instance<derived>& o)
            : issuer_(issuer<derived>::instantiate(*o.issuer_))
            , cache_(*issuer_)
        {
        }

        issuer_instance(issuer_instance<derived>&& o) noexcept
            : issuer_(issuer<derived>::instantiate(std::move(*o.issuer_)))
            , cache_(*issuer_)
        {
        }

        issuer_instance<derived>& operator=(const issuer_instance<derived>& o)
        {
            if (this != &o)
                *issuer_ = *o.issuer_;

            return *this;
        }

        issuer_instance<derived>& operator=(issuer_instance<derived>&& o) noexcept
        {
            if (this != &o)
                *issuer_ = std::move(*o.issuer_);

            return *this;
        }

        const derived& operator*() const
        {
            return cache_;
        }

        derived& operator*()
        {
            return cache_;
        }

        const derived* operator->() const
        {
            return &cache_;
        }

        derived* operator->()
        {
            return &cache_;
        }

        const derived* address() const
        {
            return &cache_;
        }

        derived* address()
        {
            return &cache_;
        }
    };

    template<typename derived>
        requires std::derived_from<derived, issuer<derived>>
    using ticket = issuer<derived>::ticket;
}
