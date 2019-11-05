/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Window.h"

#include "../Context.h"
#include "../Editor.h"
#include "../Game.h"
#include "../Input.h"
#include "../OpenRCT2.h"
#include "../audio/audio.h"
#include "../config/Config.h"
#include "../core/Guard.hpp"
#include "../drawing/Drawing.h"
#include "../interface/Cursors.h"
#include "../localisation/Localisation.h"
#include "../localisation/StringIds.h"
#include "../platform/platform.h"
#include "../scenario/Scenario.h"
#include "../sprites.h"
#include "../ui/UiContext.h"
#include "../ui/WindowManager.h"
#include "../world/Map.h"
#include "../world/Sprite.h"
#include "Viewport.h"
#include "Widget.h"
#include "Window_internal.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <list>

std::list<std::shared_ptr<rct_window>> g_window_list;
rct_window* gWindowAudioExclusive;

uint16_t TextInputDescriptionArgs[4];
widget_identifier gCurrentTextBox = { { 255, 0 }, 0 };
char gTextBoxInput[TEXT_INPUT_SIZE] = { 0 };
int32_t gMaxTextBoxInputLength = 0;
int32_t gTextBoxFrameNo = 0;
bool gUsingWidgetTextBox = false;
TextInputSession* gTextInput;

uint16_t gWindowUpdateTicks;
uint16_t gWindowMapFlashingFlags;
colour_t gCurrentWindowColours[4];

// converted from uint16_t values at 0x009A41EC - 0x009A4230
// these are percentage coordinates of the viewport to centre to, if a window is obscuring a location, the next is tried
// clang-format off
static constexpr const float window_scroll_locations[][2] = {
    { 0.5f, 0.5f },
    { 0.75f, 0.5f },
    { 0.25f, 0.5f },
    { 0.5f, 0.75f },
    { 0.5f, 0.25f },
    { 0.75f, 0.75f },
    { 0.75f, 0.25f },
    { 0.25f, 0.75f },
    { 0.25f, 0.25f },
    { 0.125f, 0.5f },
    { 0.875f, 0.5f },
    { 0.5f, 0.125f },
    { 0.5f, 0.875f },
    { 0.875f, 0.125f },
    { 0.875f, 0.875f },
    { 0.125f, 0.875f },
    { 0.125f, 0.125f },
};
// clang-format on

namespace WindowCloseFlags
{
    static constexpr uint32_t None = 0;
    static constexpr uint32_t IterateReverse = (1 << 0);
    static constexpr uint32_t CloseSingle = (1 << 1);
} // namespace WindowCloseFlags

static int32_t window_draw_split(
    rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom);
static void window_draw_single(rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom);

std::list<std::shared_ptr<rct_window>>::iterator window_get_iterator(const rct_window* w)
{
    return std::find_if(g_window_list.begin(), g_window_list.end(), [w](const std::shared_ptr<rct_window>& w2) -> bool {
        return w == w2.get();
    });
}

void window_visit_each(std::function<void(rct_window*)> func)
{
    auto windowList = g_window_list;
    for (auto& w : windowList)
    {
        func(w.get());
    }
}

/**
 *
 *  rct2: 0x006ED7B0
 */
void window_dispatch_update_all()
{
    // gTooltipNotShownTicks++;
    window_visit_each([&](rct_window* w) { window_event_update_call(w); });
}

void window_update_all_viewports()
{
    window_visit_each([&](rct_window* w) {
        if (w->viewport != nullptr && window_is_visible(w))
        {
            viewport_update_position(w);
        }
    });
}

/**
 *
 *  rct2: 0x006E77A1
 */
void window_update_all()
{
    // gfx_draw_all_dirty_blocks();
    // window_update_all_viewports();
    // gfx_draw_all_dirty_blocks();

    // 1000 tick update
    gWindowUpdateTicks += gCurrentDeltaTime;
    if (gWindowUpdateTicks >= 1000)
    {
        gWindowUpdateTicks = 0;

        window_visit_each([](rct_window* w) { window_event_periodic_update_call(w); });
    }

    // Border flash invalidation
    window_visit_each([](rct_window* w) {
        if (w->flags & WF_WHITE_BORDER_MASK)
        {
            w->flags -= WF_WHITE_BORDER_ONE;
            if (!(w->flags & WF_WHITE_BORDER_MASK))
            {
                w->Invalidate();
            }
        }
    });

    auto windowManager = OpenRCT2::GetContext()->GetUiContext()->GetWindowManager();
    windowManager->UpdateMouseWheel();
}

static void window_close_surplus(int32_t cap, int8_t avoid_classification)
{
    // find the amount of windows that are currently open
    auto count = (int32_t)g_window_list.size();
    // difference between amount open and cap = amount to close
    auto diff = count - WINDOW_LIMIT_RESERVED - cap;
    for (auto i = 0; i < diff; i++)
    {
        // iterates through the list until it finds the newest window, or a window that can be closed
        rct_window* foundW{};
        for (auto& w : g_window_list)
        {
            if (!(w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT | WF_NO_AUTO_CLOSE)))
            {
                foundW = w.get();
                break;
            }
        }
        // skip window if window matches specified rct_windowclass (as user may be modifying via options)
        if (avoid_classification != -1 && foundW != nullptr && foundW->classification == avoid_classification)
        {
            continue;
        }
        window_close(foundW);
    }
}

/*
 * Changes the maximum amount of windows allowed
 */
void window_set_window_limit(int32_t value)
{
    int32_t prev = gConfigGeneral.window_limit;
    int32_t val = std::clamp(value, WINDOW_LIMIT_MIN, WINDOW_LIMIT_MAX);
    gConfigGeneral.window_limit = val;
    config_save_default();
    // Checks if value decreases and then closes surplus
    // windows if one sets a limit lower than the number of windows open
    if (val < prev)
    {
        window_close_surplus(val, WC_OPTIONS);
    }
}

/**
 * Closes the specified window.
 *  rct2: 0x006ECD4C
 *
 * @param window The window to close (esi).
 */
void window_close(rct_window* w)
{
    auto itWindow = window_get_iterator(w);
    if (itWindow == g_window_list.end())
        return;

    // Explicit copy of the shared ptr to keep the memory valid.
    std::shared_ptr<rct_window> window = *itWindow;

    window_event_close_call(window.get());

    // Remove viewport
    if (window->viewport != nullptr)
    {
        window->viewport->width = 0;
        window->viewport = nullptr;
    }

    // Invalidate the window (area)
    window->Invalidate();

    // The window list may have been modified in the close event
    itWindow = window_get_iterator(w);
    if (itWindow != g_window_list.end())
        g_window_list.erase(itWindow);
}

template<typename _TPred> static void window_close_by_condition(_TPred pred, uint32_t flags = WindowCloseFlags::None)
{
    bool listUpdated;
    do
    {
        listUpdated = false;

        auto closeSingle = [&](std::shared_ptr<rct_window> window) -> bool {
            if (!pred(window.get()))
            {
                return false;
            }

            // Keep track of current amount, if a new window is created upon closing
            // we need to break this current iteration and restart.
            size_t previousCount = g_window_list.size();

            window_close(window.get());

            if ((flags & WindowCloseFlags::CloseSingle) != 0)
            {
                // Only close a single one.
                return true;
            }

            if (previousCount >= g_window_list.size())
            {
                // A new window was created during the close event.
                return true;
            }

            // Keep closing windows.
            return false;
        };

        // The closest to something like for_each_if is using find_if in order to avoid duplicate code
        // to change the loop direction.
        auto windowList = g_window_list;
        if ((flags & WindowCloseFlags::IterateReverse) != 0)
            listUpdated = std::find_if(windowList.rbegin(), windowList.rend(), closeSingle) != windowList.rend();
        else
            listUpdated = std::find_if(windowList.begin(), windowList.end(), closeSingle) != windowList.end();

        // If requested to close only a single window and a new window was created during close
        // we ignore it.
        if ((flags & WindowCloseFlags::CloseSingle) != 0)
            break;

    } while (listUpdated);
}

/**
 * Closes all windows with the specified window class.
 *  rct2: 0x006ECCF4
 * @param cls (cl) with bit 15 set
 */
void window_close_by_class(rct_windowclass cls)
{
    window_close_by_condition([&](rct_window* w) -> bool { return w->classification == cls; });
}

/**
 * Closes all windows with specified window class and number.
 *  rct2: 0x006ECCF4
 * @param cls (cl) without bit 15 set
 * @param number (dx)
 */
void window_close_by_number(rct_windowclass cls, rct_windownumber number)
{
    window_close_by_condition([cls, number](rct_window* w) -> bool { return w->classification == cls && w->number == number; });
}

/**
 * Finds the first window with the specified window class.
 *  rct2: 0x006EA8A0
 * @param cls (cl) with bit 15 set
 * @returns the window or NULL if no window was found.
 */
rct_window* window_find_by_class(rct_windowclass cls)
{
    for (auto& w : g_window_list)
    {
        if (w->classification == cls)
        {
            return w.get();
        }
    }
    return nullptr;
}

/**
 * Finds the first window with the specified window class and number.
 *  rct2: 0x006EA8A0
 * @param cls (cl) without bit 15 set
 * @param number (dx)
 * @returns the window or NULL if no window was found.
 */
rct_window* window_find_by_number(rct_windowclass cls, rct_windownumber number)
{
    for (auto& w : g_window_list)
    {
        if (w->classification == cls && w->number == number)
        {
            return w.get();
        }
    }
    return nullptr;
}

/**
 * Closes the top-most window
 *
 *  rct2: 0x006E403C
 */
void window_close_top()
{
    window_close_by_class(WC_DROPDOWN);

    if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
    {
        if (gS6Info.editor_step != EDITOR_STEP_LANDSCAPE_EDITOR)
            return;
    }

    auto pred = [](rct_window* w) -> bool { return !(w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT)); };
    window_close_by_condition(pred, WindowCloseFlags::CloseSingle | WindowCloseFlags::IterateReverse);
}

/**
 * Closes all open windows
 *
 *  rct2: 0x006EE927
 */
void window_close_all()
{
    window_close_by_class(WC_DROPDOWN);
    window_close_by_condition([](rct_window* w) -> bool { return !(w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT)); });
}

void window_close_all_except_class(rct_windowclass cls)
{
    window_close_by_class(WC_DROPDOWN);

    window_close_by_condition([cls](rct_window* w) -> bool {
        return w->classification != cls && !(w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT));
    });
}

/**
 * Closes all windows, save for those having any of the passed flags.
 */
void window_close_all_except_flags(uint16_t flags)
{
    window_close_by_condition([flags](rct_window* w) -> bool { return !(w->flags & flags); });
}

/**
 *
 *  rct2: 0x006EA845
 */
rct_window* window_find_from_point(ScreenCoordsXY screenCoords)
{
    for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
    {
        auto& w = *it;
        if (screenCoords.x < w->x || screenCoords.x >= w->x + w->width || screenCoords.y < w->y
            || screenCoords.y >= w->y + w->height)
            continue;

        if (w->flags & WF_NO_BACKGROUND)
        {
            auto widgetIndex = window_find_widget_from_point(w.get(), screenCoords);
            if (widgetIndex == -1)
                continue;
        }

        return w.get();
    }

    return nullptr;
}

/**
 *
 *  rct2: 0x006EA594
 * x (ax)
 * y (bx)
 * returns widget_index (edx)
 * EDI NEEDS TO BE SET TO w->widgets[widget_index] AFTER
 */
rct_widgetindex window_find_widget_from_point(rct_window* w, ScreenCoordsXY screenCoords)
{
    // Invalidate the window
    window_event_invalidate_call(w);

    // Find the widget at point x, y
    rct_widgetindex widget_index = -1;
    for (int32_t i = 0;; i++)
    {
        rct_widget* widget = &w->widgets[i];
        if (widget->type == WWT_LAST)
        {
            break;
        }
        else if (widget->type != WWT_EMPTY)
        {
            if (screenCoords.x >= w->x + widget->left && screenCoords.x <= w->x + widget->right
                && screenCoords.y >= w->y + widget->top && screenCoords.y <= w->y + widget->bottom)
            {
                widget_index = i;
            }
        }
    }

    // Return next widget if a dropdown
    if (widget_index != -1)
        if (w->widgets[widget_index].type == WWT_DROPDOWN)
            widget_index++;

    // Return the widget index
    return widget_index;
}

/**
 * Invalidates the specified window.
 *  rct2: 0x006EB13A
 *
 * @param window The window to invalidate (esi).
 */
template<typename _TPred> static void window_invalidate_by_condition(_TPred pred)
{
    window_visit_each([pred](rct_window* w) {
        if (pred(w))
        {
            w->Invalidate();
        }
    });
}

/**
 * Invalidates all windows with the specified window class.
 *  rct2: 0x006EC3AC
 * @param cls (al) with bit 14 set
 */
void window_invalidate_by_class(rct_windowclass cls)
{
    window_invalidate_by_condition([cls](rct_window* w) -> bool { return w->classification == cls; });
}

/**
 * Invalidates all windows with the specified window class and number.
 *  rct2: 0x006EC3AC
 */
void window_invalidate_by_number(rct_windowclass cls, rct_windownumber number)
{
    window_invalidate_by_condition(
        [cls, number](rct_window* w) -> bool { return w->classification == cls && w->number == number; });
}

/**
 * Invalidates all windows.
 */
void window_invalidate_all()
{
    window_visit_each([](rct_window* w) { w->Invalidate(); });
}

/**
 * Invalidates the specified widget of a window.
 *  rct2: 0x006EC402
 */
void widget_invalidate(rct_window* w, rct_widgetindex widgetIndex)
{
    rct_widget* widget;

    assert(w != nullptr);
#ifdef DEBUG
    for (int32_t i = 0; i <= widgetIndex; i++)
    {
        assert(w->widgets[i].type != WWT_LAST);
    }
#endif

    widget = &w->widgets[widgetIndex];
    if (widget->left == -2)
        return;

    gfx_set_dirty_blocks(w->x + widget->left, w->y + widget->top, w->x + widget->right + 1, w->y + widget->bottom + 1);
}

template<typename _TPred> static void widget_invalidate_by_condition(_TPred pred)
{
    window_visit_each([pred](rct_window* w) {
        if (pred(w))
        {
            w->Invalidate();
        }
    });
}

/**
 * Invalidates the specified widget of all windows that match the specified window class.
 */
void widget_invalidate_by_class(rct_windowclass cls, rct_widgetindex widgetIndex)
{
    window_visit_each([cls, widgetIndex](rct_window* w) {
        if (w->classification == cls)
        {
            widget_invalidate(w, widgetIndex);
        }
    });
}

/**
 * Invalidates the specified widget of all windows that match the specified window class and number.
 *  rct2: 0x006EC3AC
 */
void widget_invalidate_by_number(rct_windowclass cls, rct_windownumber number, rct_widgetindex widgetIndex)
{
    window_visit_each([cls, number, widgetIndex](rct_window* w) {
        if (w->classification == cls && w->number == number)
        {
            widget_invalidate(w, widgetIndex);
        }
    });
}

/**
 *
 *  rct2: 0x006EAE4E
 *
 * @param w The window (esi).
 */
void window_update_scroll_widgets(rct_window* w)
{
    int32_t scrollIndex, width, height, scrollPositionChanged;
    rct_widgetindex widgetIndex;
    rct_scroll* scroll;
    rct_widget* widget;

    widgetIndex = 0;
    scrollIndex = 0;
    assert(w != nullptr);
    for (widget = w->widgets; widget->type != WWT_LAST; widget++, widgetIndex++)
    {
        if (widget->type != WWT_SCROLL)
            continue;

        scroll = &w->scrolls[scrollIndex];
        width = 0;
        height = 0;
        window_get_scroll_size(w, scrollIndex, &width, &height);
        if (height == 0)
        {
            scroll->v_top = 0;
        }
        else if (width == 0)
        {
            scroll->h_left = 0;
        }
        width++;
        height++;

        scrollPositionChanged = 0;
        if ((widget->content & SCROLL_HORIZONTAL) && width != scroll->h_right)
        {
            scrollPositionChanged = 1;
            scroll->h_right = width;
        }

        if ((widget->content & SCROLL_VERTICAL) && height != scroll->v_bottom)
        {
            scrollPositionChanged = 1;
            scroll->v_bottom = height;
        }

        if (scrollPositionChanged)
        {
            widget_scroll_update_thumbs(w, widgetIndex);
            w->Invalidate();
        }
        scrollIndex++;
    }
}

int32_t window_get_scroll_data_index(rct_window* w, rct_widgetindex widget_index)
{
    int32_t i, result;

    result = 0;
    assert(w != nullptr);
    for (i = 0; i < widget_index; i++)
    {
        if (w->widgets[i].type == WWT_SCROLL)
            result++;
    }
    return result;
}

/**
 *
 *  rct2: 0x006ECDA4
 */
rct_window* window_bring_to_front(rct_window* w)
{
    if (!(w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT)))
    {
        auto itSourcePos = window_get_iterator(w);
        if (itSourcePos != g_window_list.end())
        {
            // Insert in front of the first non-stick-to-front window
            auto itDestPos = g_window_list.begin();
            for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
            {
                auto& w2 = *it;
                if (!(w2->flags & WF_STICK_TO_FRONT))
                {
                    itDestPos = it.base();
                    break;
                }
            }

            g_window_list.splice(itDestPos, g_window_list, itSourcePos);
            w->Invalidate();

            if (w->x + w->width < 20)
            {
                int32_t i = 20 - w->x;
                w->x += i;
                if (w->viewport != nullptr)
                    w->viewport->x += i;
                w->Invalidate();
            }
        }
    }
    return w;
}

rct_window* window_bring_to_front_by_class_with_flags(rct_windowclass cls, uint16_t flags)
{
    rct_window* w;

    w = window_find_by_class(cls);
    if (w != nullptr)
    {
        w->flags |= flags;
        w->Invalidate();
        w = window_bring_to_front(w);
    }

    return w;
}

rct_window* window_bring_to_front_by_class(rct_windowclass cls)
{
    return window_bring_to_front_by_class_with_flags(cls, WF_WHITE_BORDER_MASK);
}

/**
 *
 *  rct2: 0x006ED78A
 * cls (cl)
 * number (dx)
 */
rct_window* window_bring_to_front_by_number(rct_windowclass cls, rct_windownumber number)
{
    rct_window* w;

    w = window_find_by_number(cls, number);
    if (w != nullptr)
    {
        w->flags |= WF_WHITE_BORDER_MASK;
        w->Invalidate();
        w = window_bring_to_front(w);
    }

    return w;
}

/**
 *
 *  rct2: 0x006EE65A
 */
void window_push_others_right(rct_window* window)
{
    window_visit_each([window](rct_window* w) {
        if (w == window)
            return;
        if (w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT))
            return;
        if (w->x >= window->x + window->width)
            return;
        if (w->x + w->width <= window->x)
            return;
        if (w->y >= window->y + window->height)
            return;
        if (w->y + w->height <= window->y)
            return;

        w->Invalidate();
        if (window->x + window->width + 13 >= context_get_width())
            return;
        uint16_t push_amount = window->x + window->width - w->x + 3;
        w->x += push_amount;
        w->Invalidate();
        if (w->viewport != nullptr)
            w->viewport->x += push_amount;
    });
}

/**
 *
 *  rct2: 0x006EE6EA
 */
void window_push_others_below(rct_window* w1)
{
    // Enumerate through all other windows
    window_visit_each([w1](rct_window* w2) {
        if (w1 == w2)
            return;
        // ?
        if (w2->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT))
            return;
        // Check if w2 intersects with w1
        if (w2->x > (w1->x + w1->width) || w2->x + w2->width < w1->x)
            return;
        if (w2->y > (w1->y + w1->height) || w2->y + w2->height < w1->y)
            return;

        // Check if there is room to push it down
        if (w1->y + w1->height + 80 >= context_get_height())
            return;

        // Invalidate the window's current area
        w2->Invalidate();

        int32_t push_amount = w1->y + w1->height - w2->y + 3;
        w2->y += push_amount;

        // Invalidate the window's new area
        w2->Invalidate();

        // Update viewport position if necessary
        if (w2->viewport != nullptr)
            w2->viewport->y += push_amount;
    });
}

/**
 *
 *  rct2: 0x006EE2E4
 */
rct_window* window_get_main()
{
    for (auto& w : g_window_list)
    {
        if (w->classification == WC_MAIN_WINDOW)
        {
            return w.get();
        }
    }
    return nullptr;
}

/**
 *
 *  rct2: 0x006E7C9C
 * @param w (esi)
 * @param x (eax)
 * @param y (ecx)
 * @param z (edx)
 */
void window_scroll_to_location(rct_window* w, int32_t x, int32_t y, int32_t z)
{
    CoordsXYZ location_3d = { x, y, z };

    assert(w != nullptr);

    window_unfollow_sprite(w);

    if (w->viewport)
    {
        int16_t height = tile_element_height({ x, y });
        if (z < height - 16)
        {
            if (!(w->viewport->flags & 1 << 0))
            {
                w->viewport->flags |= 1 << 0;
                w->Invalidate();
            }
        }
        else
        {
            if (w->viewport->flags & 1 << 0)
            {
                w->viewport->flags &= ~(1 << 0);
                w->Invalidate();
            }
        }

        auto screenCoords = translate_3d_to_2d_with_z(get_current_rotation(), location_3d);

        int32_t i = 0;
        if (!(gScreenFlags & SCREEN_FLAGS_TITLE_DEMO))
        {
            bool found = false;
            while (!found)
            {
                int16_t x2 = w->viewport->x + (int16_t)(w->viewport->width * window_scroll_locations[i][0]);
                int16_t y2 = w->viewport->y + (int16_t)(w->viewport->height * window_scroll_locations[i][1]);

                auto it = window_get_iterator(w);
                for (; it != g_window_list.end(); it++)
                {
                    auto w2 = (*it).get();
                    int16_t x1 = w2->x - 10;
                    int16_t y1 = w2->y - 10;
                    if (x2 >= x1 && x2 <= w2->width + x1 + 20)
                    {
                        if (y2 >= y1 && y2 <= w2->height + y1 + 20)
                        {
                            // window is covering this area, try the next one
                            i++;
                            found = false;
                            break;
                        }
                    }
                }
                if (it == g_window_list.end())
                {
                    found = true;
                }
                if (i >= (int32_t)std::size(window_scroll_locations))
                {
                    i = 0;
                    found = true;
                }
            }
        }
        // rct2: 0x006E7C76
        if (w->viewport_target_sprite == SPRITE_INDEX_NULL)
        {
            if (!(w->flags & WF_NO_SCROLLING))
            {
                w->saved_view_x = screenCoords.x - (int16_t)(w->viewport->view_width * window_scroll_locations[i][0]);
                w->saved_view_y = screenCoords.y - (int16_t)(w->viewport->view_height * window_scroll_locations[i][1]);
                w->flags |= WF_SCROLLING_TO_LOCATION;
            }
        }
    }
}

/**
 *
 *  rct2: 0x00688956
 */
static void call_event_viewport_rotate_on_all_windows()
{
    window_visit_each([](rct_window* w) { window_event_viewport_rotate_call(w); });
}

/**
 *
 *  rct2: 0x0068881A
 * direction can be used to alter the camera rotation:
 *      1: clockwise
 *      -1: anti-clockwise
 */
void window_rotate_camera(rct_window* w, int32_t direction)
{
    rct_viewport* viewport = w->viewport;
    if (viewport == nullptr)
        return;

    int16_t x = (viewport->width >> 1) + viewport->x;
    int16_t y = (viewport->height >> 1) + viewport->y;
    int16_t z;

    // has something to do with checking if middle of the viewport is obstructed
    rct_viewport* other;
    screen_get_map_xy(x, y, &x, &y, &other);

    // other != viewport probably triggers on viewports in ride or guest window?
    // x is LOCATION_NULL if middle of viewport is obstructed by another window?
    if (x == LOCATION_NULL || other != viewport)
    {
        x = (viewport->view_width >> 1) + viewport->view_x;
        y = (viewport->view_height >> 1) + viewport->view_y;

        viewport_adjust_for_map_height(&x, &y, &z);
    }
    else
    {
        z = tile_element_height({ x, y });
    }

    gCurrentRotation = (get_current_rotation() + direction) & 3;

    int32_t new_x, new_y;
    centre_2d_coordinates(x, y, z, &new_x, &new_y, viewport);

    w->saved_view_x = new_x;
    w->saved_view_y = new_y;
    viewport->view_x = new_x;
    viewport->view_y = new_y;

    w->Invalidate();

    call_event_viewport_rotate_on_all_windows();
    reset_all_sprite_quadrant_placements();
}

void window_viewport_get_map_coords_by_cursor(
    rct_window* w, int16_t* map_x, int16_t* map_y, int16_t* offset_x, int16_t* offset_y)
{
    // Get mouse position to offset against.
    int32_t mouse_x, mouse_y;
    context_get_cursor_position_scaled(&mouse_x, &mouse_y);

    // Compute map coordinate by mouse position.
    get_map_coordinates_from_pos(mouse_x, mouse_y, VIEWPORT_INTERACTION_MASK_NONE, map_x, map_y, nullptr, nullptr, nullptr);

    // Get viewport coordinates centring around the tile.
    int32_t base_height = tile_element_height({ *map_x, *map_y });
    int32_t dest_x, dest_y;
    centre_2d_coordinates(*map_x, *map_y, base_height, &dest_x, &dest_y, w->viewport);

    // Rebase mouse position onto centre of window, and compensate for zoom level.
    int32_t rebased_x = ((w->width >> 1) - mouse_x) * (1 << w->viewport->zoom),
            rebased_y = ((w->height >> 1) - mouse_y) * (1 << w->viewport->zoom);

    // Compute cursor offset relative to tile.
    *offset_x = (w->saved_view_x - (dest_x + rebased_x)) * (1 << w->viewport->zoom);
    *offset_y = (w->saved_view_y - (dest_y + rebased_y)) * (1 << w->viewport->zoom);
}

void window_viewport_centre_tile_around_cursor(rct_window* w, int16_t map_x, int16_t map_y, int16_t offset_x, int16_t offset_y)
{
    // Get viewport coordinates centring around the tile.
    int32_t dest_x, dest_y;
    int32_t base_height = tile_element_height({ map_x, map_y });
    centre_2d_coordinates(map_x, map_y, base_height, &dest_x, &dest_y, w->viewport);

    // Get mouse position to offset against.
    int32_t mouse_x, mouse_y;
    context_get_cursor_position_scaled(&mouse_x, &mouse_y);

    // Rebase mouse position onto centre of window, and compensate for zoom level.
    int32_t rebased_x = ((w->width >> 1) - mouse_x) * (1 << w->viewport->zoom),
            rebased_y = ((w->height >> 1) - mouse_y) * (1 << w->viewport->zoom);

    // Apply offset to the viewport.
    w->saved_view_x = dest_x + rebased_x + (offset_x / (1 << w->viewport->zoom));
    w->saved_view_y = dest_y + rebased_y + (offset_y / (1 << w->viewport->zoom));
}

void window_zoom_set(rct_window* w, int32_t zoomLevel, bool atCursor)
{
    rct_viewport* v = w->viewport;

    zoomLevel = std::clamp(zoomLevel, 0, MAX_ZOOM_LEVEL);
    if (v->zoom == zoomLevel)
        return;

    // Zooming to cursor? Remember where we're pointing at the moment.
    int16_t saved_map_x = 0;
    int16_t saved_map_y = 0;
    int16_t offset_x = 0;
    int16_t offset_y = 0;
    if (gConfigGeneral.zoom_to_cursor && atCursor)
    {
        window_viewport_get_map_coords_by_cursor(w, &saved_map_x, &saved_map_y, &offset_x, &offset_y);
    }

    // Zoom in
    while (v->zoom > zoomLevel)
    {
        v->zoom--;
        w->saved_view_x += v->view_width / 4;
        w->saved_view_y += v->view_height / 4;
        v->view_width /= 2;
        v->view_height /= 2;
    }

    // Zoom out
    while (v->zoom < zoomLevel)
    {
        v->zoom++;
        w->saved_view_x -= v->view_width / 2;
        w->saved_view_y -= v->view_height / 2;
        v->view_width *= 2;
        v->view_height *= 2;
    }

    // Zooming to cursor? Centre around the tile we were hovering over just now.
    if (gConfigGeneral.zoom_to_cursor && atCursor)
    {
        window_viewport_centre_tile_around_cursor(w, saved_map_x, saved_map_y, offset_x, offset_y);
    }

    // HACK: Prevents the redraw from failing when there is
    // a window on top of the viewport.
    window_bring_to_front(w);
    w->Invalidate();
}

/**
 *
 *  rct2: 0x006887A6
 */
void window_zoom_in(rct_window* w, bool atCursor)
{
    window_zoom_set(w, w->viewport->zoom - 1, atCursor);
}

/**
 *
 *  rct2: 0x006887E0
 */
void window_zoom_out(rct_window* w, bool atCursor)
{
    window_zoom_set(w, w->viewport->zoom + 1, atCursor);
}

void main_window_zoom(bool zoomIn, bool atCursor)
{
    if (gScreenFlags & SCREEN_FLAGS_TITLE_DEMO)
        return;
    if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gS6Info.editor_step == EDITOR_STEP_LANDSCAPE_EDITOR)
    {
        if (!(gScreenFlags & SCREEN_FLAGS_TRACK_MANAGER))
        {
            rct_window* mainWindow = window_get_main();
            if (mainWindow != nullptr)
                window_zoom_set(mainWindow, mainWindow->viewport->zoom + (zoomIn ? -1 : 1), atCursor);
        }
    }
}

/**
 * Draws a window that is in the specified region.
 *  rct2: 0x006E756C
 * left (ax)
 * top (bx)
 * right (dx)
 * bottom (bp)
 */
void window_draw(rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    if (!window_is_visible(w))
        return;

    // Split window into only the regions that require drawing
    if (window_draw_split(dpi, w, left, top, right, bottom))
        return;

    // Clamp region
    left = std::max<int32_t>(left, w->x);
    top = std::max<int32_t>(top, w->y);
    right = std::min<int32_t>(right, w->x + w->width);
    bottom = std::min<int32_t>(bottom, w->y + w->height);
    if (left >= right)
        return;
    if (top >= bottom)
        return;

    // Draw the window in this region
    for (auto it = window_get_iterator(w); it != g_window_list.end(); it++)
    {
        // Don't draw overlapping opaque windows, they won't have changed
        auto v = (*it).get();
        if ((w == v || (v->flags & WF_TRANSPARENT)) && window_is_visible(v))
        {
            window_draw_single(dpi, v, left, top, right, bottom);
        }
    }
}

/**
 * Splits a drawing of a window into regions that can be seen and are not hidden
 * by other opaque overlapping windows.
 */
static int32_t window_draw_split(
    rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    // Divide the draws up for only the visible regions of the window recursively
    auto itPos = window_get_iterator(w);
    for (auto it = std::next(itPos); it != g_window_list.end(); it++)
    {
        // Check if this window overlaps w
        auto topwindow = it->get();
        if (topwindow->x >= right || topwindow->y >= bottom)
            continue;
        if (topwindow->x + topwindow->width <= left || topwindow->y + topwindow->height <= top)
            continue;
        if (topwindow->flags & WF_TRANSPARENT)
            continue;

        // A window overlaps w, split up the draw into two regions where the window starts to overlap
        if (topwindow->x > left)
        {
            // Split draw at topwindow.left
            window_draw(dpi, w, left, top, topwindow->x, bottom);
            window_draw(dpi, w, topwindow->x, top, right, bottom);
        }
        else if (topwindow->x + topwindow->width < right)
        {
            // Split draw at topwindow.right
            window_draw(dpi, w, left, top, topwindow->x + topwindow->width, bottom);
            window_draw(dpi, w, topwindow->x + topwindow->width, top, right, bottom);
        }
        else if (topwindow->y > top)
        {
            // Split draw at topwindow.top
            window_draw(dpi, w, left, top, right, topwindow->y);
            window_draw(dpi, w, left, topwindow->y, right, bottom);
        }
        else if (topwindow->y + topwindow->height < bottom)
        {
            // Split draw at topwindow.bottom
            window_draw(dpi, w, left, top, right, topwindow->y + topwindow->height);
            window_draw(dpi, w, left, topwindow->y + topwindow->height, right, bottom);
        }

        // Drawing for this region should be done now, exit
        return 1;
    }

    // No windows overlap
    return 0;
}

static void window_draw_single(rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    // Copy dpi so we can crop it
    rct_drawpixelinfo copy = *dpi;
    dpi = &copy;

    // Clamp left to 0
    int32_t overflow = left - dpi->x;
    if (overflow > 0)
    {
        dpi->x += overflow;
        dpi->width -= overflow;
        if (dpi->width <= 0)
            return;
        dpi->pitch += overflow;
        dpi->bits += overflow;
    }

    // Clamp width to right
    overflow = dpi->x + dpi->width - right;
    if (overflow > 0)
    {
        dpi->width -= overflow;
        if (dpi->width <= 0)
            return;
        dpi->pitch += overflow;
    }

    // Clamp top to 0
    overflow = top - dpi->y;
    if (overflow > 0)
    {
        dpi->y += overflow;
        dpi->height -= overflow;
        if (dpi->height <= 0)
            return;
        dpi->bits += (dpi->width + dpi->pitch) * overflow;
    }

    // Clamp height to bottom
    overflow = dpi->y + dpi->height - bottom;
    if (overflow > 0)
    {
        dpi->height -= overflow;
        if (dpi->height <= 0)
            return;
    }

    // Invalidate modifies the window colours so first get the correct
    // colour before setting the global variables for the string painting
    window_event_invalidate_call(w);

    // Text colouring
    gCurrentWindowColours[0] = NOT_TRANSLUCENT(w->colours[0]);
    gCurrentWindowColours[1] = NOT_TRANSLUCENT(w->colours[1]);
    gCurrentWindowColours[2] = NOT_TRANSLUCENT(w->colours[2]);
    gCurrentWindowColours[3] = NOT_TRANSLUCENT(w->colours[3]);

    window_event_paint_call(w, dpi);
}

/**
 *
 *  rct2: 0x00685BE1
 *
 * @param dpi (edi)
 * @param w (esi)
 */
void window_draw_viewport(rct_drawpixelinfo* dpi, rct_window* w)
{
    viewport_render(dpi, w->viewport, dpi->x, dpi->y, dpi->x + dpi->width, dpi->y + dpi->height);
}

void window_set_position(rct_window* w, ScreenCoordsXY screenCoords)
{
    window_move_position(w, ScreenCoordsXY(screenCoords.x - w->x, screenCoords.y - w->y));
}

void window_move_position(rct_window* w, ScreenCoordsXY deltaCoords)
{
    if (deltaCoords.x == 0 && deltaCoords.y == 0)
        return;

    // Invalidate old region
    w->Invalidate();

    // Translate window and viewport
    w->x += deltaCoords.x;
    w->y += deltaCoords.y;
    if (w->viewport != nullptr)
    {
        w->viewport->x += deltaCoords.x;
        w->viewport->y += deltaCoords.y;
    }

    // Invalidate new region
    w->Invalidate();
}

void window_resize(rct_window* w, int32_t dw, int32_t dh)
{
    int32_t i;
    if (dw == 0 && dh == 0)
        return;

    // Invalidate old region
    w->Invalidate();

    // Clamp new size to minimum and maximum
    w->width = std::clamp<int16_t>(w->width + dw, w->min_width, w->max_width);
    w->height = std::clamp<int16_t>(w->height + dh, w->min_height, w->max_height);

    window_event_resize_call(w);
    window_event_invalidate_call(w);

    // Update scroll widgets
    for (i = 0; i < 3; i++)
    {
        w->scrolls[i].h_right = WINDOW_SCROLL_UNDEFINED;
        w->scrolls[i].v_bottom = WINDOW_SCROLL_UNDEFINED;
    }
    window_update_scroll_widgets(w);

    // Invalidate new region
    w->Invalidate();
}

void window_set_resize(rct_window* w, int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight)
{
    w->min_width = minWidth;
    w->min_height = minHeight;
    w->max_width = maxWidth;
    w->max_height = maxHeight;

    // Clamp width and height to minimum and maximum
    int32_t width = std::clamp<int32_t>(w->width, minWidth, maxWidth);
    int32_t height = std::clamp<int32_t>(w->height, minHeight, maxHeight);

    // Resize window if size has changed
    if (w->width != width || w->height != height)
    {
        w->Invalidate();
        w->width = width;
        w->height = height;
        w->Invalidate();
    }
}

/**
 *
 *  rct2: 0x006EE212
 *
 * @param tool (al)
 * @param widgetIndex (dx)
 * @param w (esi)
 */
bool tool_set(rct_window* w, rct_widgetindex widgetIndex, TOOL_IDX tool)
{
    if (input_test_flag(INPUT_FLAG_TOOL_ACTIVE))
    {
        if (w->classification == gCurrentToolWidget.window_classification && w->number == gCurrentToolWidget.window_number
            && widgetIndex == gCurrentToolWidget.widget_index)
        {
            tool_cancel();
            return true;
        }
        else
        {
            tool_cancel();
        }
    }

    input_set_flag(INPUT_FLAG_TOOL_ACTIVE, true);
    input_set_flag(INPUT_FLAG_6, false);
    gCurrentToolId = tool;
    gCurrentToolWidget.window_classification = w->classification;
    gCurrentToolWidget.window_number = w->number;
    gCurrentToolWidget.widget_index = widgetIndex;
    return false;
}

/**
 *
 *  rct2: 0x006EE281
 */
void tool_cancel()
{
    if (input_test_flag(INPUT_FLAG_TOOL_ACTIVE))
    {
        input_set_flag(INPUT_FLAG_TOOL_ACTIVE, false);

        map_invalidate_selection_rect();
        map_invalidate_map_selection_tiles();

        // Reset map selection
        gMapSelectFlags = 0;

        if (gCurrentToolWidget.widget_index != -1)
        {
            // Invalidate tool widget
            widget_invalidate_by_number(
                gCurrentToolWidget.window_classification, gCurrentToolWidget.window_number, gCurrentToolWidget.widget_index);

            // Abort tool event
            rct_window* w = window_find_by_number(gCurrentToolWidget.window_classification, gCurrentToolWidget.window_number);
            if (w != nullptr)
                window_event_tool_abort_call(w, gCurrentToolWidget.widget_index);
        }
    }
}

void window_event_close_call(rct_window* w)
{
    if (w->event_handlers->close != nullptr)
        w->event_handlers->close(w);
}

void window_event_mouse_up_call(rct_window* w, rct_widgetindex widgetIndex)
{
    if (w->event_handlers->mouse_up != nullptr)
        w->event_handlers->mouse_up(w, widgetIndex);
}

void window_event_resize_call(rct_window* w)
{
    if (w->event_handlers->resize != nullptr)
        w->event_handlers->resize(w);
}

void window_event_mouse_down_call(rct_window* w, rct_widgetindex widgetIndex)
{
    if (w->event_handlers->mouse_down != nullptr)
        w->event_handlers->mouse_down(w, widgetIndex, &w->widgets[widgetIndex]);
}

void window_event_dropdown_call(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex)
{
    if (w->event_handlers->dropdown != nullptr)
        w->event_handlers->dropdown(w, widgetIndex, dropdownIndex);
}

void window_event_unknown_05_call(rct_window* w)
{
    if (w->event_handlers->unknown_05 != nullptr)
        w->event_handlers->unknown_05(w);
}

void window_event_update_call(rct_window* w)
{
    if (w->event_handlers->update != nullptr)
        w->event_handlers->update(w);
}

void window_event_periodic_update_call(rct_window* w)
{
    if (w->event_handlers->periodic_update != nullptr)
        w->event_handlers->periodic_update(w);
}

void window_event_unknown_08_call(rct_window* w)
{
    if (w->event_handlers->unknown_08 != nullptr)
        w->event_handlers->unknown_08(w);
}

void window_event_tool_update_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->tool_update != nullptr)
        w->event_handlers->tool_update(w, widgetIndex, screenCoords);
}

void window_event_tool_down_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->tool_down != nullptr)
        w->event_handlers->tool_down(w, widgetIndex, screenCoords);
}

void window_event_tool_drag_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->tool_drag != nullptr)
        w->event_handlers->tool_drag(w, widgetIndex, screenCoords);
}

void window_event_tool_up_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->tool_up != nullptr)
        w->event_handlers->tool_up(w, widgetIndex, screenCoords);
}

void window_event_tool_abort_call(rct_window* w, rct_widgetindex widgetIndex)
{
    if (w->event_handlers->tool_abort != nullptr)
        w->event_handlers->tool_abort(w, widgetIndex);
}

void window_event_unknown_0E_call(rct_window* w)
{
    if (w->event_handlers->unknown_0E != nullptr)
        w->event_handlers->unknown_0E(w);
}

void window_get_scroll_size(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height)
{
    if (w->event_handlers->get_scroll_size != nullptr)
    {
        w->event_handlers->get_scroll_size(w, scrollIndex, width, height);
    }
}

void window_event_scroll_mousedown_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->scroll_mousedown != nullptr)
        w->event_handlers->scroll_mousedown(w, scrollIndex, screenCoords.x, screenCoords.y);
}

void window_event_scroll_mousedrag_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->scroll_mousedrag != nullptr)
        w->event_handlers->scroll_mousedrag(w, scrollIndex, screenCoords.x, screenCoords.y);
}

void window_event_scroll_mouseover_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->scroll_mouseover != nullptr)
        w->event_handlers->scroll_mouseover(w, scrollIndex, screenCoords.x, screenCoords.y);
}

void window_event_textinput_call(rct_window* w, rct_widgetindex widgetIndex, char* text)
{
    if (w->event_handlers->text_input != nullptr)
        w->event_handlers->text_input(w, widgetIndex, text);
}

void window_event_viewport_rotate_call(rct_window* w)
{
    if (w->event_handlers->viewport_rotate != nullptr)
        w->event_handlers->viewport_rotate(w);
}

void window_event_unknown_15_call(rct_window* w, int32_t scrollIndex, int32_t scrollAreaType)
{
    if (w->event_handlers->unknown_15 != nullptr)
        w->event_handlers->unknown_15(w, scrollIndex, scrollAreaType);
}

rct_string_id window_event_tooltip_call(rct_window* w, rct_widgetindex widgetIndex)
{
    rct_string_id result = 0;
    if (w->event_handlers->tooltip != nullptr)
        w->event_handlers->tooltip(w, widgetIndex, &result);
    return result;
}

int32_t window_event_cursor_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    int32_t cursorId = CURSOR_ARROW;
    if (w->event_handlers->cursor != nullptr)
        w->event_handlers->cursor(w, widgetIndex, screenCoords.x, screenCoords.y, &cursorId);
    return cursorId;
}

void window_event_moved_call(rct_window* w, ScreenCoordsXY screenCoords)
{
    if (w->event_handlers->moved != nullptr)
        w->event_handlers->moved(w, screenCoords.x, screenCoords.y);
}

void window_event_invalidate_call(rct_window* w)
{
    if (w->event_handlers->invalidate != nullptr)
        w->event_handlers->invalidate(w);
}

void window_event_paint_call(rct_window* w, rct_drawpixelinfo* dpi)
{
    if (w->event_handlers->paint != nullptr)
        w->event_handlers->paint(w, dpi);
}

void window_event_scroll_paint_call(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex)
{
    if (w->event_handlers->scroll_paint != nullptr)
        w->event_handlers->scroll_paint(w, dpi, scrollIndex);
}

/**
 * Bubbles an item one position up in the window list.  This is done by swapping
 * the two locations.
 *  rct2: New function not from rct2
 */
void window_bubble_list_item(rct_window* w, int32_t item_position)
{
    char swap = w->list_item_positions[item_position];
    w->list_item_positions[item_position] = w->list_item_positions[item_position + 1];
    w->list_item_positions[item_position + 1] = swap;
}

/**
 *
 *  rct2: 0x006ED710
 * Called after a window resize to move windows if they
 * are going to be out of sight.
 */
void window_relocate_windows(int32_t width, int32_t height)
{
    int32_t new_location = 8;
    window_visit_each([width, height, &new_location](rct_window* w) {
        // Work out if the window requires moving
        if (w->x + 10 < width)
        {
            if (w->flags & (WF_STICK_TO_BACK | WF_STICK_TO_FRONT))
            {
                if (w->y - 22 < height)
                {
                    return;
                }
            }
            if (w->y + 10 < height)
            {
                return;
            }
        }

        // Calculate the new locations
        int32_t x = w->x;
        int32_t y = w->y;
        w->x = new_location;
        w->y = new_location + TOP_TOOLBAR_HEIGHT + 1;

        // Move the next new location so windows are not directly on top
        new_location += 8;

        // Adjust the viewport if required.
        if (w->viewport != nullptr)
        {
            w->viewport->x -= x - w->x;
            w->viewport->y -= y - w->y;
        }
    });
}

/**
 * rct2: 0x0066B905
 */
void window_resize_gui(int32_t width, int32_t height)
{
    window_resize_gui_scenario_editor(width, height);
    if (gScreenFlags & SCREEN_FLAGS_EDITOR)
        return;

    rct_window* titleWind = window_find_by_class(WC_TITLE_MENU);
    if (titleWind != nullptr)
    {
        titleWind->x = (width - titleWind->width) / 2;
        titleWind->y = height - 154;
    }

    rct_window* exitWind = window_find_by_class(WC_TITLE_EXIT);
    if (exitWind != nullptr)
    {
        exitWind->x = width - 40;
        exitWind->y = height - 64;
    }

    rct_window* optionsWind = window_find_by_class(WC_TITLE_OPTIONS);
    if (optionsWind != nullptr)
    {
        optionsWind->x = width - 80;
    }

    gfx_invalidate_screen();
}

/**
 * rct2: 0x0066F0DD
 */
void window_resize_gui_scenario_editor(int32_t width, int32_t height)
{
    rct_window* mainWind = window_get_main();
    if (mainWind != nullptr)
    {
        rct_viewport* viewport = mainWind->viewport;
        mainWind->width = width;
        mainWind->height = height;
        viewport->width = width;
        viewport->height = height;
        viewport->view_width = width << viewport->zoom;
        viewport->view_height = height << viewport->zoom;
        if (mainWind->widgets != nullptr && mainWind->widgets[WC_MAIN_WINDOW__0].type == WWT_VIEWPORT)
        {
            mainWind->widgets[WC_MAIN_WINDOW__0].right = width;
            mainWind->widgets[WC_MAIN_WINDOW__0].bottom = height;
        }
    }

    rct_window* topWind = window_find_by_class(WC_TOP_TOOLBAR);
    if (topWind != nullptr)
    {
        topWind->width = std::max(640, width);
    }

    rct_window* bottomWind = window_find_by_class(WC_BOTTOM_TOOLBAR);
    if (bottomWind != nullptr)
    {
        bottomWind->y = height - 32;
        bottomWind->width = std::max(640, width);
    }
}

/* Based on rct2: 0x6987ED and another version from window_park */
void window_align_tabs(rct_window* w, rct_widgetindex start_tab_id, rct_widgetindex end_tab_id)
{
    int32_t i, x = w->widgets[start_tab_id].left;
    int32_t tab_width = w->widgets[start_tab_id].right - w->widgets[start_tab_id].left;

    for (i = start_tab_id; i <= end_tab_id; i++)
    {
        if (!(w->disabled_widgets & (1LL << i)))
        {
            w->widgets[i].left = x;
            w->widgets[i].right = x + tab_width;
            x += tab_width + 1;
        }
    }
}

/**
 *
 *  rct2: 0x006CBCC3
 */
void window_close_construction_windows()
{
    window_close_by_class(WC_RIDE_CONSTRUCTION);
    window_close_by_class(WC_FOOTPATH);
    window_close_by_class(WC_TRACK_DESIGN_LIST);
    window_close_by_class(WC_TRACK_DESIGN_PLACE);
}

/**
 * Update zoom based volume attenuation for ride music and clear music list.
 *  rct2: 0x006BC348
 */
void window_update_viewport_ride_music()
{
    gRideMusicParamsListEnd = &gRideMusicParamsList[0];
    g_music_tracking_viewport = nullptr;

    for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
    {
        auto w = it->get();
        auto viewport = w->viewport;
        if (viewport == nullptr || !(viewport->flags & VIEWPORT_FLAG_SOUND_ON))
            continue;

        g_music_tracking_viewport = viewport;
        gWindowAudioExclusive = w;

        switch (viewport->zoom)
        {
            case 0:
                gVolumeAdjustZoom = 0;
                break;
            case 1:
                gVolumeAdjustZoom = 30;
                break;
            default:
                gVolumeAdjustZoom = 60;
                break;
        }
        break;
    }
}

static void window_snap_left(rct_window* w, int32_t proximity)
{
    auto mainWindow = window_get_main();
    auto wBottom = w->y + w->height;
    auto wLeftProximity = w->x - (proximity * 2);
    auto wRightProximity = w->x + (proximity * 2);
    auto rightMost = INT32_MIN;

    window_visit_each([&](rct_window* w2) {
        if (w2 == w || w2 == mainWindow)
            return;

        auto right = w2->x + w2->width;

        if (wBottom < w2->y || w->y > w2->y + w2->height)
            return;

        if (right < wLeftProximity || right > wRightProximity)
            return;

        rightMost = std::max(rightMost, right);
    });

    if (0 >= wLeftProximity && 0 <= wRightProximity)
        rightMost = std::max(rightMost, 0);

    if (rightMost != INT32_MIN)
        w->x = rightMost;
}

static void window_snap_top(rct_window* w, int32_t proximity)
{
    auto mainWindow = window_get_main();
    auto wRight = w->x + w->width;
    auto wTopProximity = w->y - (proximity * 2);
    auto wBottomProximity = w->y + (proximity * 2);
    auto bottomMost = INT32_MIN;

    window_visit_each([&](rct_window* w2) {
        if (w2 == w || w2 == mainWindow)
            return;

        auto bottom = w2->y + w2->height;

        if (wRight < w2->x || w->x > w2->x + w2->width)
            return;

        if (bottom < wTopProximity || bottom > wBottomProximity)
            return;

        bottomMost = std::max(bottomMost, bottom);
    });

    if (0 >= wTopProximity && 0 <= wBottomProximity)
        bottomMost = std::max(bottomMost, 0);

    if (bottomMost != INT32_MIN)
        w->y = bottomMost;
}

static void window_snap_right(rct_window* w, int32_t proximity)
{
    auto mainWindow = window_get_main();
    auto wRight = w->x + w->width;
    auto wBottom = w->y + w->height;
    auto wLeftProximity = wRight - (proximity * 2);
    auto wRightProximity = wRight + (proximity * 2);
    auto leftMost = INT32_MAX;

    window_visit_each([&](rct_window* w2) {
        if (w2 == w || w2 == mainWindow)
            return;

        if (wBottom < w2->y || w->y > w2->y + w2->height)
            return;

        if (w2->x < wLeftProximity || w2->x > wRightProximity)
            return;

        leftMost = std::min<int32_t>(leftMost, w2->x);
    });

    auto screenWidth = context_get_width();
    if (screenWidth >= wLeftProximity && screenWidth <= wRightProximity)
        leftMost = std::min(leftMost, screenWidth);

    if (leftMost != INT32_MAX)
        w->x = leftMost - w->width;
}

static void window_snap_bottom(rct_window* w, int32_t proximity)
{
    auto mainWindow = window_get_main();
    auto wRight = w->x + w->width;
    auto wBottom = w->y + w->height;
    auto wTopProximity = wBottom - (proximity * 2);
    auto wBottomProximity = wBottom + (proximity * 2);
    auto topMost = INT32_MAX;

    window_visit_each([&](rct_window* w2) {
        if (w2 == w || w2 == mainWindow)
            return;

        if (wRight < w2->x || w->x > w2->x + w2->width)
            return;

        if (w2->y < wTopProximity || w2->y > wBottomProximity)
            return;

        topMost = std::min<int32_t>(topMost, w2->y);
    });

    auto screenHeight = context_get_height();
    if (screenHeight >= wTopProximity && screenHeight <= wBottomProximity)
        topMost = std::min(topMost, screenHeight);

    if (topMost != INT32_MAX)
        w->y = topMost - w->height;
}

void window_move_and_snap(rct_window* w, ScreenCoordsXY newWindowCoords, int32_t snapProximity)
{
    int32_t originalX = w->x;
    int32_t originalY = w->y;
    int32_t minY = (gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) ? 1 : TOP_TOOLBAR_HEIGHT + 2;

    newWindowCoords.y = std::clamp(newWindowCoords.y, minY, context_get_height() - 34);

    if (snapProximity > 0)
    {
        w->x = newWindowCoords.x;
        w->y = newWindowCoords.y;

        window_snap_right(w, snapProximity);
        window_snap_bottom(w, snapProximity);
        window_snap_left(w, snapProximity);
        window_snap_top(w, snapProximity);

        if (w->x == originalX && w->y == originalY)
            return;

        newWindowCoords.x = w->x;
        newWindowCoords.y = w->y;
        w->x = originalX;
        w->y = originalY;
    }

    window_set_position(w, newWindowCoords);
}

int32_t window_can_resize(rct_window* w)
{
    return (w->flags & WF_RESIZABLE) && (w->min_width != w->max_width || w->min_height != w->max_height);
}

/**
 *
 *  rct2: 0x006EE3C3
 */
void textinput_cancel()
{
    window_close_by_class(WC_TEXTINPUT);
}

void window_start_textbox(
    rct_window* call_w, rct_widgetindex call_widget, rct_string_id existing_text, char* existing_args, int32_t maxLength)
{
    if (gUsingWidgetTextBox)
        window_cancel_textbox();

    gUsingWidgetTextBox = true;
    gCurrentTextBox.window.classification = call_w->classification;
    gCurrentTextBox.window.number = call_w->number;
    gCurrentTextBox.widget_index = call_widget;
    gTextBoxFrameNo = 0;

    gMaxTextBoxInputLength = maxLength;

    window_close_by_class(WC_TEXTINPUT);

    // Clear the text input buffer
    std::fill_n(gTextBoxInput, maxLength, 0x00);

    // Enter in the text input buffer any existing
    // text.
    if (existing_text != STR_NONE)
        format_string(gTextBoxInput, TEXT_INPUT_SIZE, existing_text, &existing_args);

    // In order to prevent strings that exceed the maxLength
    // from crashing the game.
    gTextBoxInput[maxLength - 1] = '\0';

    gTextInput = context_start_text_input(gTextBoxInput, maxLength);
}

void window_cancel_textbox()
{
    if (gUsingWidgetTextBox)
    {
        rct_window* w = window_find_by_number(gCurrentTextBox.window.classification, gCurrentTextBox.window.number);
        window_event_textinput_call(w, gCurrentTextBox.widget_index, nullptr);
        gCurrentTextBox.window.classification = WC_NULL;
        gCurrentTextBox.window.number = 0;
        context_stop_text_input();
        gUsingWidgetTextBox = false;
        widget_invalidate(w, gCurrentTextBox.widget_index);
        gCurrentTextBox.widget_index = WWT_LAST;
    }
}

void window_update_textbox_caret()
{
    gTextBoxFrameNo++;
    if (gTextBoxFrameNo > 30)
        gTextBoxFrameNo = 0;
}

void window_update_textbox()
{
    if (gUsingWidgetTextBox)
    {
        gTextBoxFrameNo = 0;
        rct_window* w = window_find_by_number(gCurrentTextBox.window.classification, gCurrentTextBox.window.number);
        widget_invalidate(w, gCurrentTextBox.widget_index);
        window_event_textinput_call(w, gCurrentTextBox.widget_index, gTextBoxInput);
    }
}

bool window_is_visible(rct_window* w)
{
    // w->visibility is used to prevent repeat calculations within an iteration by caching the result
    if (w == nullptr)
        return false;

    if (w->visibility == VC_VISIBLE)
        return true;
    if (w->visibility == VC_COVERED)
        return false;

    // only consider viewports, consider the main window always visible
    if (w->viewport == nullptr || w->classification == WC_MAIN_WINDOW)
    {
        // default to previous behaviour
        w->visibility = VC_VISIBLE;
        return true;
    }

    // start from the window above the current
    auto itPos = window_get_iterator(w);
    for (auto it = std::next(itPos); it != g_window_list.end(); it++)
    {
        auto& w_other = *(*it);

        // if covered by a higher window, no rendering needed
        if (w_other.x <= w->x && w_other.y <= w->y && w_other.x + w_other.width >= w->x + w->width
            && w_other.y + w_other.height >= w->y + w->height)
        {
            w->visibility = VC_COVERED;
            w->viewport->visibility = VC_COVERED;
            return false;
        }
    }

    // default to previous behaviour
    w->visibility = VC_VISIBLE;
    w->viewport->visibility = VC_VISIBLE;
    return true;
}

/**
 *
 *  rct2: 0x006E7499
 * left (ax)
 * top (bx)
 * right (dx)
 * bottom (bp)
 */
void window_draw_all(rct_drawpixelinfo* dpi, int16_t left, int16_t top, int16_t right, int16_t bottom)
{
    rct_drawpixelinfo windowDPI = *dpi;
    windowDPI.bits = dpi->bits + left + ((dpi->width + dpi->pitch) * top);
    windowDPI.x = left;
    windowDPI.y = top;
    windowDPI.width = right - left;
    windowDPI.height = bottom - top;
    windowDPI.pitch = dpi->width + dpi->pitch + left - right;
    windowDPI.zoom_level = 0;

    window_visit_each([&windowDPI, left, top, right, bottom](rct_window* w) {
        if (w->flags & WF_TRANSPARENT)
            return;
        if (right <= w->x || bottom <= w->y)
            return;
        if (left >= w->x + w->width || top >= w->y + w->height)
            return;
        window_draw(&windowDPI, w, left, top, right, bottom);
    });
}

rct_viewport* window_get_previous_viewport(rct_viewport* current)
{
    bool foundPrevious = (current == nullptr);
    for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
    {
        auto& w = **it;
        if (w.viewport != nullptr)
        {
            if (foundPrevious)
            {
                return w.viewport;
            }
            if (w.viewport == current)
            {
                foundPrevious = true;
            }
        }
    }
    return nullptr;
}

void window_reset_visibilities()
{
    // reset window visibility status to unknown
    window_visit_each([](rct_window* w) {
        w->visibility = VC_UNKNOWN;
        if (w->viewport != nullptr)
        {
            w->viewport->visibility = VC_UNKNOWN;
        }
    });
}

void window_init_all()
{
    window_close_all_except_flags(0);
}

void window_follow_sprite(rct_window* w, size_t spriteIndex)
{
    if (spriteIndex < MAX_SPRITES || spriteIndex == SPRITE_INDEX_NULL)
    {
        w->viewport_smart_follow_sprite = (uint16_t)spriteIndex;
    }
}

void window_unfollow_sprite(rct_window* w)
{
    w->viewport_smart_follow_sprite = SPRITE_INDEX_NULL;
    w->viewport_target_sprite = SPRITE_INDEX_NULL;
}

rct_viewport* window_get_viewport(rct_window* w)
{
    if (w == nullptr)
    {
        return nullptr;
    }

    return w->viewport;
}

rct_window* window_get_listening()
{
    for (auto it = g_window_list.rbegin(); it != g_window_list.rend(); it++)
    {
        auto& w = **it;
        auto viewport = w.viewport;
        if (viewport != nullptr)
        {
            if (viewport->flags & VIEWPORT_FLAG_SOUND_ON)
            {
                return &w;
            }
        }
    }
    return nullptr;
}

rct_windowclass window_get_classification(rct_window* window)
{
    return window->classification;
}

/**
 *
 *  rct2: 0x006EAF26
 */
void widget_scroll_update_thumbs(rct_window* w, rct_widgetindex widget_index)
{
    rct_widget* widget = &w->widgets[widget_index];
    rct_scroll* scroll = &w->scrolls[window_get_scroll_data_index(w, widget_index)];

    if (scroll->flags & HSCROLLBAR_VISIBLE)
    {
        int32_t view_size = widget->right - widget->left - 21;
        if (scroll->flags & VSCROLLBAR_VISIBLE)
            view_size -= 11;
        int32_t x = scroll->h_left * view_size;
        if (scroll->h_right != 0)
            x /= scroll->h_right;
        scroll->h_thumb_left = x + 11;

        x = widget->right - widget->left - 2;
        if (scroll->flags & VSCROLLBAR_VISIBLE)
            x -= 11;
        x += scroll->h_left;
        if (scroll->h_right != 0)
            x = (x * view_size) / scroll->h_right;
        x += 11;
        view_size += 10;
        scroll->h_thumb_right = std::min(x, view_size);

        if (scroll->h_thumb_right - scroll->h_thumb_left < 20)
        {
            double barPosition = (scroll->h_thumb_right * 1.0) / view_size;

            scroll->h_thumb_left = (uint16_t)std::lround(scroll->h_thumb_left - (20 * barPosition));
            scroll->h_thumb_right = (uint16_t)std::lround(scroll->h_thumb_right + (20 * (1 - barPosition)));
        }
    }

    if (scroll->flags & VSCROLLBAR_VISIBLE)
    {
        int32_t view_size = widget->bottom - widget->top - 21;
        if (scroll->flags & HSCROLLBAR_VISIBLE)
            view_size -= 11;
        int32_t y = scroll->v_top * view_size;
        if (scroll->v_bottom != 0)
            y /= scroll->v_bottom;
        scroll->v_thumb_top = y + 11;

        y = widget->bottom - widget->top - 2;
        if (scroll->flags & HSCROLLBAR_VISIBLE)
            y -= 11;
        y += scroll->v_top;
        if (scroll->v_bottom != 0)
            y = (y * view_size) / scroll->v_bottom;
        y += 11;
        view_size += 10;
        scroll->v_thumb_bottom = std::min(y, view_size);

        if (scroll->v_thumb_bottom - scroll->v_thumb_top < 20)
        {
            double barPosition = (scroll->v_thumb_bottom * 1.0) / view_size;

            scroll->v_thumb_top = (uint16_t)std::lround(scroll->v_thumb_top - (20 * barPosition));
            scroll->v_thumb_bottom = (uint16_t)std::lround(scroll->v_thumb_bottom + (20 * (1 - barPosition)));
        }
    }
}
