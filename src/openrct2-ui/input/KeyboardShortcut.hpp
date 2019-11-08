#pragma once

#include <functional>
#include <openrct2/common.h>

namespace OpenRCT2::Input
{
    /*!
    \brief Class to represent a keyboard shortcut

    This class maps a key combination string to 
    */
    class KeyboardShortcut
    {
    public:
        KeyboardShortcut(const uint16_t keyCombination, const rct_string_id id, const std::function<void()>& action);
        ~KeyboardShortcut() = default;

        uint16_t GetKeyCombination();
        rct_string_id GetId();
        std::function<void()> GetAction();
        void Execute();

    private:
        const uint16_t _keyCombination;
        const rct_string_id _id;
        const std::function<void()> _action;
    };
} // namespace OpenRCT2::Input
