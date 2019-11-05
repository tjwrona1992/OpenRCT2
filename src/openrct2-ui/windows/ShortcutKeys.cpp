/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Window.h"

#include <openrct2-ui/input/KeyboardShortcuts.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2/config/Config.h>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/localisation/Localisation.h>

constexpr int32_t WW = 420;
constexpr int32_t WH = 280;

constexpr int32_t WW_SC_MAX = 1200;
constexpr int32_t WH_SC_MAX = 800;

// clang-format off
enum WINDOW_SHORTCUT_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_SCROLL,
    WIDX_RESET
};

// 0x9DE48C
static rct_widget window_shortcut_widgets[] = {
    { WWT_FRAME,            0,  0,      WW - 1, 0,      WH - 1,     STR_NONE,                   STR_NONE },
    { WWT_CAPTION,          0,  1,      WW - 2, 1,      14,         STR_SHORTCUTS_TITLE,        STR_WINDOW_TITLE_TIP },
    { WWT_CLOSEBOX,         0,  WW-13,  WW - 3, 2,      13,         STR_CLOSE_X,                STR_CLOSE_WINDOW_TIP },
    { WWT_SCROLL,           0,  4,      WW - 5, 18,     WH - 18,    SCROLL_VERTICAL,            STR_SHORTCUT_LIST_TIP },
    { WWT_BUTTON,           0,  4,      153,    WH-15,  WH - 4,     STR_SHORTCUT_ACTION_RESET,  STR_SHORTCUT_ACTION_RESET_TIP },
    { WIDGETS_END }
};

static void window_shortcut_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_shortcut_resize(rct_window *w);
static void window_shortcut_invalidate(rct_window *w);
static void window_shortcut_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_shortcut_scrollgetsize(rct_window *w, int32_t scrollIndex, int32_t *width, int32_t *height);
static void window_shortcut_scrollmousedown(rct_window *w, int32_t scrollIndex, int32_t x, int32_t y);
static void window_shortcut_scrollmouseover(rct_window *w, int32_t scrollIndex, int32_t x, int32_t y);
static void window_shortcut_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int32_t scrollIndex);

static rct_window_event_list window_shortcut_events = {
    nullptr,
    window_shortcut_mouseup,
    window_shortcut_resize,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_shortcut_scrollgetsize,
    window_shortcut_scrollmousedown,
    nullptr,
    window_shortcut_scrollmouseover,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_shortcut_invalidate,
    window_shortcut_paint,
    window_shortcut_scrollpaint
};

const rct_string_id ShortcutStringIds[SHORTCUT_COUNT] = {
    STR_SHORTCUT_CLOSE_TOP_MOST_WINDOW,
    STR_SHORTCUT_CLOSE_ALL_FLOATING_WINDOWS,
    STR_SHORTCUT_CANCEL_CONSTRUCTION_MODE,
    STR_SHORTCUT_PAUSE_GAME,
    STR_SHORTCUT_ZOOM_VIEW_OUT,
    STR_SHORTCUT_ZOOM_VIEW_IN,
    STR_SHORTCUT_ROTATE_VIEW_CLOCKWISE,
    STR_SHORTCUT_ROTATE_VIEW_ANTICLOCKWISE,
    STR_SHORTCUT_ROTATE_CONSTRUCTION_OBJECT,
    STR_SHORTCUT_UNDERGROUND_VIEW_TOGGLE,
    STR_SHORTCUT_REMOVE_BASE_LAND_TOGGLE,
    STR_SHORTCUT_REMOVE_VERTICAL_LAND_TOGGLE,
    STR_SHORTCUT_SEE_THROUGH_RIDES_TOGGLE,
    STR_SHORTCUT_SEE_THROUGH_SCENERY_TOGGLE,
    STR_SHORTCUT_INVISIBLE_SUPPORTS_TOGGLE,
    STR_SHORTCUT_INVISIBLE_PEOPLE_TOGGLE,
    STR_SHORTCUT_HEIGHT_MARKS_ON_LAND_TOGGLE,
    STR_SHORTCUT_HEIGHT_MARKS_ON_RIDE_TRACKS_TOGGLE,
    STR_SHORTCUT_HEIGHT_MARKS_ON_PATHS_TOGGLE,
    STR_SHORTCUT_ADJUST_LAND,
    STR_SHORTCUT_ADJUST_WATER,
    STR_SHORTCUT_BUILD_SCENERY,
    STR_SHORTCUT_BUILD_PATHS,
    STR_SHORTCUT_BUILD_NEW_RIDE,
    STR_SHORTCUT_SHOW_FINANCIAL_INFORMATION,
    STR_SHORTCUT_SHOW_RESEARCH_INFORMATION,
    STR_SHORTCUT_SHOW_RIDES_LIST,
    STR_SHORTCUT_SHOW_PARK_INFORMATION,
    STR_SHORTCUT_SHOW_GUEST_LIST,
    STR_SHORTCUT_SHOW_STAFF_LIST,
    STR_SHORTCUT_SHOW_RECENT_MESSAGES,
    STR_SHORTCUT_SHOW_MAP,
    STR_SHORTCUT_SCREENSHOT,
    STR_SHORTCUT_REDUCE_GAME_SPEED,
    STR_SHORTCUT_INCREASE_GAME_SPEED,
    STR_SHORTCUT_OPEN_CHEATS_WINDOW,
    STR_SHORTCUT_TOGGLE_VISIBILITY_OF_TOOLBARS,
    STR_SHORTCUT_SCROLL_MAP_UP,
    STR_SHORTCUT_SCROLL_MAP_LEFT,
    STR_SHORTCUT_SCROLL_MAP_DOWN,
    STR_SHORTCUT_SCROLL_MAP_RIGHT,
    STR_SEND_MESSAGE,
    STR_SHORTCUT_QUICK_SAVE_GAME,
    STR_SHORTCUT_SHOW_OPTIONS,
    STR_SHORTCUT_MUTE_SOUND,
    STR_SHORTCUT_WINDOWED_MODE_TOGGLE,
    STR_SHORTCUT_SHOW_MULTIPLAYER,
    STR_SHORTCUT_PAINT_ORIGINAL,
    STR_SHORTCUT_DEBUG_PAINT_TOGGLE,
    STR_SHORTCUT_SEE_THROUGH_PATHS_TOGGLE,
    STR_SHORTCUT_RIDE_CONSTRUCTION_TURN_LEFT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_TURN_RIGHT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_USE_TRACK_DEFAULT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_SLOPE_DOWN,
    STR_SHORTCUT_RIDE_CONSTRUCTION_SLOPE_UP,
    STR_SHORTCUT_RIDE_CONSTRUCTION_CHAIN_LIFT_TOGGLE,
    STR_SHORTCUT_RIDE_CONSTRUCTION_BANK_LEFT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_BANK_RIGHT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_PREVIOUS_TRACK,
    STR_SHORTCUT_RIDE_CONSTRUCTION_NEXT_TRACK,
    STR_SHORTCUT_RIDE_CONSTRUCTION_BUILD_CURRENT,
    STR_SHORTCUT_RIDE_CONSTRUCTION_DEMOLISH_CURRENT,
    STR_LOAD_GAME,
    STR_SHORTCUT_CLEAR_SCENERY,
    STR_SHORTCUT_GRIDLINES_DISPLAY_TOGGLE,
    STR_SHORTCUT_VIEW_CLIPPING,
    STR_SHORTCUT_HIGHLIGHT_PATH_ISSUES_TOGGLE,
    STR_SHORTCUT_OPEN_TILE_INSPECTOR,
    STR_ADVANCE_TO_NEXT_TICK,
    STR_SHORTCUT_OPEN_SCENERY_PICKER,
};
// clang-format on

/**
 *
 *  rct2: 0x006E3884
 */
rct_window* window_shortcut_keys_open()
{
    rct_window* w = window_bring_to_front_by_class(WC_KEYBOARD_SHORTCUT_LIST);
    if (w == nullptr)
    {
        w = window_create_auto_pos(WW, WH, &window_shortcut_events, WC_KEYBOARD_SHORTCUT_LIST, WF_RESIZABLE);

        w->widgets = window_shortcut_widgets;
        w->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_RESET);
        window_init_scroll_widgets(w);

        w->no_list_items = SHORTCUT_COUNT;
        w->selected_list_item = -1;
        w->min_width = WW;
        w->min_height = WH;
        w->max_width = WW_SC_MAX;
        w->max_height = WH_SC_MAX;
    }
    return w;
}

/**
 *
 *  rct2: 0x006E39E4
 */
static void window_shortcut_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_CLOSE:
            window_close(w);
            break;
        case WIDX_RESET:
            keyboard_shortcuts_reset();
            keyboard_shortcuts_save();
            w->Invalidate();
            break;
    }
}

static void window_shortcut_resize(rct_window* w)
{
    window_set_resize(w, w->min_width, w->min_height, w->max_width, w->max_height);
}

static void window_shortcut_invalidate(rct_window* w)
{
    window_shortcut_widgets[WIDX_BACKGROUND].right = w->width - 1;
    window_shortcut_widgets[WIDX_BACKGROUND].bottom = w->height - 1;
    window_shortcut_widgets[WIDX_TITLE].right = w->width - 2;
    window_shortcut_widgets[WIDX_CLOSE].right = w->width - 3;
    window_shortcut_widgets[WIDX_CLOSE].left = w->width - 13;
    window_shortcut_widgets[WIDX_SCROLL].right = w->width - 5;
    window_shortcut_widgets[WIDX_SCROLL].bottom = w->height - 18;
    window_shortcut_widgets[WIDX_RESET].top = w->height - 15;
    window_shortcut_widgets[WIDX_RESET].bottom = w->height - 4;
}

/**
 *
 *  rct2: 0x006E38E0
 */
static void window_shortcut_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    window_draw_widgets(w, dpi);
}

/**
 *
 *  rct2: 0x006E3A07
 */
static void window_shortcut_scrollgetsize(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height)
{
    *height = w->no_list_items * SCROLLABLE_ROW_HEIGHT;
}

/**
 *
 *  rct2: 0x006E3A3E
 */
static void window_shortcut_scrollmousedown(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y)
{
    int32_t selected_item = (y - 1) / SCROLLABLE_ROW_HEIGHT;
    if (selected_item >= w->no_list_items)
        return;

    window_shortcut_change_open(selected_item);
}

/**
 *
 *  rct2: 0x006E3A16
 */
static void window_shortcut_scrollmouseover(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y)
{
    int32_t selected_item = (y - 1) / SCROLLABLE_ROW_HEIGHT;
    if (selected_item >= w->no_list_items)
        return;

    w->selected_list_item = selected_item;

    w->Invalidate();
}

/**
 *
 *  rct2: 0x006E38E6
 */
static void window_shortcut_scrollpaint(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex)
{
    gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ColourMapA[w->colours[1]].mid_light);

    for (int32_t i = 0; i < w->no_list_items; ++i)
    {
        int32_t y = 1 + i * SCROLLABLE_ROW_HEIGHT;
        if (y > dpi->y + dpi->height)
        {
            break;
        }

        if (y + SCROLLABLE_ROW_HEIGHT < dpi->y)
        {
            continue;
        }

        int32_t format = STR_BLACK_STRING;
        if (i == w->selected_list_item)
        {
            format = STR_WINDOW_COLOUR_2_STRINGID;
            gfx_filter_rect(dpi, 0, y - 1, 800, y + (SCROLLABLE_ROW_HEIGHT - 2), PALETTE_DARKEN_1);
        }

        char templateString[128];
        keyboard_shortcuts_format_string(templateString, 128, i);

        set_format_arg(0, rct_string_id, STR_SHORTCUT_ENTRY_FORMAT);
        set_format_arg(2, rct_string_id, ShortcutStringIds[i]);
        set_format_arg(4, rct_string_id, STR_STRING);
        set_format_arg(6, char*, templateString);
        gfx_draw_string_left(dpi, format, gCommonFormatArgs, COLOUR_BLACK, 0, y - 1);
    }
}
