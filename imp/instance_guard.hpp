#pragma once

#include <stdexcept>

namespace imp
{
	template<typename derived>
	struct instance_guard
	{
	private:
		static inline derived* _instance = nullptr;

	public:
		instance_guard()
		{
			if (_instance)
				throw std::logic_error("existing active instance"); // TODO class name prefix + imp::error ?

			_instance = static_cast<derived*>(this);
		}

		instance_guard(const instance_guard&) = delete;
		instance_guard(instance_guard&&) = delete;
		instance_guard& operator=(const instance_guard&) = delete;
		instance_guard& operator=(instance_guard&&) = delete;

		~instance_guard()
		{
			if (_instance == static_cast<derived*>(this))
				_instance = nullptr;
		}

		static derived& instance()
		{
			if (_instance)
				return *_instance;
			else
				throw std::logic_error("no active instance"); // TODO class name prefix + imp::error ?
		}

		static derived* active_instance()
		{
			return _instance;
		}
	};
}
