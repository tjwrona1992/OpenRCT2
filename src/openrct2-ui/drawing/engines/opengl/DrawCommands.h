/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "GLSLTypes.h"
#include "OpenGLAPI.h"
#include "TextureCache.h"

#include <openrct2/common.h>
#include <vector>

template<typename T> class CommandBatch
{
private:
    std::vector<T> _instances;
    size_t _numInstances;

public:
    CommandBatch()
        : _numInstances(0)
    {
    }
    [[nodiscard]] bool empty() const
    {
        return _numInstances == 0;
    }
    void clear()
    {
        _numInstances = 0;
    }
    T& allocate()
    {
        if (_numInstances + 1 > _instances.size())
        {
            _instances.resize((_numInstances + 1) << 1);
        }
        return _instances[_numInstances++];
    }
    T& insert(const T& value)
    {
        if (_numInstances + 1 > _instances.size())
        {
            _instances.resize((_numInstances + 1) << 1);
        }
        return _instances[_numInstances++] = value;
    }
    [[nodiscard]] size_t size() const
    {
        return _numInstances;
    }
    const T* data() const
    {
        return _instances.data();
    }
    const T& operator[](size_t idx) const
    {
        return _instances.at(idx);
    }

    typename std::vector<T>::iterator begin()
    {
        return _instances.begin();
    }
    typename std::vector<T>::const_iterator begin() const
    {
        return _instances.cbegin();
    }
    typename std::vector<T>::const_iterator cbegin() const
    {
        return _instances.cbegin();
    }
    typename std::vector<T>::iterator end()
    {
        return _instances.begin() + _numInstances;
    }
    typename std::vector<T>::const_iterator end() const
    {
        return _instances.cbegin() + _numInstances;
    }
    typename std::vector<T>::const_iterator cend() const
    {
        return _instances.cbegin() + _numInstances;
    }
};

struct DrawLineCommand
{
    ivec4 clip;
    ivec4 bounds;
    GLuint colour;
    GLint depth;
};

// Per-instance data for images
struct DrawRectCommand
{
    ivec4 clip;
    GLint texColourAtlas;
    vec4 texColourBounds;
    GLint texMaskAtlas;
    vec4 texMaskBounds;
    ivec3 palettes;
    GLint flags;
    GLuint colour;
    ivec4 bounds;
    GLint depth;

    enum
    {
        FLAG_NO_TEXTURE = (1U << 2U),
        FLAG_MASK = (1U << 3U),
        FLAG_CROSS_HATCH = (1U << 4U),
    };
};

using LineCommandBatch = CommandBatch<DrawLineCommand>;
using RectCommandBatch = CommandBatch<DrawRectCommand>;
