#pragma once

#include <unordered_map>
#include <vector>

namespace imp
{
	template<typename ty, typename hash = std::hash<ty>, typename equals = std::equal_to<ty>>
	class counter
	{
		std::unordered_map<ty, size_t, hash, equals> _map;

	public:
		void increment(const ty& obj, size_t by = 1)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
				it->second += by;
			else if (by > 0)
				_map.emplace(obj, by);
		}

		void increment(ty&& obj, size_t by = 1)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
				it->second += by;
			else if (by > 0)
				_map.emplace(std::move(obj), by);
		}

		template<typename U = ty>
		bool decrement(const U& obj)
		{
			auto it = _map.find(obj);
			if (it != _map.end())
			{
				if (--it->second == 0)
					_map.erase(it);
				return true;
			}
			else
				return false;
		}

		size_t count(const ty& obj) const
		{
			auto it = _map.find(obj);
			return it != _map.end() ? it->second : 0;
		}

		bool contains(const ty& obj) const
		{
			return count(obj) > 0;
		}

		void clear()
		{
			_map.clear();
		}

		void accumulate(const std::vector<ty>& vec)
		{
			for (const ty& el : vec)
				increment(el);
		}

		void accumulate(const counter<ty, hash, equals>& other)
		{
			for (auto it = other._map.begin(); it != other._map.end(); ++it)
				increment(it->first, it->second);
		}

		void accumulate(counter<ty, hash, equals>&& other)
		{
			for (auto it = std::make_move_iterator(other._map.begin()); it != std::make_move_iterator(other._map.end()); ++it)
				increment(std::move(it->first), it->second);
		}
	};
}
