#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <imp/functional_event.hpp>
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
		std::vector<std::unique_ptr<undo_action>> _undo;
		size_t _undo_stack_size = 0;
		std::vector<std::unique_ptr<undo_action>> _redo;
		size_t _redo_stack_size = 0;
		size_t _count_limit;
		size_t _size_limit;

	public:
		undo_history(size_t count_limit, size_t size_limit);
		virtual ~undo_history() = default;

		static undo_history& active_instance();

		void execute(std::unique_ptr<undo_action>&& action);

		virtual void push(std::unique_ptr<undo_action>&& action);

		void undo();
		void redo();

		virtual void prune();
		virtual void clear();

		void set_limits(size_t count_limit, size_t size_limit);

	private:
		void prune_undo_count(size_t count_limit);
		void prune_undo_size(size_t size_limit);

	protected:
		size_t undo_count() const;

		virtual void on_undo_post_success() {};
		virtual void on_undo_pre_fail() {};
		virtual void on_redo_post_success() {};
		virtual void on_redo_pre_fail() {};
	};

	struct active_undo_history : public imp::instance_stack<active_undo_history>
	{
		undo_history& uh_instance;

		active_undo_history(undo_history& undo_history);
	};

	class checkpoint_undo_history : public undo_history
	{
		std::optional<size_t> _clean_marker;

	public:
		imp::functional_event<> on_potential_clean;

		using undo_history::undo_history;

		void push(std::unique_ptr<undo_action>&& action) override;
		void prune() override;
		void clear() override;

		void mark_clean();

	protected:
		void on_undo_post_success() override;
		void on_undo_pre_fail() override;
		void on_redo_post_success() override;
		void on_redo_pre_fail() override;
	};
}
