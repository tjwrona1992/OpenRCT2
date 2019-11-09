#include "KeyboardShortcut.hpp"

#include <openrct2/localisation/Localisation.h>
#include <sstream>

namespace OpenRCT2::Input
{
    KeyboardShortcut::KeyboardShortcut(const uint16_t keyCombination, const std::function<void()>& action)
        : _keyCombination(keyCombination)
        , _action(action)
    {
    }

    std::string KeyboardShortcut::GetString()
    {
        std::stringstream ss;
        if (_keyCombination & SHIFT)
        {
            ss << format_string(STR_SHIFT_PLUS, nullptr);
        }
        if (_keyCombination & CTRL)
        {
            ss << format_string(STR_CTRL_PLUS, nullptr);
        }
        if (_keyCombination & ALT)
        {
#if defined __MACOSX__
            ss << format_string(STR_OPTION_PLUS, nullptr);
#else
            ss << format_string(STR_ALT_PLUS, nullptr);
#endif
        }


        return "";
    }

    void KeyboardShortcut::Execute()
    {
        _action();
    }

} // namespace OpenRCT2::Input
