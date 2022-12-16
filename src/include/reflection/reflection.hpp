
#pragma once

#include "field_reflection.hpp"
#include "function_reflection.hpp"
#include "class_reflection.hpp"

#define REFLECT_FUNCTIONS_INFO()                                                         \
    static auto const reflected_functions()                                              \
    {                                                                                    \
        static constexpr auto map =                                                      \
            refl::to_frozen_map<refl::gather_functions,                                  \
                                this_type,                                               \
                                refl::func_counts<this_type>>::make_map();               \
        return &map;                                                                     \
    }

#define REFLECT_FIELDS_INFO()                                                            \
    static auto const reflected_fields()                                                 \
    {                                                                                    \
        static constexpr auto map =                                                      \
            refl::to_frozen_map<refl::gather_fields,                                     \
                                this_type,                                               \
                                refl::field_counts<this_type>>::make_map();              \
        return &map;                                                                     \
    }

#define REFLECT_CLASS_INFO()                                                             \
    static auto const reflected_class()                                                  \
    {                                                                                    \
        static constexpr auto info = refl::rclass_t::make_class<this_type>();            \
        return &info;                                                                    \
    }

#define REFLECT_CLASS()                                                                  \
    DECLARE_TYPE();                                                                      \
                                                                                         \
private:                                                                                 \
    REFLECT_CLASS_INFO();                                                                \
    REFLECT_FIELDS_INFO();                                                               \
    REFLECT_FUNCTIONS_INFO();
