#pragma once

#include "imp/type_erasure.hpp"
#include "imp/macros.hpp"

#include <array>

#define _IMP_EXTRACT_UID_COMMA(Type) imp::erase_type<IMP_UNPAREN(Type)>().uid(),
#define _IMP_TYPE_DELIM_ARRAY(GENERATOR, name) \
        constexpr inline auto name##_array = []() \
        { \
            struct _comma_sentinel {}; \
            return std::array{ GENERATOR(_IMP_EXTRACT_UID_COMMA) imp::erase_type<_comma_sentinel>().uid() }; \
        }();

#define _IMP_TYPE_DELIM_COUNT(name) constexpr inline size_t name##_count = name##_array.size() - 1;
#define _IMP_TYPE_DELIM_INDEX(name) \
        constexpr size_t name##_index(imp::type_erasure type) \
        { \
            for (size_t i = 0; i < name##_count; ++i) \
            { \
                if (name##_array[i] == type.uid()) \
                    return i; \
            } \
            throw std::bad_typeid(); /* TODO imp error ? */ \
        } \
        template<typename ty> constexpr size_t name##_index() { return name##_index(imp::erase_type<ty>()); }

#define _IMP_OR_ERASES_TO_IMPL(Type) || imp::erases_to<ty, IMP_UNPAREN(Type)>()
#define _IMP_TYPE_DELIM_CONCEPT(GENERATOR, name) template<typename ty> concept name##_check = false GENERATOR(_IMP_OR_ERASES_TO_IMPL);

#define IMP_TYPE_DELIMITER(GENERATOR, name) \
       _IMP_TYPE_DELIM_ARRAY(GENERATOR, name) \
       _IMP_TYPE_DELIM_COUNT(name) \
       _IMP_TYPE_DELIM_INDEX(name) \
       _IMP_TYPE_DELIM_CONCEPT(GENERATOR, name)
