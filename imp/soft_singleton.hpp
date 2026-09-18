#pragma once

#include <type_traits>

namespace imp
{
	template<typename derived>
	class soft_singleton
	{
	protected:
		soft_singleton() = default;
		~soft_singleton() = default;

    private:
		soft_singleton(const soft_singleton&) = delete;
		soft_singleton(soft_singleton&&) = delete;
		soft_singleton& operator=(const soft_singleton&) = delete;
		soft_singleton& operator=(soft_singleton&&) = delete;

	public:
		static derived& instance()
		{
            static_assert(std::is_final_v<derived>, "imp::singleton derived type must be final");
            // TODO note in documentation recommending that derived class should have a private ctor and make singleton its friend
			static derived inst;
			return inst;
		}
	};
}
