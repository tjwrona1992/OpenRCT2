/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <SDL_scancode.h>
#include <functional>
#include <fstream>
#include <memory>
#include <openrct2/common.h>
#include <openrct2/world/Location.hpp>
#include <string>
#include <unordered_map>

#define SHIFT 0x100
#define CTRL 0x200
#define ALT 0x400
#define CMD 0x800
#ifdef __MACOSX__
#    define PLATFORM_MODIFIER CMD
#else
#    define PLATFORM_MODIFIER CTRL
#endif

namespace OpenRCT2::Input
{
    using KeyCombination = uint16_t;
    using Action = std::function<void()>;
    using KeyboardShortcutMap = std::unordered_map<KeyCombination, Action>;

    class KeyboardShortcuts
    {
    public:
        KeyboardShortcuts(const std::string& configFile)
            : _shortcuts(Load(configFile))
        {
        }

    private:
        KeyboardShortcutMap Load(const std::string& configFile)
        {
            try
            {
                const std::ifstream fileStream(configFile, std::ifstream::in);
                uint16_t version = fileStream.ReadValue<uint16_t>();
                if (version == KeyboardShortcuts::CURRENT_FILE_VERSION)
                {
                    int32_t numShortcutsInFile = (fileStream.GetLength() - sizeof(uint16_t)) / sizeof(uint16_t);
                    for (int32_t i = 0; i < numShortcutsInFile; i++)
                    {
                        _keys[i] = fileStream.ReadValue<uint16_t>();
                    }
                }
            }
            catch (const std::exception& ex)
            {
                Console::WriteLine("Error reading shortcut keys: %s", ex.what());
            }
        }

        KeyboardShortcutMap _shortcuts;
    };
} // namespace OpenRCT2::Input

enum class Shortcut
{
    CLOSE_TOP_MOST_WINDOW,
    CLOSE_ALL_FLOATING_WINDOWS,
    CANCEL_CONSTRUCTION_MODE,
    PAUSE_GAME,
    ZOOM_VIEW_OUT,
    ZOOM_VIEW_IN,
    ROTATE_VIEW_CLOCKWISE,
    ROTATE_VIEW_ANTICLOCKWISE,
    ROTATE_CONSTRUCTION_OBJECT,
    UNDERGROUND_VIEW_TOGGLE,
    REMOVE_BASE_LAND_TOGGLE,
    REMOVE_VERTICAL_LAND_TOGGLE,
    SEE_THROUGH_RIDES_TOGGLE,
    SEE_THROUGH_SCENERY_TOGGLE,
    INVISIBLE_SUPPORTS_TOGGLE,
    INVISIBLE_PEOPLE_TOGGLE,
    HEIGHT_MARKS_ON_LAND_TOGGLE,
    HEIGHT_MARKS_ON_RIDE_TRACKS_TOGGLE,
    HEIGHT_MARKS_ON_PATHS_TOGGLE,
    ADJUST_LAND,
    ADJUST_WATER,
    BUILD_SCENERY,
    BUILD_PATHS,
    BUILD_NEW_RIDE,
    SHOW_FINANCIAL_INFORMATION,
    SHOW_RESEARCH_INFORMATION,
    SHOW_RIDES_LIST,
    SHOW_PARK_INFORMATION,
    SHOW_GUEST_LIST,
    SHOW_STAFF_LIST,
    SHOW_RECENT_MESSAGES,
    SHOW_MAP,
    SCREENSHOT,

    // New
    REDUCE_GAME_SPEED,
    INCREASE_GAME_SPEED,
    OPEN_CHEAT_WINDOW,
    REMOVE_TOP_BOTTOM_TOOLBAR_TOGGLE,
    SCROLL_MAP_UP,
    SCROLL_MAP_LEFT,
    SCROLL_MAP_DOWN,
    SCROLL_MAP_RIGHT,
    OPEN_CHAT_WINDOW,
    QUICK_SAVE_GAME,
    SHOW_OPTIONS,
    MUTE_SOUND,
    WINDOWED_MODE_TOGGLE,
    SHOW_MULTIPLAYER,
    PAINT_ORIGINAL_TOGGLE,
    DEBUG_PAINT_TOGGLE,
    SEE_THROUGH_PATHS_TOGGLE,
    RIDE_CONSTRUCTION_TURN_LEFT,
    RIDE_CONSTRUCTION_TURN_RIGHT,
    RIDE_CONSTRUCTION_USE_TRACK_DEFAULT,
    RIDE_CONSTRUCTION_SLOPE_DOWN,
    RIDE_CONSTRUCTION_SLOPE_UP,
    RIDE_CONSTRUCTION_CHAIN_LIFT_TOGGLE,
    RIDE_CONSTRUCTION_BANK_LEFT,
    RIDE_CONSTRUCTION_BANK_RIGHT,
    RIDE_CONSTRUCTION_PREVIOUS_TRACK,
    RIDE_CONSTRUCTION_NEXT_TRACK,
    RIDE_CONSTRUCTION_BUILD_CURRENT,
    RIDE_CONSTRUCTION_DEMOLISH_CURRENT,
    LOAD_GAME,
    CLEAR_SCENERY,
    GRIDLINES_DISPLAY_TOGGLE,
    VIEW_CLIPPING,
    HIGHLIGHT_PATH_ISSUES_TOGGLE,
    TILE_INSPECTOR,
    ADVANCE_TO_NEXT_TICK,
    SCENERY_PICKER,

    COUNT,

    UNDEFINED = 0xFFFF,
};

// namespace OpenRCT2
//{
//    interface IPlatformEnvironment;
//
//    namespace Input
//    {
//        class KeyboardShortcuts
//        {
//        private:
//            constexpr static int32_t CURRENT_FILE_VERSION = 1;
//            static const uint16_t DefaultKeys[SHORTCUT_COUNT];
//
//            std::shared_ptr<IPlatformEnvironment> const _env;
//            uint16_t _keys[SHORTCUT_COUNT];
//
//        public:
//            KeyboardShortcuts(const std::shared_ptr<IPlatformEnvironment>& env);
//            ~KeyboardShortcuts();
//
//            void Reset();
//            bool Load();
//            bool Save();
//
//            std::string GetShortcutString(int32_t shortcut) const;
//
//            void Set(int32_t key);
//            Shortcut GetFromKey(int32_t key);
//            ScreenCoordsXY GetKeyboardMapScroll(const uint8_t* keysState) const;
//        };
//    } // namespace Input
//} // namespace OpenRCT2

/** The current shortcut being changed. */
extern uint8_t gKeyboardShortcutChangeId;
//extern const rct_string_id ShortcutStringIds[SHORTCUT_COUNT];

void keyboard_shortcuts_reset();
bool keyboard_shortcuts_load();
bool keyboard_shortcuts_save();
void keyboard_shortcuts_set(int32_t key);
Shortcut keyboard_shortcuts_get_from_key(int32_t key);
void keyboard_shortcuts_format_string(char* buffer, size_t bufferSize, int32_t shortcut);

void keyboard_shortcut_handle(int32_t key);
void keyboard_shortcut_handle_command(Shortcut shortcut);

ScreenCoordsXY get_keyboard_map_scroll(const uint8_t* keysState);
