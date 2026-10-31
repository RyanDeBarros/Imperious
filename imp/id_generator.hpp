#pragma once

#include <imp/issuer.hpp>

#include <limits>
#include <stack>
#include <stdexcept>
#include <string>

namespace imp
{
    template<std::unsigned_integral ty>
    class soft_id_generator
    {
        ty next_;
        ty max_;
        std::stack<ty> yielded_;

    public:
        soft_id_generator(ty initial = ty(0), ty max = std::numeric_limits<ty>::max())
            : next_(initial)
            , max_(max)
        {
        }

        ty gen()
        {
            if (yielded_.empty())
            {
                if (next_ > max_)
                    throw std::out_of_range(std::to_string(next_) + " > " + std::to_string(max_)); // TODO imp error ?
                return next_++;
            }

            ty _id = yielded_.top();
            yielded_.pop();
            return _id;
        }

        void yield(ty _id)
        {
            yielded_.push(_id);
        }
    };

    template<typename ty>
    class strict_id;

    namespace internal
    {
        template<std::unsigned_integral ty>
        class strict_id_generator
            : public issuer<strict_id_generator<ty>>
        {
            using super = issuer<strict_id_generator<ty>>;
            friend class strict_id<ty>;

        private:
            ty next_;
            ty max_;
            std::stack<ty> yielded_;

        public:
            strict_id_generator(ty initial, ty max)
                : next_(initial)
                , max_(max)
            {
            }

            strict_id_generator(const strict_id_generator&) = delete;
            strict_id_generator(strict_id_generator&&) noexcept = default;
            strict_id_generator& operator=(strict_id_generator&&) noexcept = default;

            strict_id<ty> generate()
            {
                return super::issue();
            }
        };
    }

    template<typename ty>
    class strict_id
        : public ticket<internal::strict_id_generator<ty>>
    {
        using super = ticket<internal::strict_id_generator<ty>>;
        friend class internal::strict_id_generator<ty>;

    private:
        ty _id = ty(-1);

    private:
        strict_id(super&& o) noexcept
            : super(std::move(o))
        {
            init();
        }

    public:
        strict_id() = default;

        strict_id(const strict_id& o)
            : super(o)
        {
            init();
        }

        strict_id(strict_id&& o) noexcept
            : super(std::move(o))
            , _id(o._id)
        {
            o._id = ty(-1);
        }

        strict_id& operator=(const strict_id& o)
        {
            if (this != &o)
            {
                del();
                super::operator=(o);
                init();
            }
        }

        strict_id& operator=(strict_id&& o) noexcept
        {
            if (this != &o)
            {
                del();
                super::operator=(std::move(o));
                _id = o._id;
                o._id = ty(-1);
            }
            return *this;
        }

        ~strict_id()
        {
            del();
        }

        ty get() const
        {
            if (super::is_valid())
                return _id;
            else
                throw std::logic_error("invalid id"); // TODO imp error ?
        }

        void yield()
        {
            del();
        }

    private:
        void init()
        {
            auto accessor = this->lock();
            if (internal::strict_id_generator<ty>* generator = accessor.get())
            {
                if (generator->yielded_.empty())
                {
                    if (generator->next_ > generator->max_)
                    {
                        // TODO imp error ?
                        throw std::out_of_range(
                            std::to_string(generator->next_) + " > " + std::to_string(generator->max_));
                    }

                    _id = generator->next_++;
                }
                else
                {
                    _id = generator->yielded_.top();
                    generator->yielded_.pop();
                }
            }
        }

        void del()
        {
            auto accessor = super::lock();
            if (internal::strict_id_generator<ty>* generator = accessor.get())
                generator->yielded_.push(_id);
            super::reset();
            _id = ty(-1);
        }
    };

    template<std::unsigned_integral ty>
    using strict_id_generator = issuer_instance<internal::strict_id_generator<ty>>;
}
