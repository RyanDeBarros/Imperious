#pragma once

#include "imp/type_erasure.hpp"

#include <memory>

namespace imp
{
	class box
	{
		template<typename ty>
		void(*deleter())(void*)
		{
			return [](void* ptr) { delete static_cast<ty*>(ptr); };
		}

		template<typename ty>
		void* (*copier())(const void*)
		{
			return [](const void* ptr) { return ptr ? static_cast<void*>(new ty(*static_cast<const ty*>(ptr))) : nullptr; };
		}

		void* _raw = nullptr;
		std::type_index _type;
		void(*_dtor)(void*);
		void* (*_copy)(const void*);

	public:
		box()
			: _raw(nullptr), _type(typeid(void)), _dtor(nullptr), _copy(nullptr)
		{
		}

		template<typename ty>
		box(ty* raw)
			: _raw(raw), _type(typeid(ty)), _dtor(deleter<ty>()), _copy(copier<ty>())
		{
		}

		box(const box& o)
			: _raw(o._copy ? o._copy(o._raw) : nullptr), _type(o._type), _dtor(o._dtor), _copy(o._copy)
		{
		}

		box(box&& o) noexcept
			: _raw(o._raw), _type(o._type), _dtor(o._dtor), _copy(o._copy)
		{
			o._raw = nullptr;
			o._type = typeid(void);
			o._dtor = nullptr;
			o._copy = nullptr;
		}

		~box()
		{
			if (_dtor)
				_dtor(_raw);
		}

		box& operator=(const box& o)
		{
			if (this != &o)
			{
				if (_dtor)
					_dtor(_raw);

				_raw = o._copy ? o._copy(o._raw) : nullptr;
				_type = o._type;
				_dtor = o._dtor;
				_copy = o._copy;
			}

			return *this;
		}

		box& operator=(box&& o) noexcept
		{
			if (this != &o)
			{
				if (_dtor)
					_dtor(_raw);

				_raw = o._raw;
				_type = o._type;
				_dtor = o._dtor;
				_copy = o._copy;

				o._raw = nullptr;
				o._type = typeid(void);
				o._dtor = nullptr;
				o._copy = nullptr;
			}

			return *this;
		}

		template<typename ty>
		bool holds() const
		{
			return _type == typeid(ty);
		}

		bool holds(std::type_index ti) const
		{
			return _type == ti;
		}

		bool holds(type_erasure type) const
		{
			return type == _type;
		}

		template<typename ty>
		const ty* as() const
		{
			if (holds<ty>())
				return static_cast<const ty*>(_raw);
			else
				return nullptr;
		}

		template<typename ty>
		ty* as()
		{
			if (holds<ty>())
				return static_cast<ty*>(_raw);
			else
				return nullptr;
		}

		template<typename ty>
		std::unique_ptr<ty> consume()
		{
			if (holds<ty>())
			{
				std::unique_ptr<ty> ptr(static_cast<ty*>(_raw));
				_raw = nullptr;
				return ptr;
			}
			else
				return nullptr;
		}
	};

	template<typename ty, typename... args> requires (!std::is_void_v<ty>)
	box make_box(args&&... args_)
	{
		return box(new ty(std::forward<args>(args_)...));
	}
}
