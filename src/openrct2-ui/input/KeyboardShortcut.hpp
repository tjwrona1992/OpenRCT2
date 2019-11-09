#pragma once

#include <functional>
#include <openrct2/common.h>

namespace OpenRCT2::Input
{
    /*!
    \brief Class to represent a keyboard shortcut

    This class maps a key combination to a shortcut action.
    */
    class KeyboardShortcut
    {
    public:
        KeyboardShortcut(const uint16_t keyCombination, const std::function<void()>& action);
        ~KeyboardShortcut() = default;

        std::string GetString();
        void Execute();

    private:
        const uint16_t _keyCombination;
        const std::function<void()> _action;
    };
} // namespace OpenRCT2::Input
