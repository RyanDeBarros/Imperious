#include "undo_history.hpp"

#include <stack>

namespace imp
{
    undo_history::undo_history(size_t count_limit, size_t size_limit)
        : count_limit_(count_limit)
        , size_limit_(size_limit)
    {
    }

    void undo_history::push(std::unique_ptr<undo_action>&& action)
    {
        undo_stack_size_ += action->empirical_size();
        redo_stack_size_ = 0;

        undo_.push_back(std::move(action));
        redo_.clear();

        prune();
    }

    void undo_history::prune()
    {
        if (redo_.size() >= count_limit_)
            prune_undo_count(0);
        else
            prune_undo_count(count_limit_ - redo_.size());

        if (redo_stack_size_ >= size_limit_)
            prune_undo_size(0);
        else
            prune_undo_size(size_limit_ - redo_stack_size_);
    }

    void undo_history::clear()
    {
        undo_stack_size_ = 0;
        undo_.clear();

        redo_stack_size_ = 0;
        redo_.clear();
    }

    undo_history& undo_history::active_instance()
    {
        return active_undo_history::instance().uh_instance;
    }

    void undo_history::execute(std::unique_ptr<undo_action>&& action)
    {
        if (action->forward())
            push(std::move(action));
        else
            clear();
    }

    void undo_history::undo()
    {
        if (!undo_.empty())
        {
            const size_t sz = undo_.back()->empirical_size();
            undo_stack_size_ -= sz;
            redo_stack_size_ += sz;

            std::unique_ptr<undo_action> action = std::move(undo_.back());
            undo_.pop_back();

            if (action->backward())
            {
                redo_.push_back(std::move(action));
                on_undo_post_success();
            }
            else
            {
                on_undo_pre_fail();
                undo_stack_size_ = 0;
                undo_.clear();
            }

            prune();
        }
    }

    void undo_history::redo()
    {
        if (!redo_.empty())
        {
            const size_t sz = redo_.back()->empirical_size();
            redo_stack_size_ -= sz;
            undo_stack_size_ += sz;

            std::unique_ptr<undo_action> action = std::move(redo_.back());
            redo_.pop_back();

            if (action->forward())
            {
                undo_.push_back(std::move(action));
                on_redo_post_success();
            }
            else
            {
                on_redo_pre_fail();
                redo_stack_size_ = 0;
                redo_.clear();
            }

            prune();
        }
    }

    void undo_history::set_limits(size_t count_limit, size_t size_limit)
    {
        count_limit_ = count_limit;
        size_limit_ = size_limit;
        prune();
    }

    size_t undo_history::undo_count() const
    {
        return undo_.size();
    }

    void undo_history::prune_undo_count(size_t count_limit)
    {
        if (count_limit == 0)
        {
            undo_stack_size_ = 0;
            undo_.clear();
        }
        else
        {
            if (undo_.size() > count_limit)
            {
                const size_t amount = undo_.size() - count_limit;

                for (size_t i = 0; i < amount; ++i)
                    undo_stack_size_ -= undo_[i]->empirical_size();

                undo_.erase(undo_.begin(), undo_.begin() + amount);
            }
        }
    }

    void undo_history::prune_undo_size(size_t size_limit)
    {
        if (size_limit == 0)
        {
            undo_stack_size_ = 0;
            undo_.clear();
        }
        else
        {
            auto it = undo_.begin();
            while (undo_stack_size_ > size_limit && it != undo_.end())
                undo_stack_size_ -= (*it++)->empirical_size();

            if (it != undo_.begin())
                undo_.erase(undo_.begin(), it);
        }
    }

    active_undo_history::active_undo_history(undo_history& undo_history)
        : uh_instance(undo_history)
    {
    }

    void checkpoint_undo_history::push(std::unique_ptr<undo_action>&& action)
    {
        if (clean_marker_ && *clean_marker_ > undo_count())
            clean_marker_.reset();

        undo_history::push(std::move(action));
    }

    void checkpoint_undo_history::prune()
    {
        const size_t initial_undo_count = undo_count();

        undo_history::prune();

        if (clean_marker_)
        {
            size_t delta = initial_undo_count - undo_count();

            if (*clean_marker_ >= delta)
                clean_marker_ = *clean_marker_ - delta;
            else
                clean_marker_.reset();
        }
    }

    void checkpoint_undo_history::clear()
    {
        undo_history::clear();

        if (clean_marker_ && *clean_marker_ > 0)
            clean_marker_.reset();
    }

    void checkpoint_undo_history::mark_clean()
    {
        clean_marker_ = undo_count();
    }

    void checkpoint_undo_history::on_undo_post_success()
    {
        if (clean_marker_ && (undo_count() == *clean_marker_ || undo_count() + 1 == *clean_marker_))
            on_potential_clean.invoke();
    }

    void checkpoint_undo_history::on_undo_pre_fail()
    {
        if (clean_marker_)
        {
            if (*clean_marker_ <= undo_count())
                clean_marker_.reset();
            else
                clean_marker_ = *clean_marker_ - (undo_count() + 1);
        }
    }

    void checkpoint_undo_history::on_redo_post_success()
    {
        if (clean_marker_ && (undo_count() == *clean_marker_ || undo_count() == *clean_marker_ + 1))
            on_potential_clean.invoke();
    }

    void checkpoint_undo_history::on_redo_pre_fail()
    {
        if (clean_marker_ && *clean_marker_ > undo_count())
            clean_marker_.reset();
    }
}
