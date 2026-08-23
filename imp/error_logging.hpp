#pragma once

#include <utility>

namespace imp
{
	template<typename error, typename func, typename logger>
	void handle_error(func&& func_, logger&& logger_)
	{
		try
		{
			std::forward<func>(func_)();
		}
		catch (const error& e)
		{
			std::forward<logger>(logger_)(e);
		}
	}

	template<typename error, typename... errors, typename func, typename logger> requires (sizeof...(errors) > 0)
	void handle_error(func&& func_, logger&& logger_)
	{
		try
		{
			handle_error<errors...>(std::forward<func>(func_), logger_);
		}
		catch (const error& e)
		{
			logger_(e);
		}
	}
}
