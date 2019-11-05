/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "OpenGLAPI.h"

#include <openrct2/common.h>

#pragma pack(push, 1)

namespace detail
{
    template<typename T_> struct vec2
    {
        using ValueType = T_;

        union
        {
            ValueType x;
            ValueType s;
            ValueType r;
        };
        union
        {
            ValueType y;
            ValueType t;
            ValueType g;
        };
    };

    template struct vec2<GLfloat>;
    template struct vec2<GLint>;

    template<typename T_> struct vec3
    {
        using ValueType = T_;

        union
        {
            ValueType x;
            ValueType s;
            ValueType r;
        };
        union
        {
            ValueType y;
            ValueType t;
            ValueType g;
        };
        union
        {
            ValueType z;
            ValueType p;
            ValueType b;
        };
    };

    template struct vec3<GLfloat>;
    template struct vec3<GLint>;

    template<typename T_> struct vec4
    {
        using ValueType = T_;

        union
        {
            ValueType x;
            ValueType s;
            ValueType r;
        };
        union
        {
            ValueType y;
            ValueType t;
            ValueType g;
        };
        union
        {
            ValueType z;
            ValueType p;
            ValueType b;
        };
        union
        {
            ValueType w;
            ValueType q;
            ValueType a;
        };
    };

    template struct vec4<GLfloat>;
    template struct vec4<GLint>;

} // namespace detail

using vec2 = detail::vec2<GLfloat>;
using ivec2 = detail::vec2<GLint>;

using vec3 = detail::vec3<GLfloat>;
using ivec3 = detail::vec3<GLint>;

using vec4 = detail::vec4<GLfloat>;
using ivec4 = detail::vec4<GLint>;

#pragma pack(pop)
