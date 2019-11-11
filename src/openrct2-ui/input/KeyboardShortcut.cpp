#include "KeyboardShortcut.hpp"

#include <SDL.h>
#include <openrct2/localisation/Localisation.h>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace OpenRCT2::Input
{
    static const std::unordered_map<const SDL_Scancode, const rct_string_id> gLocalizationMap = {
        { SDL_SCANCODE_BACKSPACE, STR_SHORTCUT_BACKSPACE },
        { SDL_SCANCODE_TAB, STR_SHORTCUT_TAB },
        { SDL_SCANCODE_CLEAR, STR_SHORTCUT_CLEAR },
        { SDL_SCANCODE_RETURN, STR_SHORTCUT_RETURN },
        { SDL_SCANCODE_LALT, STR_SHORTCUT_ALT },
        { SDL_SCANCODE_PAUSE, STR_SHORTCUT_PAUSE },
        { SDL_SCANCODE_CAPSLOCK, STR_SHORTCUT_CAPS },
        { SDL_SCANCODE_ESCAPE, STR_SHORTCUT_ESCAPE },
        { SDL_SCANCODE_SPACE, STR_SHORTCUT_SPACEBAR },
        { SDL_SCANCODE_PAGEUP, STR_SHORTCUT_PGUP },
        { SDL_SCANCODE_PAGEDOWN, STR_SHORTCUT_PGDN },
        { SDL_SCANCODE_END, STR_SHORTCUT_END },
        { SDL_SCANCODE_HOME, STR_SHORTCUT_HOME },
        { SDL_SCANCODE_LEFT, STR_SHORTCUT_LEFT },
        { SDL_SCANCODE_UP, STR_SHORTCUT_UP },
        { SDL_SCANCODE_RIGHT, STR_SHORTCUT_RIGHT },
        { SDL_SCANCODE_DOWN, STR_SHORTCUT_DOWN },
        { SDL_SCANCODE_SELECT, STR_SHORTCUT_SELECT },
        { SDL_SCANCODE_PRINTSCREEN, STR_SHORTCUT_PRINT },
        { SDL_SCANCODE_EXECUTE, STR_SHORTCUT_EXECUTE },
        { SDL_SCANCODE_SYSREQ, STR_SHORTCUT_SNAPSHOT },
        { SDL_SCANCODE_INSERT, STR_SHORTCUT_INSERT },
        { SDL_SCANCODE_DELETE, STR_SHORTCUT_DELETE },
        { SDL_SCANCODE_HELP, STR_SHORTCUT_HELP },
        { SDL_SCANCODE_APPLICATION, STR_SHORTCUT_MENU },
        { SDL_SCANCODE_KP_0, STR_SHORTCUT_NUMPAD_0 },
        { SDL_SCANCODE_KP_1, STR_SHORTCUT_NUMPAD_1 },
        { SDL_SCANCODE_KP_2, STR_SHORTCUT_NUMPAD_2 },
        { SDL_SCANCODE_KP_3, STR_SHORTCUT_NUMPAD_3 },
        { SDL_SCANCODE_KP_4, STR_SHORTCUT_NUMPAD_4 },
        { SDL_SCANCODE_KP_5, STR_SHORTCUT_NUMPAD_5 },
        { SDL_SCANCODE_KP_6, STR_SHORTCUT_NUMPAD_6 },
        { SDL_SCANCODE_KP_7, STR_SHORTCUT_NUMPAD_7 },
        { SDL_SCANCODE_KP_8, STR_SHORTCUT_NUMPAD_8 },
        { SDL_SCANCODE_KP_9, STR_SHORTCUT_NUMPAD_9 },
        { SDL_SCANCODE_KP_MULTIPLY, STR_SHORTCUT_NUMPAD_MULTIPLY },
        { SDL_SCANCODE_KP_PLUS, STR_SHORTCUT_NUMPAD_PLUS },
        { SDL_SCANCODE_KP_MINUS, STR_SHORTCUT_NUMPAD_MINUS },
        { SDL_SCANCODE_KP_PERIOD, STR_SHORTCUT_NUMPAD_PERIOD },
        { SDL_SCANCODE_KP_DIVIDE, STR_SHORTCUT_NUMPAD_DIVIDE },
        { SDL_SCANCODE_NUMLOCKCLEAR, STR_SHORTCUT_NUMLOCK },
        { SDL_SCANCODE_SCROLLLOCK, STR_SHORTCUT_SCROLL },
    };

    // Converts an SDL_Scancode to the corresponding localized string
    static std::string ScancodeToString(const SDL_Scancode key)
    {
        auto it = gLocalizationMap.find(key);
        if (it != gLocalizationMap.end())
        {
            return format_string(it->second, nullptr);
        }
        else
        {
            return SDL_GetKeyName(SDL_GetKeyFromScancode(key));
        }
    }

    KeyboardShortcut::KeyboardShortcut(const std::vector<uint16_t>& modifiers, const uint16_t key, const std::function<void()>& action)
        : _modifiers(modifiers)
        , _key(key)
        , _action(action)
    {
    }

    std::string KeyboardShortcut::GetString() const
    {
        std::stringstream ss;
        for (auto it = _keys.begin(); it != _keys.end(); ++it)
        {
            std::string keyName;
            
            std::next(it) != _keys.end()
                ? ss << ScancodeToString(*it) << " + "
                : ss << ScancodeToString(*it);
        }

        return ss.str();
    }

    void KeyboardShortcut::Execute() const
    {
        _action();
    }

} // namespace OpenRCT2::Input
