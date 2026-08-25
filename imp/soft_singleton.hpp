#pragma once

namespace imp
{
	template<typename ty>
	class soft_singleton
	{
	protected:
		soft_singleton() = default;
		~soft_singleton() = default;

	public:
		soft_singleton(const soft_singleton&) = delete;
		soft_singleton(soft_singleton&&) = delete;
		soft_singleton& operator=(const soft_singleton&) = delete;
		soft_singleton& operator=(soft_singleton&&) = delete;

		static ty& instance()
		{
			static ty inst;
			return inst;
		}
	};
}
