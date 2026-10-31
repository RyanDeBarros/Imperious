#pragma once

#include <optional>
#include <vector>

namespace imp
{
    template<typename ty>
    class timeline_queue
    {
        size_t limit_;
        std::vector<ty> past_;
        std::optional<ty> present_;
        std::vector<ty> future_;

    public:
        timeline_queue(size_t limit)
            : limit_(limit)
        {
        }

        void push(ty obj)
        {
            if (present_)
                past_.push_back(std::move(*present_));

            present_ = std::move(obj);
            future_.clear();
            prune();
        }

        void move_backward()
        {
            if (present_)
            {
                future_.push_back(std::move(*present_));
                present_.reset();
            }

            if (!past_.empty())
            {
                present_ = std::move(past_.back());
                past_.pop_back();
            }
        }

        void move_forward()
        {
            if (present_)
            {
                past_.push_back(std::move(*present_));
                present_.reset();
            }

            if (!future_.empty())
            {
                present_ = std::move(future_.back());
                future_.pop_back();
            }
        }

        const ty* get_present() const
        {
            return present_ ? &*present_ : nullptr;
        }

        ty* get_present()
        {
            return present_ ? &*present_ : nullptr;
        }

        void set_limit(size_t limit)
        {
            limit_ = limit;
            prune();
        }

        void prune()
        {
            if (future_.size() + 1 >= limit_)
                past_.clear();
            else
            {
                const size_t past_limit = limit_ - future_.size() - 1;
                if (past_.size() > past_limit)
                    past_.erase(past_.begin(), past_.end() - past_limit);
            }
        }

        void clear()
        {
            past_.clear();
            present_.reset();
            future_.clear();
        }

        bool empty_backwards() const
        {
            return past_.empty();
        }

        bool empty_forwards() const
        {
            return future_.empty();
        }
    };
}
