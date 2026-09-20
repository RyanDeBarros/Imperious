#pragma once

#include <memory>
#include <stdexcept>

namespace imp
{
    template<typename derived>
    class issuer : public std::enable_shared_from_this<issuer<derived>>
    {
        using super = std::enable_shared_from_this<issuer<derived>>;

    protected:
        issuer() = default;
        issuer(const issuer&) = default;
        issuer(issuer&&) noexcept = default;
        ~issuer() = default;
        issuer& operator=(const issuer&) = default;
        issuer& operator=(issuer&&) noexcept = default;

    public:
        template<typename... args>
        static std::shared_ptr<derived> instantiate(args&&... args_)
        {
            return std::make_shared<derived>(std::forward<args>(args_)...);
        }

        class ticket
        {
            friend class issuer<derived>;
            std::weak_ptr<issuer<derived>> _issuer;

        protected:
            ticket(std::weak_ptr<issuer<derived>>&& _issuer)
                : _issuer(std::move(_issuer))
            {
            }

        public:
            ticket() = default;

            bool is_valid() const
            {
                return !_issuer.expired();
            }

        protected:
            class accessor
            {
                friend class issuer<derived>::ticket;
                std::shared_ptr<issuer<derived>> _issuer;

                accessor(std::shared_ptr<issuer>&& issuer)
                    : _issuer(std::move(issuer))
                {
                }

            public:
                const derived* get() const
                {
                    if (_issuer)
                        return static_cast<const derived*>(_issuer.get());
                    else
                        return nullptr;
                }

                derived* get()
                {
                    if (_issuer)
                        return static_cast<derived*>(_issuer.get());
                    else
                        return nullptr;
                }

                const derived& operator*() const
                {
                    if (_issuer)
                        return static_cast<const derived&>(*_issuer);
                    else
                        throw std::runtime_error("null pointer"); // TODO imp error ?
                }

                derived& operator*()
                {
                    if (_issuer)
                        return static_cast<derived&>(*_issuer);
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
                    return _issuer && _issuer.get();
                }

                void reset()
                {
                    _issuer.reset();
                }
            };

            accessor lock() const
            {
                return accessor(_issuer.lock());
            }

            void reset()
            {
                _issuer.reset();
            }

            void reset(issuer<derived>& _issuer)
            {
                this->_issuer = _issuer.weak_from_this();
            }
        };

        ticket issue()
        {
            return ticket(super::shared_from_this());
        }
    };

    template<typename derived> requires std::derived_from<derived, issuer<derived>>
    class issuer_instance
    {
        std::shared_ptr<derived> _issuer;
        derived& _cache;

    public:
        template<typename... Args>
        issuer_instance(Args&&... args)
            : _issuer(issuer<derived>::instantiate(std::forward<Args>(args)...)), _cache(*_issuer)
        {
        }

        issuer_instance(const issuer_instance<derived>& other)
            : _issuer(issuer<derived>::instantiate(*other._issuer)), _cache(*_issuer)
        {
        }

        issuer_instance(issuer_instance<derived>&& other) noexcept
            : _issuer(issuer<derived>::instantiate(std::move(*other._issuer))), _cache(*_issuer)
        {
        }

        issuer_instance<derived>& operator=(const issuer_instance<derived>& other)
        {
            if (this != &other)
                *_issuer = *other._issuer;
            return *this;
        }

        issuer_instance<derived>& operator=(issuer_instance<derived>&& other) noexcept
        {
            if (this != &other)
                *_issuer = std::move(*other._issuer);
            return *this;
        }

        const derived& operator*() const
        {
            return _cache;
        }

        derived& operator*()
        {
            return _cache;
        }

        const derived* operator->() const
        {
            return &_cache;
        }

        derived* operator->()
        {
            return &_cache;
        }

        const derived* address() const
        {
            return &_cache;
        }

        derived* address()
        {
            return &_cache;
        }
    };

    template<typename derived> requires std::derived_from<derived, issuer<derived>>
    using ticket = issuer<derived>::ticket;
}
