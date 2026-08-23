#pragma once

#include <utility>

namespace imp
{
	template<typename ty>
	class modifiable
	{
		bool _dirty = false;
		ty _obj;

	public:
		modifiable(ty ini = ty()) : _obj(ini) {}

		modifiable(const modifiable& o)
		{
			Set(o._obj);
		}

		modifiable(modifiable&& o) noexcept
			: _dirty(o._dirty), _obj(std::move(o._obj))
		{
		}

		modifiable& operator=(const modifiable& o)
		{
			if (this != &o)
				set(o._obj);

			return *this;
		}

		modifiable& operator=(modifiable&& o)
		{
			if (this != &o)
			{
				_dirty = o._dirty;
				_obj = std::move(o._obj);
			}

			return *this;
		}

		modifiable& operator=(ty val)
		{
			set(val);
			return *this;
		}

		operator ty() const
		{
			return get();
		}

		ty get() const
		{
			return _obj;
		}

		void set(ty val)
		{
			if (_obj != val)
			{
				_obj = val;
				_dirty = true;
			}
		}

		bool modified()
		{
			return _dirty;
		}

		bool consume_modified()
		{
			if (_dirty)
			{
				_dirty = false;
				return true;
			}
			else
				return false;
		}
	};
}
