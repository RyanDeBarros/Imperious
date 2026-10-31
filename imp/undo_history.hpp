#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <imp/event.hpp>
#include <imp/instance_stack.hpp>

namespace imp
{
    struct undo_action
    {
        virtual ~undo_action() = default;

        virtual bool forward() = 0;
        virtual bool backward() = 0;
        virtual size_t empirical_size() const = 0;
    };

    class undo_history
    {
        std::vector<std::unique_ptr<undo_action>> undo_;
        size_t undo_stack_size_ = 0;
        std::vector<std::unique_ptr<undo_action>> redo_;
        size_t redo_stack_size_ = 0;
        size_t count_limit_;
        size_t size_limit_;

    public:
        undo_history(size_t count_limit, size_t size_limit);
        virtual ~undo_history() = default;

    public:
        virtual void push(std::unique_ptr<undo_action>&& action);
        virtual void prune();
        virtual void clear();

    protected:
        virtual void on_undo_post_success() {};
        virtual void on_undo_pre_fail() {};
        virtual void on_redo_post_success() {};
        virtual void on_redo_pre_fail() {};

    public:
        static undo_history& active_instance();

        void execute(std::unique_ptr<undo_action>&& action);

        void undo();
        void redo();

        void set_limits(size_t count_limit, size_t size_limit);

    protected:
        size_t undo_count() const;

    private:
        void prune_undo_count(size_t count_limit);
        void prune_undo_size(size_t size_limit);
    };

    struct active_undo_history
        : public imp::instance_stack<active_undo_history>
    {
        undo_history& uh_instance;

        active_undo_history(undo_history& undo_history);
    };

    class checkpoint_undo_history
        : public undo_history
    {
        std::optional<size_t> clean_marker_;

    public:
        imp::event<> on_potential_clean;

    public:
        using undo_history::undo_history;

    public:
        void push(std::unique_ptr<undo_action>&& action) override;
        void prune() override;
        void clear() override;

    protected:
        void on_undo_post_success() override;
        void on_undo_pre_fail() override;
        void on_redo_post_success() override;
        void on_redo_pre_fail() override;

    public:
        void mark_clean();
    };
}
