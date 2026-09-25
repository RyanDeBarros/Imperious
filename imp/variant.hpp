#pragma once

#include <imp/dependent_false.hpp>
#include <imp/macros.hpp>

#include <stdexcept>

namespace imp
{
    template<typename... types>
    class variant
    {
        static_assert(imp::dependent_false_v<variant<types...>>);
    };

#define _IMP_ENUM_DEF(N) _##N
#define _IMP_TYPE_ENUM(N)\
	public:\
	enum class type\
	{\
		none,\
		_IMP_REPEAT_COMMA(_IMP_ENUM_DEF, N)\
	};\
	private:\
		type _type;

#define _IMP_STORAGE_DEF(N) ty_##N ty##N;
#define _IMP_STORAGE(N) union storage\
	{\
		_IMP_REPEAT(_IMP_STORAGE_DEF, N)\
		storage() {}\
		~storage() {}\
	} _storage;

#define _IMP_DEL_CASE(N) case type::_##N: _storage.ty##N.~ty_##N(); break;
#define _IMP_DEL(N)\
	void del()\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_DEL_CASE, N);\
		default: break;\
		}\
		_type = type::none;\
	}

#define _IMP_COPY_CASE(N) case type::_##N: new (&_storage.ty##N) ty_##N(o._storage.ty##N); break;
#define _IMP_COPY_FROM(N)\
	void copy_from(const variant& o)\
	{\
		_type = o._type;\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_COPY_CASE, N);\
		default: break;\
		}\
	}

#define _IMP_MOVE_CASE(N) case type::_##N: new (&_storage.ty##N) ty_##N(std::move(o._storage.ty##N)); break;
#define _IMP_MOVE_FROM(N)\
	void move_from(variant&& o) noexcept\
	{\
		_type = o._type;\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_MOVE_CASE, N);\
		default: break;\
		}\
		o._type = type::none;\
	}

#define _IMP_PTR_CASE(N) case type::_##N: return &_storage.ty##N;
#define _IMP_PTR(N)\
	const void* ptr() const\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_PTR_CASE, N);\
		default: return nullptr;\
		}\
	}\
	void* ptr()\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_PTR_CASE, N);\
		default: return nullptr;\
		}\
	}

#define _IMP_VALUE_CTOR(N)\
	variant(const ty_##N& ty##N)\
		: _type(type::_##N)\
	{\
		new (&_storage.ty##N) ty_##N(ty##N);\
	}\
	variant(ty_##N&& ty##N) noexcept\
		: _type(type::_##N)\
	{\
		new (&_storage.ty##N) ty_##N(std::move(ty##N));\
	}\
	variant& operator=(const ty_##N& ty##N)\
	{\
		if (_type == type::_##N)\
			_storage.ty##N = ty##N;\
		else\
		{\
			del();\
			_type = type::_##N;\
			new (&_storage.ty##N) ty_##N(ty##N);\
		}\
		return *this;\
	}\
	variant& operator=(ty_##N&& ty##N) noexcept\
	{\
		if (_type == type::_##N)\
			_storage.ty##N = std::move(ty##N);\
		else\
		{\
			del();\
			_type = type::_##N;\
			new (&_storage.ty##N) ty_##N(std::move(ty##N));\
		}\
		return *this;\
	}

#define _IMP_CTOR(N)\
	variant() : _type(type::none) {}\
	_IMP_REPEAT(_IMP_VALUE_CTOR, N)\
	variant(const variant& o)\
    {\
        copy_from(o);\
    }\
	variant(variant&& o) noexcept\
    {\
        move_from(std::move(o));\
    }\
	~variant()\
    {\
        del();\
    }\
	variant& operator=(const variant& o)\
    {\
        if (this != &o)\
        {\
            del();\
            copy_from(o);\
        }\
        return *this;\
    }\
	variant& operator=(variant&& o) noexcept\
    {\
        if (this != &o)\
        {\
            del();\
            move_from(std::move(o));\
        }\
        return *this;\
    }

#define _IMP_HOLDS_CHECK(N) if constexpr (std::is_same_v<ty_, ty_##N>) return _type == type::_##N;
#define _IMP_HOLDS(N)\
	template<typename ty_>\
	bool holds() const\
	{\
		_IMP_REPEAT(_IMP_HOLDS_CHECK, N);\
		return false;\
	}

#define _IMP_GET(N)\
	template<typename ty_>\
	const ty_& get() const\
	{\
		if (holds<ty_>())\
			return *reinterpret_cast<const ty_*>(ptr());\
		else\
			throw std::bad_variant_access(); /* TODO imp error ? */\
	}\
	template<typename ty_>\
	ty_& get()\
	{\
		if (holds<ty_>())\
			return *reinterpret_cast<ty_*>(ptr());\
		else\
			throw std::bad_variant_access(); /* TODO imp error ? */\
	}\
	template<typename ty_>\
	const ty_* safe_get() const\
	{\
		if (holds<ty_>())\
			return reinterpret_cast<const ty_*>(ptr());\
		else\
			return nullptr;\
	}\
	template<typename ty_>\
	ty_* safe_get()\
	{\
		if (holds<ty_>())\
			return reinterpret_cast<ty_*>(ptr());\
		else\
			return nullptr;\
	}

#define _IMP_INVOKE_CASE(N) case type::_##N: return std::invoke(std::forward<fn_##N>(fn##N), _storage.ty##N);
#define _IMP_VISIT_SINGLE_CASE(N) case type::_##N: return std::invoke(std::forward<fn_>(fn), _storage.ty##N);
#define _IMP_TYPENAME_FN(N) typename fn_##N
#define _IMP_VISIT_ARG(N) fn_##N&& fn##N
#define _IMP_VISIT(N)\
	template<_IMP_REPEAT_COMMA(_IMP_TYPENAME_FN, N)>\
	decltype(auto) visit(_IMP_REPEAT_COMMA(_IMP_VISIT_ARG, N)) const\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_INVOKE_CASE, N);\
		default: throw std::bad_variant_access(); /* TODO imp error ? */\
		}\
	}\
	template<_IMP_REPEAT_COMMA(_IMP_TYPENAME_FN, N)>\
	decltype(auto) visit(_IMP_REPEAT_COMMA(_IMP_VISIT_ARG, N))\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_INVOKE_CASE, N);\
		default: throw std::bad_variant_access(); /* TODO imp error ? */\
		}\
	}\
	template<typename fn_>\
	decltype(auto) visit(fn_&& fn) const\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_VISIT_SINGLE_CASE, N);\
		default: throw std::bad_variant_access(); /* TODO imp error ? */\
		}\
	}\
	template<typename fn_>\
	decltype(auto) visit(fn_&& fn)\
	{\
		switch (_type)\
		{\
			_IMP_REPEAT(_IMP_VISIT_SINGLE_CASE, N);\
		default: throw std::bad_variant_access(); /* TODO imp error ? */\
		}\
	}

#define _IMP_TYPE_OF_CASE(N) if constexpr (std::is_same_v<ty_, ty_##N>) return type::_##N;
#define _IMP_GET_TYPE(N)\
	type get_type() const { return _type; }\
	template<typename ty_>\
	static constexpr type type_of()\
	{\
		_IMP_REPEAT(_IMP_TYPE_OF_CASE, N);\
		static_assert(imp::dependent_false_v<ty_>);\
	}

#define _IMP_TYPENAME_TY(N) typename ty_##N
#define _IMP_TYPE_PARAMETER(N) ty_##N
#define _IMP_VARIANT(N)\
	template<_IMP_REPEAT_COMMA(_IMP_TYPENAME_TY, N)>\
	class variant<_IMP_REPEAT_COMMA(_IMP_TYPE_PARAMETER, N)>\
	{\
		_IMP_TYPE_ENUM(N);\
		_IMP_STORAGE(N);\
		_IMP_DEL(N);\
		_IMP_COPY_FROM(N);\
		_IMP_MOVE_FROM(N);\
		_IMP_PTR(N);\
	public:\
		_IMP_CTOR(N);\
		_IMP_HOLDS(N);\
		bool empty() const { return _type == type::none; }\
		_IMP_GET(N);\
		_IMP_VISIT(N);\
		_IMP_GET_TYPE(N);\
	};

    // ###########################################################################
    _IMP_VARIANT(2);
    _IMP_VARIANT(3);
    _IMP_VARIANT(4);
    _IMP_VARIANT(5);
    _IMP_VARIANT(6);
    _IMP_VARIANT(7);
    _IMP_VARIANT(8);
    _IMP_VARIANT(9);
    _IMP_VARIANT(10);
    _IMP_VARIANT(11);
    _IMP_VARIANT(12);
    _IMP_VARIANT(13);
    _IMP_VARIANT(14);
    _IMP_VARIANT(15);
    _IMP_VARIANT(16);
    // ###########################################################################

#undef _IMP_ENUM_DEF
#undef _IMP_TYPE_ENUM
#undef _IMP_STORAGE_DEF
#undef _IMP_STORAGE
#undef _IMP_DEL_CASE
#undef _IMP_DEL
#undef _IMP_COPY_CASE
#undef _IMP_COPY_FROM
#undef _IMP_MOVE_CASE
#undef _IMP_MOVE_FROM
#undef _IMP_PTR_CASE
#undef _IMP_PTR
#undef _IMP_VALUE_CTOR
#undef _IMP_CTOR
#undef _IMP_HOLDS_CHECK
#undef _IMP_HOLDS
#undef _IMP_GET
#undef _IMP_INVOKE_CASE
#undef _IMP_TYPENAME_FN
#undef _IMP_VISIT_ARG
#undef _IMP_VISIT
#undef _IMP_TYPE_OF_CASE
#undef _IMP_GET_TYPE
#undef _IMP_TYPENAME_TY
#undef _IMP_TYPE_PARAMETER
#undef _IMP_VARIANT
}
