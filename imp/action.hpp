#pragma once

#include <functional>
#include <stdexcept>
#include <vector>

namespace imp
{
    template<typename... args>
    class action_receiver;

    template<typename... args>
    class action_delegator
    {
        using receiver = action_receiver<args...>;
        friend class receiver;

    private:
        std::vector<receiver*> receivers_;

    public:
        action_delegator() = default;

        action_delegator(const action_delegator<args...>&) = delete;

        action_delegator(action_delegator<args...>&& o) noexcept
            : receivers_(std::move(o.receivers_))
        {
            set_receivers();
        }

        ~action_delegator()
        {
            unset_receivers();
        }

        action_delegator& operator=(const action_delegator<args...>&) = delete;

        action_delegator& operator=(action_delegator<args...>&& o) noexcept
        {
            if (this != &o)
            {
                unset_receivers();
                receivers_ = std::move(o.receivers_);
                set_receivers();
            }

            return *this;
        }

        void connect(receiver& receiver)
        {
            receiver.connect(*this);
        }

        void disconnect_all()
        {
            unset_receivers();
            receivers_.clear();
        }

        size_t receiver_count() const
        {
            return receivers_.size();
        }

        const receiver& get_receiver(size_t i) const
        {
            return *receivers_[i];
        }

        receiver& get_receiver(size_t i)
        {
            return *receivers_[i];
        }

        void emit(args... action_args) const
        {
            for (receiver* receiver : receivers_)
                receiver->call(std::forward<args>(action_args)...);
        }

    private:
        void set_receivers()
        {
            for (receiver* receiver : receivers_)
                receiver->delegator_ = this;
        }

        void unset_receivers()
        {
            for (receiver* receiver : receivers_)
                receiver->delegator_ = nullptr;
        }
    };

    template<typename... args>
    class action_receiver
    {
        using delegator = action_delegator<args...>;
        friend class delegator;

    private:
        action_delegator<args...>* delegator_ = nullptr;

    public:
        action_receiver() = default;

        action_receiver(const action_receiver<args...>& o)
            : delegator_(o.delegator_)
        {
            add_to_delegator();
        }

        action_receiver(action_receiver<args...>&& o) noexcept
            : delegator_(o.delegator_)
        {
            o.remove_from_delegator();
            o.delegator_ = nullptr;
            add_to_delegator();
        }

        ~action_receiver()
        {
            remove_from_delegator();
        }

        action_receiver<args...>& operator=(const action_receiver<args...>& o) noexcept
        {
            if (this != &o)
            {
                if (delegator_ != o.delegator_)
                {
                    remove_from_delegator();
                    delegator_ = o.delegator_;
                    add_to_delegator();
                }
            }

            return *this;
        }

        action_receiver<args...>& operator=(action_receiver<args...>&& o) noexcept
        {
            if (this != &o)
            {
                if (delegator_ != o.delegator_)
                {
                    remove_from_delegator();
                    delegator_ = o.delegator_;
                    o.remove_from_delegator();
                    o.delegator_ = nullptr;
                    add_to_delegator();
                }
                else
                {
                    o.remove_from_delegator();
                    o.delegator_ = nullptr;
                }
            }

            return *this;
        }

        void connect(delegator& d)
        {
            if (&d != delegator_)
            {
                if (delegator_)
                    remove_from_delegator();
                delegator_ = &d;
                add_to_delegator();
            }
        }

        void disconnect()
        {
            remove_from_delegator();
        }

        const delegator& get_delegator() const
        {
            if (delegator_)
                return *delegator_;
            else
                throw std::exception("Null pointer"); // TODO imp error ?
        }

        delegator& get_delegator()
        {
            if (delegator_)
                return *delegator_;
            else
                throw std::exception("Null pointer"); // TODO imp error ?
        }

        virtual void call(args... action_args) = 0;

    private:
        void add_to_delegator()
        {
            if (delegator_)
                delegator_->receivers_.push_back(this);
        }

        void remove_from_delegator()
        {
            if (delegator_)
            {
                auto& receivers_ = delegator_->receivers_;
                auto it = std::find(receivers_.begin(), receivers_.end(), this);
                receivers_.erase(it);
            }
        }
    };

    template<typename... args>
    struct generic_action_receiver
        : public action_receiver<args...>
    {
        using function = std::function<void(args...)>;

        function fn;

        generic_action_receiver(function fn)
            : fn(std::move(fn))
        {
        }

        void call(args... action_args) override
        {
            fn(action_args...);
        }
    };
}
