#pragma once

#include "Key.hpp"
#include <functional>
#include <openrct2/common.h>
#include <SDL.h>
#include <vector>

namespace OpenRCT2::Input
{
    /*!
    \brief Class to represent a keyboard shortcut.

    This class maps a key press to a shortcut action.
    */
    class KeyboardShortcut
    {
    public:
        /*!
        \brief Constructs a keyboard shortcut.

        A keyboard shortcut is composed of a key press and a corresponding action to execute when the key is pressed.

        \param[in] modifiers List of modifiers (SHIFT, CTRL, ALT, etc.) used with the shortcut.
        \param[in] key Key that will trigger the shortcut.
        \param[in] action Action to be performed when the keyboard shortcut is triggered.
        */
        KeyboardShortcut(const std::vector<uint16_t>& modifiers, const uint16_t key, const std::function<void()>& action);

        /*!
        \brief Destructor.

        Use the compiler generated default destructor.
        */
        ~KeyboardShortcut() = default;

        /*!
        \brief Gets the string that corresponds to the key combination.

        \return Key combination string.
        */
        std::string GetString() const;

        /*!
        \brief Executes the action associated with the keyboard shortcut.
        */
        void Execute() const;

    private:
        const std::vector<uint16_t> _modifiers;
        const uint16_t _key;
        const std::function<void()> _action;
    };
} // namespace OpenRCT2::Input
