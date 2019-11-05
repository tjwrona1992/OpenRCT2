/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <algorithm>
#include <openrct2-ui/interface/LandTool.h>
#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/actions/LandBuyRightsAction.hpp>
#include <openrct2/drawing/Drawing.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/world/Park.h>

// clang-format off
enum WINDOW_WATER_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_PREVIEW,
    WIDX_DECREMENT,
    WIDX_INCREMENT,
    WIDX_BUY_LAND_RIGHTS,
    WIDX_BUY_CONSTRUCTION_RIGHTS
};

static rct_widget window_land_rights_widgets[] = {
    { WWT_FRAME,    0,  0,  97, 0,  93, 0xFFFFFFFF,                                 STR_NONE },                             // panel / background
    { WWT_CAPTION,  0,  1,  96, 1,  14, STR_LAND_RIGHTS,                            STR_WINDOW_TITLE_TIP },                 // title bar
    { WWT_CLOSEBOX, 0,  85, 95, 2,  13, STR_CLOSE_X,                                STR_CLOSE_WINDOW_TIP },                 // close x button
    { WWT_IMGBTN,   0,  27, 70, 17, 48, SPR_LAND_TOOL_SIZE_0,                       STR_NONE },                             // preview box
    { WWT_TRNBTN,   2,  28, 43, 18, 33, IMAGE_TYPE_REMAP | SPR_LAND_TOOL_DECREASE,        STR_ADJUST_SMALLER_LAND_RIGHTS_TIP },   // decrement size
    { WWT_TRNBTN,   2,  54, 69, 32, 47, IMAGE_TYPE_REMAP | SPR_LAND_TOOL_INCREASE,        STR_ADJUST_LARGER_LAND_RIGHTS_TIP },    // increment size
    { WWT_FLATBTN,  2,  22, 45, 53, 76, IMAGE_TYPE_REMAP | SPR_BUY_LAND_RIGHTS,           STR_BUY_LAND_RIGHTS_TIP },              // land rights
    { WWT_FLATBTN,  2,  52, 75, 53, 76, IMAGE_TYPE_REMAP | SPR_BUY_CONSTRUCTION_RIGHTS,   STR_BUY_CONSTRUCTION_RIGHTS_TIP },      // construction rights
    { WIDGETS_END },
};

static void window_land_rights_close(rct_window *w);
static void window_land_rights_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_land_rights_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget *widget);
static void window_land_rights_update(rct_window *w);
static void window_land_rights_invalidate(rct_window *w);
static void window_land_rights_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_land_rights_textinput(rct_window *w, rct_widgetindex widgetIndex, char *text);
static void window_land_rights_inputsize(rct_window *w);
static void window_land_rights_toolupdate(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_land_rights_tooldown(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_land_rights_tooldrag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_land_rights_toolabort(rct_window *w, rct_widgetindex widgetIndex);
static bool land_rights_tool_is_active();


static rct_window_event_list window_land_rights_events = {
    window_land_rights_close,
    window_land_rights_mouseup,
    nullptr,
    window_land_rights_mousedown,
    nullptr,
    nullptr,
    window_land_rights_update,
    nullptr,
    nullptr,
    window_land_rights_toolupdate,
    window_land_rights_tooldown,
    window_land_rights_tooldrag,
    nullptr,
    window_land_rights_toolabort,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_land_rights_textinput,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_land_rights_invalidate,
    window_land_rights_paint,
    nullptr
};
// clang-format on

constexpr uint8_t LAND_RIGHTS_MODE_BUY_CONSTRUCTION_RIGHTS = 0;
constexpr uint8_t LAND_RIGHTS_MODE_BUY_LAND = 1;

static uint8_t _landRightsMode;
static money32 _landRightsCost;

rct_window* window_land_rights_open()
{
    rct_window* window;

    // Check if window is already open
    window = window_find_by_class(WC_LAND_RIGHTS);
    if (window != nullptr)
        return window;

    window = window_create(ScreenCoordsXY(context_get_width() - 98, 29), 98, 94, &window_land_rights_events, WC_LAND_RIGHTS, 0);
    window->widgets = window_land_rights_widgets;
    window->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_DECREMENT) | (1 << WIDX_INCREMENT) | (1 << WIDX_PREVIEW)
        | (1 << WIDX_BUY_LAND_RIGHTS) | (1 << WIDX_BUY_CONSTRUCTION_RIGHTS);
    window->hold_down_widgets = (1 << WIDX_INCREMENT) | (1 << WIDX_DECREMENT);
    window_init_scroll_widgets(window);
    window_push_others_below(window);

    _landRightsMode = LAND_RIGHTS_MODE_BUY_LAND;
    window->pressed_widgets = (1 << WIDX_BUY_LAND_RIGHTS);

    gLandToolSize = 1;

    show_gridlines();
    tool_set(window, WIDX_BUY_LAND_RIGHTS, TOOL_UP_ARROW);
    input_set_flag(INPUT_FLAG_6, true);

    show_land_rights();

    if (gLandRemainingConstructionSales == 0)
    {
        show_construction_rights();
    }

    return window;
}

static void window_land_rights_close(rct_window* w)
{
    if (gLandRemainingConstructionSales == 0)
    {
        hide_construction_rights();
    }

    // If the tool wasn't changed, turn tool off
    if (land_rights_tool_is_active())
        tool_cancel();
}

static void window_land_rights_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_CLOSE:
            window_close(w);
            break;
        case WIDX_PREVIEW:
            window_land_rights_inputsize(w);
            break;
        case WIDX_BUY_LAND_RIGHTS:
            if (_landRightsMode != LAND_RIGHTS_MODE_BUY_LAND)
            {
                tool_set(w, WIDX_BUY_LAND_RIGHTS, TOOL_UP_ARROW);
                _landRightsMode = LAND_RIGHTS_MODE_BUY_LAND;
                show_land_rights();
                w->Invalidate();
            }
            break;
        case WIDX_BUY_CONSTRUCTION_RIGHTS:
            if (_landRightsMode != LAND_RIGHTS_MODE_BUY_CONSTRUCTION_RIGHTS)
            {
                tool_set(w, WIDX_BUY_CONSTRUCTION_RIGHTS, TOOL_UP_ARROW);
                _landRightsMode = LAND_RIGHTS_MODE_BUY_CONSTRUCTION_RIGHTS;
                show_construction_rights();
                w->Invalidate();
            }
            break;
    }
}

static void window_land_rights_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex)
    {
        case WIDX_DECREMENT:
            // Decrement land rights tool size
            gLandToolSize = std::max(MINIMUM_TOOL_SIZE, gLandToolSize - 1);

            // Invalidate the window
            w->Invalidate();
            break;
        case WIDX_INCREMENT:
            // Decrement land rights tool size
            gLandToolSize = std::min(MAXIMUM_TOOL_SIZE, gLandToolSize + 1);

            // Invalidate the window
            w->Invalidate();
            break;
    }
}

static void window_land_rights_textinput(rct_window* w, rct_widgetindex widgetIndex, char* text)
{
    int32_t size;
    char* end;

    if (widgetIndex != WIDX_PREVIEW || text == nullptr)
        return;

    size = strtol(text, &end, 10);
    if (*end == '\0')
    {
        size = std::max(MINIMUM_TOOL_SIZE, size);
        size = std::min(MAXIMUM_TOOL_SIZE, size);
        gLandToolSize = size;
        w->Invalidate();
    }
}

static void window_land_rights_inputsize(rct_window* w)
{
    TextInputDescriptionArgs[0] = MINIMUM_TOOL_SIZE;
    TextInputDescriptionArgs[1] = MAXIMUM_TOOL_SIZE;
    window_text_input_open(w, WIDX_PREVIEW, STR_SELECTION_SIZE, STR_ENTER_SELECTION_SIZE, STR_NONE, STR_NONE, 3);
}

static void window_land_rights_update(rct_window* w)
{
    w->frame_no++;
    // Close window if another tool is open
    if (!land_rights_tool_is_active())
        window_close(w);
}

static void window_land_rights_invalidate(rct_window* w)
{
    // Set the preview image button to be pressed down
    w->pressed_widgets |= (1 << WIDX_PREVIEW)
        | (1 << ((_landRightsMode == LAND_RIGHTS_MODE_BUY_LAND) ? WIDX_BUY_LAND_RIGHTS : WIDX_BUY_CONSTRUCTION_RIGHTS));
    w->pressed_widgets &= ~(
        1
        << ((_landRightsMode == LAND_RIGHTS_MODE_BUY_CONSTRUCTION_RIGHTS) ? WIDX_BUY_LAND_RIGHTS
                                                                          : WIDX_BUY_CONSTRUCTION_RIGHTS));

    // Update the preview image
    window_land_rights_widgets[WIDX_PREVIEW].image = land_tool_size_to_sprite_index(gLandToolSize);

    // Disable ownership and/or construction buying functions if there are no tiles left for sale
    if (gLandRemainingOwnershipSales == 0)
    {
        w->disabled_widgets |= (1 << WIDX_BUY_LAND_RIGHTS);
        window_land_rights_widgets[WIDX_BUY_LAND_RIGHTS].tooltip = STR_NO_LAND_RIGHTS_FOR_SALE_TIP;
    }
    else
    {
        w->disabled_widgets &= ~(1 << WIDX_BUY_LAND_RIGHTS);
        window_land_rights_widgets[WIDX_BUY_LAND_RIGHTS].tooltip = STR_BUY_LAND_RIGHTS_TIP;
    }

    if (gLandRemainingConstructionSales == 0)
    {
        w->disabled_widgets |= (1 << WIDX_BUY_CONSTRUCTION_RIGHTS);
        window_land_rights_widgets[WIDX_BUY_CONSTRUCTION_RIGHTS].tooltip = STR_NO_CONSTRUCTION_RIGHTS_FOR_SALE_TIP;
    }
    else
    {
        w->disabled_widgets &= ~(1 << WIDX_BUY_CONSTRUCTION_RIGHTS);
        window_land_rights_widgets[WIDX_BUY_CONSTRUCTION_RIGHTS].tooltip = STR_BUY_CONSTRUCTION_RIGHTS_TIP;
    }
}

static void window_land_rights_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    int32_t x, y;

    x = w->x + (window_land_rights_widgets[WIDX_PREVIEW].left + window_land_rights_widgets[WIDX_PREVIEW].right) / 2;
    y = w->y + (window_land_rights_widgets[WIDX_PREVIEW].top + window_land_rights_widgets[WIDX_PREVIEW].bottom) / 2;

    window_draw_widgets(w, dpi);
    // Draw number for tool sizes bigger than 7
    if (gLandToolSize > MAX_TOOL_SIZE_WITH_SPRITE)
    {
        gfx_draw_string_centred(dpi, STR_LAND_TOOL_SIZE_VALUE, x, y - 2, COLOUR_BLACK, &gLandToolSize);
    }

    // Draw cost amount
    if (_landRightsCost != MONEY32_UNDEFINED && _landRightsCost != 0 && !(gParkFlags & PARK_FLAGS_NO_MONEY))
    {
        x = (window_land_rights_widgets[WIDX_PREVIEW].left + window_land_rights_widgets[WIDX_PREVIEW].right) / 2 + w->x;
        y = window_land_rights_widgets[WIDX_PREVIEW].bottom + w->y + 32;
        gfx_draw_string_centred(dpi, STR_COST_AMOUNT, x, y, COLOUR_BLACK, &_landRightsCost);
    }
}

static void window_land_rights_tool_update_land_rights(int16_t x, int16_t y)
{
    map_invalidate_selection_rect();
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;

    LocationXY16 mapTile = {};
    screen_get_map_xy(x, y, &mapTile.x, &mapTile.y, nullptr);

    if (mapTile.x == LOCATION_NULL)
    {
        if (_landRightsCost != MONEY32_UNDEFINED)
        {
            _landRightsCost = MONEY32_UNDEFINED;
            window_invalidate_by_class(WC_CLEAR_SCENERY);
        }
        return;
    }

    uint8_t state_changed = 0;

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
    {
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        state_changed++;
    }

    if (gMapSelectType != MAP_SELECT_TYPE_FULL)
    {
        gMapSelectType = MAP_SELECT_TYPE_FULL;
        state_changed++;
    }

    int16_t tool_size = gLandToolSize;
    if (tool_size == 0)
        tool_size = 1;

    int16_t tool_length = (tool_size - 1) * 32;

    // Move to tool bottom left
    mapTile.x -= (tool_size - 1) * 16;
    mapTile.y -= (tool_size - 1) * 16;
    mapTile.x &= 0xFFE0;
    mapTile.y &= 0xFFE0;

    if (gMapSelectPositionA.x != mapTile.x)
    {
        gMapSelectPositionA.x = mapTile.x;
        state_changed++;
    }

    if (gMapSelectPositionA.y != mapTile.y)
    {
        gMapSelectPositionA.y = mapTile.y;
        state_changed++;
    }

    mapTile.x += tool_length;
    mapTile.y += tool_length;

    if (gMapSelectPositionB.x != mapTile.x)
    {
        gMapSelectPositionB.x = mapTile.x;
        state_changed++;
    }

    if (gMapSelectPositionB.y != mapTile.y)
    {
        gMapSelectPositionB.y = mapTile.y;
        state_changed++;
    }

    map_invalidate_selection_rect();
    if (!state_changed)
        return;

    auto landBuyRightsAction = LandBuyRightsAction(
        { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
        (_landRightsMode == LAND_RIGHTS_MODE_BUY_LAND) ? LandBuyRightSetting::BuyLand
                                                       : LandBuyRightSetting::BuyConstructionRights);
    auto res = GameActions::Query(&landBuyRightsAction);

    _landRightsCost = res->Error == GA_ERROR::OK ? res->Cost : MONEY32_UNDEFINED;
}

/**
 *
 *  rct2: 0x0066822A
 */
static void window_land_rights_toolabort(rct_window* w, rct_widgetindex widgetIndex)
{
    hide_gridlines();
    if (_landRightsMode == LAND_RIGHTS_MODE_BUY_LAND)
    {
        hide_land_rights();
    }
    else
    {
        hide_construction_rights();
    }
}

/**
 *
 *  rct2: 0x006681D1
 */
static void window_land_rights_toolupdate(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    window_land_rights_tool_update_land_rights(screenCoords.x, screenCoords.y);
}

/**
 *
 *  rct2: 0x006681E6
 */
static void window_land_rights_tooldown(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (_landRightsMode == LAND_RIGHTS_MODE_BUY_LAND)
    {
        if (screenCoords.x != LOCATION_NULL)
        {
            auto landBuyRightsAction = LandBuyRightsAction(
                { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                LandBuyRightSetting::BuyLand);
            GameActions::Execute(&landBuyRightsAction);
        }
    }
    else
    {
        if (screenCoords.x != LOCATION_NULL)
        {
            auto landBuyRightsAction = LandBuyRightsAction(
                { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                LandBuyRightSetting::BuyConstructionRights);
            GameActions::Execute(&landBuyRightsAction);
        }
    }
}

/**
 *
 *  rct2: 0x006681FB
 */
static void window_land_rights_tooldrag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    if (_landRightsMode == LAND_RIGHTS_MODE_BUY_LAND)
    {
        if (screenCoords.x != LOCATION_NULL)
        {
            auto landBuyRightsAction = LandBuyRightsAction(
                { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                LandBuyRightSetting::BuyLand);
            GameActions::Execute(&landBuyRightsAction);
        }
    }
    else
    {
        if (screenCoords.x != LOCATION_NULL)
        {
            auto landBuyRightsAction = LandBuyRightsAction(
                { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                LandBuyRightSetting::BuyConstructionRights);
            GameActions::Execute(&landBuyRightsAction);
        }
    }
}

static bool land_rights_tool_is_active()
{
    if (!(input_test_flag(INPUT_FLAG_TOOL_ACTIVE)))
        return false;
    if (gCurrentToolWidget.window_classification != WC_LAND_RIGHTS)
        return false;
    return true;
}
