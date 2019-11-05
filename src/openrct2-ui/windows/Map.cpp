/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <algorithm>
#include <iterator>
#include <openrct2-ui/interface/LandTool.h>
#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Cheats.h>
#include <openrct2/Context.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/actions/LandSetRightsAction.hpp>
#include <openrct2/actions/SurfaceSetStyleAction.hpp>
#include <openrct2/audio/audio.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/ride/Track.h>
#include <openrct2/world/Entrance.h>
#include <openrct2/world/Footpath.h>
#include <openrct2/world/Scenery.h>
#include <openrct2/world/Sprite.h>
#include <openrct2/world/Surface.h>
#include <vector>

#define MAP_COLOUR_2(colourA, colourB) (((colourA) << 8) | (colourB))
#define MAP_COLOUR(colour) MAP_COLOUR_2(colour, colour)
#define MAP_COLOUR_UNOWNED(colour) (PALETTE_INDEX_10 | ((colour)&0xFF00))

constexpr int32_t MAP_WINDOW_MAP_SIZE = MAXIMUM_MAP_SIZE_TECHNICAL * 2;

// Some functions manipulate coordinates on the map. These are the coordinates of the pixels in the
// minimap. In order to distinguish those from actual coordinates, we use a separate name.
using MapCoordsXY = TileCoordsXY;

// clang-format off
enum {
    PAGE_PEEPS,
    PAGE_RIDES
};

enum WINDOW_MAP_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_RESIZE = 3,
    WIDX_PEOPLE_TAB = 4,
    WIDX_RIDES_TAB = 5,
    WIDX_MAP = 6,
    WIDX_MAP_SIZE_SPINNER = 7,
    WIDX_MAP_SIZE_SPINNER_UP = 8,
    WIDX_MAP_SIZE_SPINNER_DOWN = 9,
    WIDX_SET_LAND_RIGHTS = 10,
    WIDX_BUILD_PARK_ENTRANCE = 11,
    WIDX_PEOPLE_STARTING_POSITION = 12,
    WIDX_LAND_TOOL = 13,
    WIDX_LAND_TOOL_SMALLER = 14,
    WIDX_LAND_TOOL_LARGER = 15,
    WIDX_LAND_OWNED_CHECKBOX = 16,
    WIDX_CONSTRUCTION_RIGHTS_OWNED_CHECKBOX = 17,
    WIDX_LAND_SALE_CHECKBOX = 18,
    WIDX_CONSTRUCTION_RIGHTS_SALE_CHECKBOX = 19,
    WIDX_ROTATE_90 = 20,
    WIDX_MAP_GENERATOR = 21
};

validate_global_widx(WC_MAP, WIDX_ROTATE_90);

static rct_widget window_map_widgets[] = {
    { WWT_FRAME,            0,  0,      244,    0,      258,    STR_NONE,                               STR_NONE },
    { WWT_CAPTION,          0,  1,      243,    1,      14,     STR_MAP_LABEL,                          STR_WINDOW_TITLE_TIP },
    { WWT_CLOSEBOX,         0,  232,    242,    2,      13,     STR_CLOSE_X,                            STR_CLOSE_WINDOW_TIP },
    { WWT_RESIZE,           1,  0,      244,    43,     257,    STR_NONE,                               STR_NONE },
    { WWT_COLOURBTN,        1,  3,      33,     17,     43,     IMAGE_TYPE_REMAP | SPR_TAB,                   STR_SHOW_PEOPLE_ON_MAP_TIP },
    { WWT_COLOURBTN,        1,  34,     64,     17,     43,     IMAGE_TYPE_REMAP | SPR_TAB,                   STR_SHOW_RIDES_STALLS_ON_MAP_TIP },
    { WWT_SCROLL,           1,  3,      241,    46,     225,    SCROLL_BOTH,                            STR_NONE },
      SPINNER_WIDGETS      (1,  104,    198,    229,    240,    STR_MAP_SIZE_VALUE,                     STR_NONE), // NB: 3 widgets
    { WWT_FLATBTN,          1,  4,      27,     1,      24,     SPR_BUY_LAND_RIGHTS,                    STR_SELECT_PARK_OWNED_LAND_TIP },
    { WWT_FLATBTN,          1,  4,      27,     1,      24,     SPR_PARK_ENTRANCE,                      STR_BUILD_PARK_ENTRANCE_TIP },
    { WWT_FLATBTN,          1,  28,     51,     1,      24,     (uint32_t) SPR_NONE,                      STR_SET_STARTING_POSITIONS_TIP },
    { WWT_IMGBTN,           1,  4,      47,     17,     48,     SPR_LAND_TOOL_SIZE_0,                   STR_NONE },
    { WWT_TRNBTN,           1,  5,      20,     18,     33,     IMAGE_TYPE_REMAP | SPR_LAND_TOOL_DECREASE,    STR_ADJUST_SMALLER_LAND_TIP },
    { WWT_TRNBTN,           1,  31,     46,     32,     47,     IMAGE_TYPE_REMAP | SPR_LAND_TOOL_INCREASE,    STR_ADJUST_LARGER_LAND_TIP },
    { WWT_CHECKBOX,         1,  58,     241,    197,    208,    STR_LAND_OWNED,                         STR_SET_LAND_TO_BE_OWNED_TIP },
    { WWT_CHECKBOX,         1,  58,     241,    197,    208,    STR_CONSTRUCTION_RIGHTS_OWNED,          STR_SET_CONSTRUCTION_RIGHTS_TO_BE_OWNED_TIP },
    { WWT_CHECKBOX,         1,  58,     241,    197,    208,    STR_LAND_SALE,                          STR_SET_LAND_TO_BE_AVAILABLE_TIP },
    { WWT_CHECKBOX,         1,  58,     231,    197,    208,    STR_CONSTRUCTION_RIGHTS_SALE,           STR_SET_CONSTRUCTION_RIGHTS_TO_BE_AVAILABLE_TIP },
    { WWT_FLATBTN,          1,  218,    241,    45,     68,     SPR_ROTATE_ARROW,                       STR_ROTATE_OBJECTS_90 },
    { WWT_BUTTON,           1,  110,    240,    190,    201,    STR_MAPGEN_WINDOW_TITLE,                STR_MAP_GENERATOR_TIP},
    { WIDGETS_END },
};

// used in transforming viewport view coordinates to minimap coordinates
// rct2: 0x00981BBC
static constexpr const LocationXY16 MiniMapOffsets[] = {
    {     MAXIMUM_MAP_SIZE_TECHNICAL - 8,                              0 },
    { 2 * MAXIMUM_MAP_SIZE_TECHNICAL - 8,     MAXIMUM_MAP_SIZE_TECHNICAL },
    {     MAXIMUM_MAP_SIZE_TECHNICAL - 8, 2 * MAXIMUM_MAP_SIZE_TECHNICAL },
    {                              0 - 8,     MAXIMUM_MAP_SIZE_TECHNICAL }
};

/** rct2: 0x00981BCC */
static constexpr const uint16_t RideKeyColours[] = {
    MAP_COLOUR(PALETTE_INDEX_61),   // COLOUR_KEY_RIDE
    MAP_COLOUR(PALETTE_INDEX_42),   // COLOUR_KEY_FOOD
    MAP_COLOUR(PALETTE_INDEX_20),   // COLOUR_KEY_DRINK
    MAP_COLOUR(PALETTE_INDEX_209),  // COLOUR_KEY_SOUVENIR
    MAP_COLOUR(PALETTE_INDEX_136),  // COLOUR_KEY_KIOSK
    MAP_COLOUR(PALETTE_INDEX_102),  // COLOUR_KEY_FIRST_AID
    MAP_COLOUR(PALETTE_INDEX_55),   // COLOUR_KEY_CASH_MACHINE
    MAP_COLOUR(PALETTE_INDEX_161),  // COLOUR_KEY_TOILETS
};

static void window_map_close(rct_window *w);
static void window_map_resize(rct_window *w);
static void window_map_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_map_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_map_update(rct_window *w);
static void window_map_toolupdate(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_map_tooldown(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_map_tooldrag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_map_toolabort(rct_window *w, rct_widgetindex widgetIndex);
static void window_map_scrollgetsize(rct_window *w, int32_t scrollIndex, int32_t *width, int32_t *height);
static void window_map_scrollmousedown(rct_window *w, int32_t scrollIndex, int32_t x, int32_t y);
static void window_map_textinput(rct_window *w, rct_widgetindex widgetIndex, char *text);
static void window_map_invalidate(rct_window *w);
static void window_map_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_map_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int32_t scrollIndex);

static rct_window_event_list window_map_events = {
    window_map_close,
    window_map_mouseup,
    window_map_resize,
    window_map_mousedown,
    nullptr,
    nullptr,
    window_map_update,
    nullptr,
    nullptr,
    window_map_toolupdate,
    window_map_tooldown,
    window_map_tooldrag,
    nullptr,
    window_map_toolabort,
    nullptr,
    window_map_scrollgetsize,
    window_map_scrollmousedown,
    window_map_scrollmousedown,
    nullptr,
    window_map_textinput,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_map_invalidate,
    window_map_paint,
    window_map_scrollpaint
};
// clang-format on

/** rct2: 0x00F1AD61 */
static uint8_t _activeTool;

/** rct2: 0x00F1AD6C */
static uint32_t _currentLine;

/** rct2: 0x00F1AD68 */
static std::vector<uint8_t> _mapImageData;

static uint16_t _landRightsToolSize;

static void window_map_init_map();
static void window_map_centre_on_view_point();
static void window_map_show_default_scenario_editor_buttons(rct_window* w);
static void window_map_draw_tab_images(rct_window* w, rct_drawpixelinfo* dpi);
static void window_map_paint_peep_overlay(rct_drawpixelinfo* dpi);
static void window_map_paint_train_overlay(rct_drawpixelinfo* dpi);
static void window_map_paint_hud_rectangle(rct_drawpixelinfo* dpi);
static void window_map_inputsize_land(rct_window* w);
static void window_map_inputsize_map(rct_window* w);

static void window_map_set_land_rights_tool_update(int32_t x, int32_t y);
static void window_map_place_park_entrance_tool_update(int32_t x, int32_t y);
static void window_map_set_peep_spawn_tool_update(int32_t x, int32_t y);
static void window_map_place_park_entrance_tool_down(int32_t x, int32_t y);
static void window_map_set_peep_spawn_tool_down(int32_t x, int32_t y);
static void map_window_increase_map_size();
static void map_window_decrease_map_size();
static void map_window_set_pixels(rct_window* w);

static CoordsXY map_window_screen_to_map(int32_t screenX, int32_t screenY);

/**
 *
 *  rct2: 0x0068C88A
 */
rct_window* window_map_open()
{
    rct_window* w;

    // Check if window is already open
    w = window_bring_to_front_by_class(WC_MAP);
    if (w != nullptr)
    {
        w->selected_tab = 0;
        w->list_information_type = 0;
        return w;
    }

    try
    {
        _mapImageData.resize(MAP_WINDOW_MAP_SIZE * MAP_WINDOW_MAP_SIZE);
    }
    catch (const std::bad_alloc&)
    {
        return nullptr;
    }

    w = window_create_auto_pos(245, 259, &window_map_events, WC_MAP, WF_10);
    w->widgets = window_map_widgets;
    w->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_PEOPLE_TAB) | (1 << WIDX_RIDES_TAB) | (1 << WIDX_MAP_SIZE_SPINNER)
        | (1 << WIDX_MAP_SIZE_SPINNER_UP) | (1 << WIDX_MAP_SIZE_SPINNER_DOWN) | (1 << WIDX_LAND_TOOL)
        | (1 << WIDX_LAND_TOOL_SMALLER) | (1 << WIDX_LAND_TOOL_LARGER) | (1 << WIDX_SET_LAND_RIGHTS)
        | (1 << WIDX_LAND_OWNED_CHECKBOX) | (1 << WIDX_CONSTRUCTION_RIGHTS_OWNED_CHECKBOX) | (1 << WIDX_LAND_SALE_CHECKBOX)
        | (1 << WIDX_CONSTRUCTION_RIGHTS_SALE_CHECKBOX) | (1 << WIDX_BUILD_PARK_ENTRANCE) | (1 << WIDX_ROTATE_90)
        | (1 << WIDX_PEOPLE_STARTING_POSITION) | (1 << WIDX_MAP_GENERATOR);

    w->hold_down_widgets = (1 << WIDX_MAP_SIZE_SPINNER_UP) | (1 << WIDX_MAP_SIZE_SPINNER_DOWN) | (1 << WIDX_LAND_TOOL_LARGER)
        | (1 << WIDX_LAND_TOOL_SMALLER);

    window_init_scroll_widgets(w);

    w->map.rotation = get_current_rotation();

    window_map_init_map();
    gWindowSceneryRotation = 0;
    window_map_centre_on_view_point();

    // Reset land rights tool size
    _landRightsToolSize = 1;

    return w;
}

void window_map_reset()
{
    rct_window* w;

    // Check if window is even opened
    w = window_bring_to_front_by_class(WC_MAP);
    if (w == nullptr)
    {
        return;
    }

    window_map_init_map();
    window_map_centre_on_view_point();
}

/**
 *
 *  rct2: 0x0068D0F1
 */
static void window_map_close(rct_window* w)
{
    _mapImageData.clear();
    _mapImageData.shrink_to_fit();
    if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE)) && gCurrentToolWidget.window_classification == w->classification
        && gCurrentToolWidget.window_number == w->number)
    {
        tool_cancel();
    }
}

/**
 *
 *  rct2: 0x0068CFC1
 */
static void window_map_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_CLOSE:
            window_close(w);
            break;
        case WIDX_SET_LAND_RIGHTS:
            w->Invalidate();
            if (tool_set(w, widgetIndex, TOOL_UP_ARROW))
                break;
            _activeTool = 2;
            // Prevent mountain tool size.
            _landRightsToolSize = std::max<uint16_t>(MINIMUM_TOOL_SIZE, _landRightsToolSize);
            show_gridlines();
            show_land_rights();
            show_construction_rights();
            break;
        case WIDX_LAND_OWNED_CHECKBOX:
            _activeTool ^= 2;

            if (_activeTool & 2)
                _activeTool &= 0xF2;

            w->Invalidate();
            break;
        case WIDX_LAND_SALE_CHECKBOX:
            _activeTool ^= 8;

            if (_activeTool & 8)
                _activeTool &= 0xF8;

            w->Invalidate();
            break;
        case WIDX_CONSTRUCTION_RIGHTS_OWNED_CHECKBOX:
            _activeTool ^= 1;

            if (_activeTool & 1)
                _activeTool &= 0xF1;

            w->Invalidate();
            break;
        case WIDX_CONSTRUCTION_RIGHTS_SALE_CHECKBOX:
            _activeTool ^= 4;

            if (_activeTool & 4)
                _activeTool &= 0xF4;

            w->Invalidate();
            break;
        case WIDX_BUILD_PARK_ENTRANCE:
            w->Invalidate();
            if (tool_set(w, widgetIndex, TOOL_UP_ARROW))
                break;

            gParkEntranceGhostExists = false;
            input_set_flag(INPUT_FLAG_6, true);

            show_gridlines();
            show_land_rights();
            show_construction_rights();
            break;
        case WIDX_ROTATE_90:
            gWindowSceneryRotation = (gWindowSceneryRotation + 1) & 3;
            break;
        case WIDX_PEOPLE_STARTING_POSITION:
            if (tool_set(w, widgetIndex, TOOL_UP_ARROW))
                break;

            show_gridlines();
            show_land_rights();
            show_construction_rights();
            break;
        case WIDX_LAND_TOOL:
            window_map_inputsize_land(w);
            break;
        case WIDX_MAP_SIZE_SPINNER:
            window_map_inputsize_map(w);
            break;
        case WIDX_MAP_GENERATOR:
            context_open_window(WC_MAPGEN);
            break;
        default:
            if (widgetIndex >= WIDX_PEOPLE_TAB && widgetIndex <= WIDX_RIDES_TAB)
            {
                widgetIndex -= WIDX_PEOPLE_TAB;
                if (widgetIndex == w->selected_tab)
                    break;

                w->selected_tab = widgetIndex;
                w->list_information_type = 0;
            }
    }
}

/**
 *
 *  rct2: 0x0068D7DC
 */
static void window_map_resize(rct_window* w)
{
    w->flags |= WF_RESIZABLE;
    w->min_width = 245;
    w->max_width = 800;
    w->min_height = 259;
    w->max_height = 560;
}

/**
 *
 *  rct2: 0x0068D040
 */
static void window_map_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex)
    {
        case WIDX_MAP_SIZE_SPINNER_UP:
            map_window_increase_map_size();
            break;
        case WIDX_MAP_SIZE_SPINNER_DOWN:
            map_window_decrease_map_size();
            break;
        case WIDX_LAND_TOOL_SMALLER:
            // Decrement land rights tool size
            _landRightsToolSize = std::max(MINIMUM_TOOL_SIZE, _landRightsToolSize - 1);

            w->Invalidate();
            break;
        case WIDX_LAND_TOOL_LARGER:
            // Increment land rights tool size
            _landRightsToolSize = std::min(MAXIMUM_TOOL_SIZE, _landRightsToolSize + 1);

            w->Invalidate();
            break;
    }
}

/**
 *
 *  rct2: 0x0068D7FB
 */
static void window_map_update(rct_window* w)
{
    if (get_current_rotation() != w->map.rotation)
    {
        w->map.rotation = get_current_rotation();
        window_map_init_map();
        window_map_centre_on_view_point();
    }

    for (int32_t i = 0; i < 16; i++)
        map_window_set_pixels(w);

    w->Invalidate();

    // Update tab animations
    w->list_information_type++;
    switch (w->selected_tab)
    {
        case PAGE_PEEPS:
            if (w->list_information_type >= 32)
            {
                w->list_information_type = 0;
            }
            break;
        case PAGE_RIDES:
            if (w->list_information_type >= 64)
            {
                w->list_information_type = 0;
            }
            break;
    }
}

/**
 *
 *  rct2: 0x0068D093
 */
static void window_map_toolupdate(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_SET_LAND_RIGHTS:
            window_map_set_land_rights_tool_update(screenCoords.x, screenCoords.y);
            break;
        case WIDX_BUILD_PARK_ENTRANCE:
            window_map_place_park_entrance_tool_update(screenCoords.x, screenCoords.y);
            break;
        case WIDX_PEOPLE_STARTING_POSITION:
            window_map_set_peep_spawn_tool_update(screenCoords.x, screenCoords.y);
            break;
    }
}

/**
 *
 *  rct2: 0x0068D074
 */
static void window_map_tooldown(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_BUILD_PARK_ENTRANCE:
            window_map_place_park_entrance_tool_down(screenCoords.x, screenCoords.y);
            break;
        case WIDX_PEOPLE_STARTING_POSITION:
            window_map_set_peep_spawn_tool_down(screenCoords.x, screenCoords.y);
            break;
    }
}

/**
 *
 *  rct2: 0x0068D088
 */
static void window_map_tooldrag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_SET_LAND_RIGHTS:
            if (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE)
            {
                auto landSetRightsAction = LandSetRightsAction(
                    { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                    LandSetRightSetting::SetOwnershipWithChecks, _activeTool << 4);
                GameActions::Execute(&landSetRightsAction);
            }
            break;
    }
}

/**
 *
 *  rct2: 0x0068D055
 */
static void window_map_toolabort(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_SET_LAND_RIGHTS:
            w->Invalidate();
            hide_gridlines();
            hide_land_rights();
            hide_construction_rights();
            break;
        case WIDX_BUILD_PARK_ENTRANCE:
            park_entrance_remove_ghost();
            w->Invalidate();
            hide_gridlines();
            hide_land_rights();
            hide_construction_rights();
            break;
        case WIDX_PEOPLE_STARTING_POSITION:
            w->Invalidate();
            hide_gridlines();
            hide_land_rights();
            hide_construction_rights();
            break;
    }
}

/**
 *
 *  rct2: 0x0068D7CC
 */
static void window_map_scrollgetsize(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height)
{
    window_map_invalidate(w);

    *width = MAP_WINDOW_MAP_SIZE;
    *height = MAP_WINDOW_MAP_SIZE;
}

/**
 *
 *  rct2: 0x0068D726
 */
static void window_map_scrollmousedown(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y)
{
    CoordsXY c = map_window_screen_to_map(x, y);
    int32_t mapX = std::clamp(c.x, 0, MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1);
    int32_t mapY = std::clamp(c.y, 0, MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1);
    int32_t mapZ = tile_element_height({ x, y });

    rct_window* mainWindow = window_get_main();
    if (mainWindow != nullptr)
    {
        window_scroll_to_location(mainWindow, mapX, mapY, mapZ);
    }

    if (land_tool_is_active())
    {
        // Set land terrain
        int32_t landToolSize = std::max<int32_t>(1, gLandToolSize);
        int32_t size = (landToolSize * 32) - 32;
        int32_t radius = (landToolSize * 16) - 16;
        mapX = (mapX - radius) & 0xFFE0;
        mapY = (mapY - radius) & 0xFFE0;

        map_invalidate_selection_rect();
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        gMapSelectType = MAP_SELECT_TYPE_FULL;
        gMapSelectPositionA.x = mapX;
        gMapSelectPositionA.y = mapY;
        gMapSelectPositionB.x = mapX + size;
        gMapSelectPositionB.y = mapY + size;
        map_invalidate_selection_rect();

        auto surfaceSetStyleAction = SurfaceSetStyleAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
            gLandToolTerrainSurface, gLandToolTerrainEdge);
        GameActions::Execute(&surfaceSetStyleAction);
    }
    else if (widget_is_active_tool(w, WIDX_SET_LAND_RIGHTS))
    {
        // Set land rights
        int32_t landRightsToolSize = std::max<int32_t>(1, _landRightsToolSize);
        int32_t size = (landRightsToolSize * 32) - 32;
        int32_t radius = (landRightsToolSize * 16) - 16;
        mapX = (mapX - radius) & 0xFFE0;
        mapY = (mapY - radius) & 0xFFE0;

        map_invalidate_selection_rect();
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        gMapSelectType = MAP_SELECT_TYPE_FULL;
        gMapSelectPositionA.x = mapX;
        gMapSelectPositionA.y = mapY;
        gMapSelectPositionB.x = mapX + size;
        gMapSelectPositionB.y = mapY + size;
        map_invalidate_selection_rect();

        auto landSetRightsAction = LandSetRightsAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
            LandSetRightSetting::SetOwnershipWithChecks, _activeTool << 4);
        GameActions::Execute(&landSetRightsAction);
    }
}

static void window_map_textinput(rct_window* w, rct_widgetindex widgetIndex, char* text)
{
    int32_t size;
    char* end;

    if (text == nullptr)
        return;

    switch (widgetIndex)
    {
        case WIDX_LAND_TOOL:
            size = strtol(text, &end, 10);
            if (*end == '\0')
            {
                size = std::clamp(size, MINIMUM_TOOL_SIZE, MAXIMUM_TOOL_SIZE);
                _landRightsToolSize = size;
                w->Invalidate();
            }
            break;
        case WIDX_MAP_SIZE_SPINNER:
            size = strtol(text, &end, 10);
            if (*end == '\0')
            {
                // The practical size is 2 lower than the technical size
                size += 2;
                size = std::clamp(size, MINIMUM_MAP_SIZE_TECHNICAL, MAXIMUM_MAP_SIZE_TECHNICAL);

                int32_t currentSize = gMapSize;
                while (size < currentSize)
                {
                    map_window_decrease_map_size();
                    currentSize--;
                }
                while (size > currentSize)
                {
                    map_window_increase_map_size();
                    currentSize++;
                }
                w->Invalidate();
            }
            break;
    }
}

/**
 *
 *  rct2: 0x0068CA8F
 */
static void window_map_invalidate(rct_window* w)
{
    uint64_t pressedWidgets;
    int32_t i, height;

    // Set the pressed widgets
    pressedWidgets = w->pressed_widgets;
    pressedWidgets &= (1ULL << WIDX_PEOPLE_TAB);
    pressedWidgets &= (1ULL << WIDX_RIDES_TAB);
    pressedWidgets &= (1ULL << WIDX_MAP);
    pressedWidgets &= (1ULL << WIDX_LAND_OWNED_CHECKBOX);
    pressedWidgets &= (1ULL << WIDX_CONSTRUCTION_RIGHTS_OWNED_CHECKBOX);
    pressedWidgets &= (1ULL << WIDX_LAND_SALE_CHECKBOX);
    pressedWidgets &= (1ULL << WIDX_CONSTRUCTION_RIGHTS_SALE_CHECKBOX);

    pressedWidgets |= (1ULL << (WIDX_PEOPLE_TAB + w->selected_tab));
    pressedWidgets |= (1ULL << WIDX_LAND_TOOL);

    if (_activeTool & (1 << 3))
        pressedWidgets |= (1 << WIDX_LAND_SALE_CHECKBOX);

    if (_activeTool & (1 << 2))
        pressedWidgets |= (1 << WIDX_CONSTRUCTION_RIGHTS_SALE_CHECKBOX);

    if (_activeTool & (1 << 1))
        pressedWidgets |= (1 << WIDX_LAND_OWNED_CHECKBOX);

    if (_activeTool & (1 << 0))
        pressedWidgets |= (1 << WIDX_CONSTRUCTION_RIGHTS_OWNED_CHECKBOX);

    w->pressed_widgets = pressedWidgets;

    // Resize widgets to window size
    w->widgets[WIDX_BACKGROUND].right = w->width - 1;
    w->widgets[WIDX_BACKGROUND].bottom = w->height - 1;
    w->widgets[WIDX_RESIZE].right = w->width - 1;
    w->widgets[WIDX_RESIZE].bottom = w->height - 1;
    w->widgets[WIDX_TITLE].right = w->width - 2;
    w->widgets[WIDX_CLOSE].left = w->width - 2 - 11;
    w->widgets[WIDX_CLOSE].right = w->width - 2 - 11 + 10;
    w->widgets[WIDX_MAP].right = w->width - 4;

    if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode)
        w->widgets[WIDX_MAP].bottom = w->height - 1 - 72;
    else if (w->selected_tab == PAGE_RIDES)
        w->widgets[WIDX_MAP].bottom = w->height - 1 - (4 * LIST_ROW_HEIGHT + 4);
    else
        w->widgets[WIDX_MAP].bottom = w->height - 1 - 14;

    w->widgets[WIDX_MAP_SIZE_SPINNER].top = w->height - 15;
    w->widgets[WIDX_MAP_SIZE_SPINNER].bottom = w->height - 4;
    w->widgets[WIDX_MAP_SIZE_SPINNER_UP].top = w->height - 14;
    w->widgets[WIDX_MAP_SIZE_SPINNER_UP].bottom = w->height - 5;
    w->widgets[WIDX_MAP_SIZE_SPINNER_DOWN].top = w->height - 14;
    w->widgets[WIDX_MAP_SIZE_SPINNER_DOWN].bottom = w->height - 5;

    w->widgets[WIDX_SET_LAND_RIGHTS].top = w->height - 70;
    w->widgets[WIDX_SET_LAND_RIGHTS].bottom = w->height - 70 + 23;
    w->widgets[WIDX_BUILD_PARK_ENTRANCE].top = w->height - 46;
    w->widgets[WIDX_BUILD_PARK_ENTRANCE].bottom = w->height - 46 + 23;
    w->widgets[WIDX_ROTATE_90].top = w->height - 46;
    w->widgets[WIDX_ROTATE_90].bottom = w->height - 46 + 23;
    w->widgets[WIDX_PEOPLE_STARTING_POSITION].top = w->height - 46;
    w->widgets[WIDX_PEOPLE_STARTING_POSITION].bottom = w->height - 46 + 23;

    w->widgets[WIDX_LAND_TOOL].top = w->height - 42;
    w->widgets[WIDX_LAND_TOOL].bottom = w->height - 42 + 30;
    w->widgets[WIDX_LAND_TOOL_SMALLER].top = w->height - 41;
    w->widgets[WIDX_LAND_TOOL_SMALLER].bottom = w->height - 41 + 15;
    w->widgets[WIDX_LAND_TOOL_LARGER].top = w->height - 27;
    w->widgets[WIDX_LAND_TOOL_LARGER].bottom = w->height - 27 + 15;

    w->widgets[WIDX_MAP_GENERATOR].top = w->height - 69;
    w->widgets[WIDX_MAP_GENERATOR].bottom = w->height - 69 + 11;

    // Land tool mode (4 checkboxes)
    height = w->height - 55;
    for (i = 0; i < 4; i++)
    {
        w->widgets[WIDX_LAND_OWNED_CHECKBOX + i].top = height;
        height += 11;
        w->widgets[WIDX_LAND_OWNED_CHECKBOX + i].bottom = height;
        height += 2;
    }

    // Disable all scenario editor related widgets
    for (i = WIDX_MAP_SIZE_SPINNER; i <= WIDX_MAP_GENERATOR; i++)
    {
        w->widgets[i].type = WWT_EMPTY;
    }

    if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode)
    {
        // scenario editor: build park entrance selected, show rotate button
        if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE)) && gCurrentToolWidget.window_classification == WC_MAP
            && gCurrentToolWidget.widget_index == WIDX_BUILD_PARK_ENTRANCE)
        {
            w->widgets[WIDX_ROTATE_90].type = WWT_FLATBTN;
        }

        // Always show set land rights button
        w->widgets[WIDX_SET_LAND_RIGHTS].type = WWT_FLATBTN;

        // If any tool is active
        if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE)) && gCurrentToolWidget.window_classification == WC_MAP)
        {
            // if not in set land rights mode: show the default scenario editor buttons
            if (gCurrentToolWidget.widget_index != WIDX_SET_LAND_RIGHTS)
            {
                window_map_show_default_scenario_editor_buttons(w);
            }
            else
            { // if in set land rights mode: show land tool buttons + modes
                w->widgets[WIDX_LAND_TOOL].type = WWT_IMGBTN;
                w->widgets[WIDX_LAND_TOOL_SMALLER].type = WWT_TRNBTN;
                w->widgets[WIDX_LAND_TOOL_LARGER].type = WWT_TRNBTN;

                for (i = 0; i < 4; i++)
                    w->widgets[WIDX_LAND_OWNED_CHECKBOX + i].type = WWT_CHECKBOX;

                w->widgets[WIDX_LAND_TOOL].image = land_tool_size_to_sprite_index(_landRightsToolSize);
            }
        }
        else
        {
            // if no tool is active: show the default scenario editor buttons
            window_map_show_default_scenario_editor_buttons(w);
        }
    }
}

/**
 *
 *  rct2: 0x0068CDA9
 */
static void window_map_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    window_draw_widgets(w, dpi);
    window_map_draw_tab_images(w, dpi);

    int32_t x = w->x + (window_map_widgets[WIDX_LAND_TOOL].left + window_map_widgets[WIDX_LAND_TOOL].right) / 2;
    int32_t y = w->y + (window_map_widgets[WIDX_LAND_TOOL].top + window_map_widgets[WIDX_LAND_TOOL].bottom) / 2;

    // Draw land tool size
    if (widget_is_active_tool(w, WIDX_SET_LAND_RIGHTS) && _landRightsToolSize > MAX_TOOL_SIZE_WITH_SPRITE)
    {
        gfx_draw_string_centred(dpi, STR_LAND_TOOL_SIZE_VALUE, x, y - 2, COLOUR_BLACK, &_landRightsToolSize);
    }
    y = w->y + window_map_widgets[WIDX_LAND_TOOL].bottom + 5;

    // People starting position (scenario editor only)
    if (w->widgets[WIDX_PEOPLE_STARTING_POSITION].type != WWT_EMPTY)
    {
        x = w->x + w->widgets[WIDX_PEOPLE_STARTING_POSITION].left + 12;
        y = w->y + w->widgets[WIDX_PEOPLE_STARTING_POSITION].top + 18;
        gfx_draw_sprite(
            dpi, IMAGE_TYPE_REMAP | IMAGE_TYPE_REMAP_2_PLUS | (COLOUR_LIGHT_BROWN << 24) | (COLOUR_BRIGHT_RED << 19) | SPR_6410,
            x, y, 0);
    }

    if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode)
    {
        // Render the map legend
        if (w->selected_tab == PAGE_RIDES)
        {
            x = w->x + 4;
            y = w->y + w->widgets[WIDX_MAP].bottom + 2;

            static rct_string_id mapLabels[] = {
                STR_MAP_RIDE,       STR_MAP_FOOD_STALL, STR_MAP_DRINK_STALL,  STR_MAP_SOUVENIR_STALL,
                STR_MAP_INFO_KIOSK, STR_MAP_FIRST_AID,  STR_MAP_CASH_MACHINE, STR_MAP_TOILET,
            };

            for (uint32_t i = 0; i < std::size(RideKeyColours); i++)
            {
                gfx_fill_rect(dpi, x, y + 2, x + 6, y + 8, RideKeyColours[i]);
                gfx_draw_string_left(dpi, mapLabels[i], w, COLOUR_BLACK, x + LIST_ROW_HEIGHT, y);
                y += LIST_ROW_HEIGHT;
                if (i == 3)
                {
                    x += 118;
                    y -= LIST_ROW_HEIGHT * 4;
                }
            }
        }
    }
    else if (!widget_is_active_tool(w, WIDX_SET_LAND_RIGHTS))
    {
        gfx_draw_string_left(
            dpi, STR_MAP_SIZE, nullptr, w->colours[1], w->x + 4, w->y + w->widgets[WIDX_MAP_SIZE_SPINNER].top + 1);
    }
}

/**
 *
 *  rct2: 0x0068CF23
 */
static void window_map_scrollpaint(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex)
{
    gfx_clear(dpi, PALETTE_INDEX_10);

    rct_g1_element g1temp = {};
    g1temp.offset = _mapImageData.data();
    g1temp.width = MAP_WINDOW_MAP_SIZE;
    g1temp.height = MAP_WINDOW_MAP_SIZE;
    g1temp.x_offset = -8;
    g1temp.y_offset = -8;
    gfx_set_g1_element(SPR_TEMP, &g1temp);
    drawing_engine_invalidate_image(SPR_TEMP);
    gfx_draw_sprite(dpi, SPR_TEMP, 0, 0, 0);

    if (w->selected_tab == PAGE_PEEPS)
    {
        window_map_paint_peep_overlay(dpi);
    }
    else
    {
        window_map_paint_train_overlay(dpi);
    }
    window_map_paint_hud_rectangle(dpi);
}

/**
 *
 *  rct2: 0x0068CA6C
 */
static void window_map_init_map()
{
    std::fill(_mapImageData.begin(), _mapImageData.end(), PALETTE_INDEX_10);
    _currentLine = 0;
}

/**
 *
 *  rct2: 0x0068C990
 */
static void window_map_centre_on_view_point()
{
    rct_window* w = window_get_main();
    rct_window* w_map;
    int16_t ax, bx, cx, dx;
    int16_t bp, di;

    if (w == nullptr || w->viewport == nullptr)
        return;

    w_map = window_find_by_class(WC_MAP);
    if (w_map == nullptr)
        return;

    LocationXY16 offset = MiniMapOffsets[get_current_rotation()];

    // calculate centre view point of viewport and transform it to minimap coordinates

    cx = ((w->viewport->view_width >> 1) + w->viewport->view_x) >> 5;
    dx = ((w->viewport->view_height >> 1) + w->viewport->view_y) >> 4;
    cx += offset.x;
    dx += offset.y;

    // calculate width and height of minimap

    ax = w_map->widgets[WIDX_MAP].right - w_map->widgets[WIDX_MAP].left - 11;
    bx = w_map->widgets[WIDX_MAP].bottom - w_map->widgets[WIDX_MAP].top - 11;
    bp = ax;
    di = bx;

    ax >>= 1;
    bx >>= 1;
    cx = std::max(cx - ax, 0);
    dx = std::max(dx - bx, 0);

    bp = w_map->scrolls[0].h_right - bp;
    di = w_map->scrolls[0].v_bottom - di;

    if (bp < 0 && (bp - cx) < 0)
        cx = 0;

    if (di < 0 && (di - dx) < 0)
        dx = 0;

    w_map->scrolls[0].h_left = cx;
    w_map->scrolls[0].v_top = dx;
    widget_scroll_update_thumbs(w_map, WIDX_MAP);
}

/**
 *
 *  rct2: 0x0068CD35 (part of 0x0068CA8F)
 */
static void window_map_show_default_scenario_editor_buttons(rct_window* w)
{
    w->widgets[WIDX_BUILD_PARK_ENTRANCE].type = WWT_FLATBTN;
    w->widgets[WIDX_PEOPLE_STARTING_POSITION].type = WWT_FLATBTN;
    w->widgets[WIDX_MAP_SIZE_SPINNER].type = WWT_SPINNER;
    w->widgets[WIDX_MAP_SIZE_SPINNER_UP].type = WWT_BUTTON;
    w->widgets[WIDX_MAP_SIZE_SPINNER_DOWN].type = WWT_BUTTON;

    // Only show this in the scenario editor, even when in sandbox mode.
    if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
        w->widgets[WIDX_MAP_GENERATOR].type = WWT_BUTTON;

    set_format_arg(2, uint16_t, gMapSize - 2);
}

static void window_map_inputsize_land(rct_window* w)
{
    TextInputDescriptionArgs[0] = MINIMUM_TOOL_SIZE;
    TextInputDescriptionArgs[1] = MAXIMUM_TOOL_SIZE;
    window_text_input_open(w, WIDX_LAND_TOOL, STR_SELECTION_SIZE, STR_ENTER_SELECTION_SIZE, STR_NONE, STR_NONE, 3);
}

static void window_map_inputsize_map(rct_window* w)
{
    TextInputDescriptionArgs[0] = MINIMUM_MAP_SIZE_PRACTICAL;
    TextInputDescriptionArgs[1] = MAXIMUM_MAP_SIZE_PRACTICAL;
    window_text_input_open(w, WIDX_MAP_SIZE_SPINNER, STR_MAP_SIZE_2, STR_ENTER_MAP_SIZE, STR_NONE, STR_NONE, 4);
}

static void window_map_draw_tab_images(rct_window* w, rct_drawpixelinfo* dpi)
{
    uint32_t image;

    // Guest tab image (animated)
    image = SPR_TAB_GUESTS_0;
    if (w->selected_tab == PAGE_PEEPS)
        image += w->list_information_type / 4;

    gfx_draw_sprite(dpi, image, w->x + w->widgets[WIDX_PEOPLE_TAB].left, w->y + w->widgets[WIDX_PEOPLE_TAB].top, 0);

    // Ride/stall tab image (animated)
    image = SPR_TAB_RIDE_0;
    if (w->selected_tab == PAGE_RIDES)
        image += w->list_information_type / 4;

    gfx_draw_sprite(dpi, image, w->x + w->widgets[WIDX_RIDES_TAB].left, w->y + w->widgets[WIDX_RIDES_TAB].top, 0);
}

/**
 *
 * part of window_map_paint_peep_overlay and window_map_paint_train_overlay
 */
static MapCoordsXY window_map_transform_to_map_coords(CoordsXY c)
{
    int32_t x = c.x, y = c.y;

    switch (get_current_rotation())
    {
        case 3:
            std::swap(x, y);
            x = MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - x;
            break;
        case 2:
            x = MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - x;
            y = MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - y;
            break;
        case 1:
            std::swap(x, y);
            y = MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - y;
            break;
        case 0:
            break;
    }
    x /= 32;
    y /= 32;

    return { -x + y + MAXIMUM_MAP_SIZE_TECHNICAL - 8, x + y - 8 };
}

/**
 *
 *  rct2: 0x0068DADA
 */
static void window_map_paint_peep_overlay(rct_drawpixelinfo* dpi)
{
    Peep* peep;
    uint16_t spriteIndex;

    FOR_ALL_PEEPS (spriteIndex, peep)
    {
        if (peep->x == LOCATION_NULL)
            continue;

        MapCoordsXY c = window_map_transform_to_map_coords({ peep->x, peep->y });
        int16_t left = c.x;
        int16_t top = c.y;

        int16_t right = left;
        int16_t bottom = top;

        int16_t colour = PALETTE_INDEX_20;

        if (sprite_get_flashing((rct_sprite*)peep))
        {
            if (peep->type == PEEP_TYPE_STAFF)
            {
                if ((gWindowMapFlashingFlags & (1 << 3)) != 0)
                {
                    colour = PALETTE_INDEX_138;
                    left--;
                    if ((gWindowMapFlashingFlags & (1 << 15)) == 0)
                        colour = PALETTE_INDEX_10;
                }
            }
            else
            {
                if ((gWindowMapFlashingFlags & (1 << 1)) != 0)
                {
                    colour = PALETTE_INDEX_172;
                    left--;
                    if ((gWindowMapFlashingFlags & (1 << 15)) == 0)
                        colour = PALETTE_INDEX_21;
                }
            }
        }
        gfx_fill_rect(dpi, left, top, right, bottom, colour);
    }
}

/**
 *
 *  rct2: 0x0068DBC1
 */
static void window_map_paint_train_overlay(rct_drawpixelinfo* dpi)
{
    rct_vehicle *train, *vehicle;
    uint16_t train_index, vehicle_index;

    for (train_index = gSpriteListHead[SPRITE_LIST_VEHICLE_HEAD]; train_index != SPRITE_INDEX_NULL; train_index = train->next)
    {
        train = GET_VEHICLE(train_index);
        for (vehicle_index = train_index; vehicle_index != SPRITE_INDEX_NULL; vehicle_index = vehicle->next_vehicle_on_train)
        {
            vehicle = GET_VEHICLE(vehicle_index);
            if (vehicle->x == LOCATION_NULL)
                continue;

            MapCoordsXY c = window_map_transform_to_map_coords({ vehicle->x, vehicle->y });

            gfx_fill_rect(dpi, c.x, c.y, c.x, c.y, PALETTE_INDEX_171);
        }
    }
}

/**
 * The call to gfx_fill_rect was originally wrapped in sub_68DABD which made sure that arguments were ordered correctly,
 * but it doesn't look like it's ever necessary here so the call was removed.
 *
 *  rct2: 0x0068D8CE
 */
static void window_map_paint_hud_rectangle(rct_drawpixelinfo* dpi)
{
    rct_window* main_window = window_get_main();
    if (main_window == nullptr)
        return;

    rct_viewport* viewport = main_window->viewport;
    if (viewport == nullptr)
        return;

    LocationXY16 offset = MiniMapOffsets[get_current_rotation()];
    int16_t left = (viewport->view_x >> 5) + offset.x;
    int16_t right = ((viewport->view_x + viewport->view_width) >> 5) + offset.x;
    int16_t top = (viewport->view_y >> 4) + offset.y;
    int16_t bottom = ((viewport->view_y + viewport->view_height) >> 4) + offset.y;

    // top horizontal lines
    gfx_fill_rect(dpi, left, top, left + 3, top, PALETTE_INDEX_56);
    gfx_fill_rect(dpi, right - 3, top, right, top, PALETTE_INDEX_56);

    // left vertical lines
    gfx_fill_rect(dpi, left, top, left, top + 3, PALETTE_INDEX_56);
    gfx_fill_rect(dpi, left, bottom - 3, left, bottom, PALETTE_INDEX_56);

    // bottom horizontal lines
    gfx_fill_rect(dpi, left, bottom, left + 3, bottom, PALETTE_INDEX_56);
    gfx_fill_rect(dpi, right - 3, bottom, right, bottom, PALETTE_INDEX_56);

    // right vertical lines
    gfx_fill_rect(dpi, right, top, right, top + 3, PALETTE_INDEX_56);
    gfx_fill_rect(dpi, right, bottom - 3, right, bottom, PALETTE_INDEX_56);
}

/**
 *
 *  rct2: 0x0068D24E
 */
static void window_map_set_land_rights_tool_update(int32_t x, int32_t y)
{
    int16_t mapX, mapY;
    rct_viewport* viewport;

    map_invalidate_selection_rect();
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    screen_get_map_xy(x, y, &mapX, &mapY, &viewport);
    if (mapX == LOCATION_NULL)
        return;

    gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
    gMapSelectType = MAP_SELECT_TYPE_FULL;

    int32_t landRightsToolSize = _landRightsToolSize;
    if (landRightsToolSize == 0)
        landRightsToolSize = 1;

    int32_t size = (landRightsToolSize * 32) - 32;
    int32_t radius = (landRightsToolSize * 16) - 16;
    mapX = (mapX - radius) & 0xFFE0;
    mapY = (mapY - radius) & 0xFFE0;
    gMapSelectPositionA.x = mapX;
    gMapSelectPositionA.y = mapY;
    gMapSelectPositionB.x = mapX + size;
    gMapSelectPositionB.y = mapY + size;
    map_invalidate_selection_rect();
}

/**
 *
 *  rct2: 0x00666EEF
 */
static void place_park_entrance_get_map_position(
    int32_t x, int32_t y, int16_t* mapX, int16_t* mapY, int16_t* mapZ, int32_t* direction)
{
    sub_68A15E(x, y, mapX, mapY);
    if (*mapX == LOCATION_NULL)
        return;

    auto surfaceElement = map_get_surface_element_at(*mapX >> 5, *mapY >> 5);
    *mapZ = surfaceElement->GetWaterHeight();
    if (*mapZ == 0)
    {
        *mapZ = surfaceElement->base_height / 2;
        if ((surfaceElement->GetSlope() & TILE_ELEMENT_SLOPE_ALL_CORNERS_UP) != 0)
        {
            (*mapZ)++;
            if (surfaceElement->GetSlope() & TILE_ELEMENT_SLOPE_DOUBLE_HEIGHT)
            {
                (*mapZ)++;
            }
        }
    }
    *direction = (gWindowSceneryRotation - get_current_rotation()) & 3;
}

/**
 *
 *  rct2: 0x00666FD0
 */
static void window_map_place_park_entrance_tool_update(int32_t x, int32_t y)
{
    int16_t mapX, mapY, mapZ = 0;
    int32_t direction = 0, sideDirection;

    map_invalidate_selection_rect();
    map_invalidate_map_selection_tiles();
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
    place_park_entrance_get_map_position(x, y, &mapX, &mapY, &mapZ, &direction);
    if (mapX == (int16_t)-1)
    {
        park_entrance_remove_ghost();
        return;
    }

    sideDirection = (direction + 1) & 3;
    gMapSelectionTiles.clear();
    gMapSelectionTiles.push_back({ mapX, mapY });
    gMapSelectionTiles.push_back(
        { mapX + CoordsDirectionDelta[sideDirection].x, mapY + CoordsDirectionDelta[sideDirection].y });
    gMapSelectionTiles.push_back(
        { mapX - CoordsDirectionDelta[sideDirection].x, mapY - CoordsDirectionDelta[sideDirection].y });

    gMapSelectArrowPosition.x = mapX;
    gMapSelectArrowPosition.y = mapY;
    gMapSelectArrowPosition.z = mapZ * 16;
    gMapSelectArrowDirection = direction;

    gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_CONSTRUCT | MAP_SELECT_FLAG_ENABLE_ARROW;
    map_invalidate_map_selection_tiles();
    if (gParkEntranceGhostExists && mapX == gParkEntranceGhostPosition.x && mapY == gParkEntranceGhostPosition.y
        && direction == gParkEntranceGhostDirection)
    {
        return;
    }

    park_entrance_remove_ghost();
    park_entrance_place_ghost(mapX, mapY, mapZ, direction);
}

/**
 *
 *  rct2: 0x0068D4E9
 */
static void window_map_set_peep_spawn_tool_update(int32_t x, int32_t y)
{
    int32_t mapX, mapY, mapZ, direction;
    TileElement* tileElement;

    map_invalidate_selection_rect();
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_ARROW;
    footpath_bridge_get_info_from_pos(x, y, &mapX, &mapY, &direction, &tileElement);
    if ((mapX & 0xFFFF) == 0x8000)
        return;

    mapZ = tileElement->base_height * 8;
    if (tileElement->GetType() == TILE_ELEMENT_TYPE_SURFACE)
    {
        if ((tileElement->AsSurface()->GetSlope() & TILE_ELEMENT_SLOPE_ALL_CORNERS_UP) != 0)
            mapZ += 16;
        if (tileElement->AsSurface()->GetSlope() & TILE_ELEMENT_SLOPE_DOUBLE_HEIGHT)
            mapZ += 16;
    }

    gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
    gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_ARROW;
    gMapSelectType = MAP_SELECT_TYPE_FULL;
    gMapSelectPositionA.x = mapX;
    gMapSelectPositionA.y = mapY;
    gMapSelectPositionB.x = mapX;
    gMapSelectPositionB.y = mapY;
    gMapSelectArrowDirection = direction_reverse(direction);
    gMapSelectArrowPosition.x = mapX;
    gMapSelectArrowPosition.y = mapY;
    gMapSelectArrowPosition.z = mapZ;
    map_invalidate_selection_rect();
}

/**
 *
 *  rct2: 0x006670A4
 */
static void window_map_place_park_entrance_tool_down(int32_t x, int32_t y)
{
    park_entrance_remove_ghost();

    int16_t mapX, mapY, mapZ;
    int32_t direction;
    place_park_entrance_get_map_position(x, y, &mapX, &mapY, &mapZ, &direction);
    if (mapX != LOCATION_NULL)
    {
        money32 price = place_park_entrance(mapX, mapY, mapZ, direction);
        if (price != MONEY32_UNDEFINED)
        {
            audio_play_sound_at_location(SoundId::PlaceItem, { gCommandPosition.x, gCommandPosition.y, gCommandPosition.z });
        }
    }
}

/**
 *
 *  rct2: 0x0068D573
 */
static void window_map_set_peep_spawn_tool_down(int32_t x, int32_t y)
{
    TileElement* tileElement;
    int32_t mapX, mapY, mapZ, direction;

    // Verify footpath exists at location, and retrieve coordinates
    footpath_get_coordinates_from_pos(x, y, &mapX, &mapY, &direction, &tileElement);
    if (mapX == LOCATION_NULL)
        return;

    mapZ = tileElement->base_height * 8;

    bool result = place_peep_spawn({ mapX, mapY, mapZ, (uint8_t)direction });
    if (result)
    {
        audio_play_sound_at_location(SoundId::PlaceItem, { gCommandPosition.x, gCommandPosition.y, gCommandPosition.z });
    }
}

/**
 *
 *  rct2: 0x0068D641
 */
static void map_window_increase_map_size()
{
    if (gMapSize >= MAXIMUM_MAP_SIZE_TECHNICAL)
    {
        context_show_error(STR_CANT_INCREASE_MAP_SIZE_ANY_FURTHER, STR_NONE);
        return;
    }

    gMapSize++;
    gMapSizeUnits = (gMapSize - 1) * 32;
    gMapSizeMinus2 = (gMapSize * 32) + MAXIMUM_MAP_SIZE_PRACTICAL;
    gMapSizeMaxXY = ((gMapSize - 1) * 32) - 1;
    map_extend_boundary_surface();
    window_map_init_map();
    window_map_centre_on_view_point();
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0068D6B4
 */
static void map_window_decrease_map_size()
{
    if (gMapSize < 16)
    {
        context_show_error(STR_CANT_DECREASE_MAP_SIZE_ANY_FURTHER, STR_NONE);
        return;
    }

    gMapSize--;
    gMapSizeUnits = (gMapSize - 1) * 32;
    gMapSizeMinus2 = (gMapSize * 32) + MAXIMUM_MAP_SIZE_PRACTICAL;
    gMapSizeMaxXY = ((gMapSize - 1) * 32) - 1;
    map_remove_out_of_range_elements();
    window_map_init_map();
    window_map_centre_on_view_point();
    gfx_invalidate_screen();
}

static constexpr const uint16_t WaterColour = MAP_COLOUR(PALETTE_INDEX_195);
static constexpr const uint16_t TerrainColour[] = {
    MAP_COLOUR(PALETTE_INDEX_73),                      // TERRAIN_GRASS
    MAP_COLOUR(PALETTE_INDEX_40),                      // TERRAIN_SAND
    MAP_COLOUR(PALETTE_INDEX_108),                     // TERRAIN_DIRT
    MAP_COLOUR(PALETTE_INDEX_12),                      // TERRAIN_ROCK
    MAP_COLOUR(PALETTE_INDEX_62),                      // TERRAIN_MARTIAN
    MAP_COLOUR_2(PALETTE_INDEX_10, PALETTE_INDEX_16),  // TERRAIN_CHECKERBOARD
    MAP_COLOUR_2(PALETTE_INDEX_73, PALETTE_INDEX_108), // TERRAIN_GRASS_CLUMPS
    MAP_COLOUR(PALETTE_INDEX_141),                     // TERRAIN_ICE
    MAP_COLOUR_2(PALETTE_INDEX_172, PALETTE_INDEX_10), // TERRAIN_GRID_RED
    MAP_COLOUR_2(PALETTE_INDEX_54, PALETTE_INDEX_10),  // TERRAIN_GRID_YELLOW
    MAP_COLOUR_2(PALETTE_INDEX_162, PALETTE_INDEX_10), // TERRAIN_GRID_BLUE
    MAP_COLOUR_2(PALETTE_INDEX_102, PALETTE_INDEX_10), // TERRAIN_GRID_GREEN
    MAP_COLOUR(PALETTE_INDEX_111),                     // TERRAIN_SAND_DARK
    MAP_COLOUR(PALETTE_INDEX_222),                     // TERRAIN_SAND_LIGHT
};

static constexpr const uint16_t ElementTypeMaskColour[] = {
    0xFFFF, // TILE_ELEMENT_TYPE_SURFACE
    0x0000, // TILE_ELEMENT_TYPE_PATH
    0x00FF, // TILE_ELEMENT_TYPE_TRACK
    0xFF00, // TILE_ELEMENT_TYPE_SMALL_SCENERY
    0x0000, // TILE_ELEMENT_TYPE_ENTRANCE
    0xFFFF, // TILE_ELEMENT_TYPE_WALL
    0x0000, // TILE_ELEMENT_TYPE_LARGE_SCENERY
    0xFFFF, // TILE_ELEMENT_TYPE_BANNER
    0x0000, // TILE_ELEMENT_TYPE_CORRUPT
};

static constexpr const uint16_t ElementTypeAddColour[] = {
    MAP_COLOUR(PALETTE_INDEX_0),                      // TILE_ELEMENT_TYPE_SURFACE
    MAP_COLOUR(PALETTE_INDEX_17),                     // TILE_ELEMENT_TYPE_PATH
    MAP_COLOUR_2(PALETTE_INDEX_183, PALETTE_INDEX_0), // TILE_ELEMENT_TYPE_TRACK
    MAP_COLOUR_2(PALETTE_INDEX_0, PALETTE_INDEX_99),  // TILE_ELEMENT_TYPE_SMALL_SCENERY
    MAP_COLOUR(PALETTE_INDEX_186),                    // TILE_ELEMENT_TYPE_ENTRANCE
    MAP_COLOUR(PALETTE_INDEX_0),                      // TILE_ELEMENT_TYPE_WALL
    MAP_COLOUR(PALETTE_INDEX_99),                     // TILE_ELEMENT_TYPE_LARGE_SCENERY
    MAP_COLOUR(PALETTE_INDEX_0),                      // TILE_ELEMENT_TYPE_BANNER
    MAP_COLOUR(PALETTE_INDEX_68),                     // TILE_ELEMENT_TYPE_CORRUPT
};

enum
{
    COLOUR_KEY_RIDE,
    COLOUR_KEY_FOOD,
    COLOUR_KEY_DRINK,
    COLOUR_KEY_SOUVENIR,
    COLOUR_KEY_KIOSK,
    COLOUR_KEY_FIRST_AID,
    COLOUR_KEY_CASH_MACHINE,
    COLOUR_KEY_TOILETS
};

static constexpr const uint8_t RideColourKey[] = {
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SPIRAL_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_STAND_UP_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SUSPENDED_SWINGING_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_INVERTED_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_JUNIOR_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINIATURE_RAILWAY
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MONORAIL
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINI_SUSPENDED_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_BOAT_HIRE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_WOODEN_WILD_MOUSE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_STEEPLECHASE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_CAR_RIDE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LAUNCHED_FREEFALL
    COLOUR_KEY_RIDE,         // RIDE_TYPE_BOBSLEIGH_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_OBSERVATION_TOWER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LOOPING_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_DINGHY_SLIDE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINE_TRAIN_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_CHAIRLIFT
    COLOUR_KEY_RIDE,         // RIDE_TYPE_CORKSCREW_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MAZE = 20
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SPIRAL_SLIDE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_GO_KARTS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LOG_FLUME
    COLOUR_KEY_RIDE,         // RIDE_TYPE_RIVER_RAPIDS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_DODGEMS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SWINGING_SHIP
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SWINGING_INVERTER_SHIP
    COLOUR_KEY_FOOD,         // RIDE_TYPE_FOOD_STALL
    COLOUR_KEY_FOOD,         // RIDE_TYPE_1D
    COLOUR_KEY_DRINK,        // RIDE_TYPE_DRINK_STALL
    COLOUR_KEY_DRINK,        // RIDE_TYPE_1F
    COLOUR_KEY_SOUVENIR,     // RIDE_TYPE_SHOP
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MERRY_GO_ROUND
    COLOUR_KEY_SOUVENIR,     // RIDE_TYPE_22
    COLOUR_KEY_KIOSK,        // RIDE_TYPE_INFORMATION_KIOSK
    COLOUR_KEY_TOILETS,      // RIDE_TYPE_TOILETS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_FERRIS_WHEEL
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MOTION_SIMULATOR
    COLOUR_KEY_RIDE,         // RIDE_TYPE_3D_CINEMA
    COLOUR_KEY_RIDE,         // RIDE_TYPE_TOP_SPIN
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SPACE_RINGS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_REVERSE_FREEFALL_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LIFT
    COLOUR_KEY_RIDE,         // RIDE_TYPE_VERTICAL_DROP_ROLLER_COASTER
    COLOUR_KEY_CASH_MACHINE, // RIDE_TYPE_CASH_MACHINE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_TWIST
    COLOUR_KEY_RIDE,         // RIDE_TYPE_HAUNTED_HOUSE
    COLOUR_KEY_FIRST_AID,    // RIDE_TYPE_FIRST_AID
    COLOUR_KEY_RIDE,         // RIDE_TYPE_CIRCUS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_GHOST_TRAIN
    COLOUR_KEY_RIDE,         // RIDE_TYPE_TWISTER_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_WOODEN_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SIDE_FRICTION_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_STEEL_WILD_MOUSE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MULTI_DIMENSION_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MULTI_DIMENSION_ROLLER_COASTER_ALT
    COLOUR_KEY_RIDE,         // RIDE_TYPE_FLYING_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_FLYING_ROLLER_COASTER_ALT
    COLOUR_KEY_RIDE,         // RIDE_TYPE_VIRGINIA_REEL
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SPLASH_BOATS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINI_HELICOPTERS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LAY_DOWN_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SUSPENDED_MONORAIL
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LAY_DOWN_ROLLER_COASTER_ALT
    COLOUR_KEY_RIDE,         // RIDE_TYPE_REVERSER_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_HEARTLINE_TWISTER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINI_GOLF
    COLOUR_KEY_RIDE,         // RIDE_TYPE_GIGA_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_ROTO_DROP
    COLOUR_KEY_RIDE,         // RIDE_TYPE_FLYING_SAUCERS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_CROOKED_HOUSE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MONORAIL_CYCLES
    COLOUR_KEY_RIDE,         // RIDE_TYPE_COMPACT_INVERTED_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_WATER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_AIR_POWERED_VERTICAL_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_INVERTED_HAIRPIN_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MAGIC_CARPET
    COLOUR_KEY_RIDE,         // RIDE_TYPE_SUBMARINE_RIDE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_RIVER_RAFTS
    COLOUR_KEY_RIDE,         // RIDE_TYPE_50
    COLOUR_KEY_RIDE,         // RIDE_TYPE_ENTERPRISE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_52
    COLOUR_KEY_RIDE,         // RIDE_TYPE_53
    COLOUR_KEY_RIDE,         // RIDE_TYPE_54
    COLOUR_KEY_RIDE,         // RIDE_TYPE_55
    COLOUR_KEY_RIDE,         // RIDE_TYPE_INVERTED_IMPULSE_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINI_ROLLER_COASTER
    COLOUR_KEY_RIDE,         // RIDE_TYPE_MINE_RIDE
    COLOUR_KEY_RIDE,         // RIDE_TYPE_59
    COLOUR_KEY_RIDE,         // RIDE_TYPE_LIM_LAUNCHED_ROLLER_COASTER
    COLOUR_KEY_RIDE,         //
    COLOUR_KEY_RIDE,         //
    COLOUR_KEY_RIDE,         //
};

static uint16_t map_window_get_pixel_colour_peep(CoordsXY c)
{
    auto* surfaceElement = map_get_surface_element_at(c);
    uint16_t colour = TerrainColour[surfaceElement->GetSurfaceStyle()];
    if (surfaceElement->GetWaterHeight() > 0)
        colour = WaterColour;

    if (!(surfaceElement->GetOwnership() & OWNERSHIP_OWNED))
        colour = MAP_COLOUR_UNOWNED(colour);

    const int32_t maxSupportedTileElementType = (int32_t)std::size(ElementTypeAddColour);
    auto tileElement = reinterpret_cast<TileElement*>(surfaceElement);
    while (!(tileElement++)->IsLastForTile())
    {
        if (tileElement->IsGhost())
        {
            colour = MAP_COLOUR(PALETTE_INDEX_21);
            break;
        }

        int32_t tileElementType = tileElement->GetType() >> 2;
        if (tileElementType >= maxSupportedTileElementType)
        {
            tileElementType = TILE_ELEMENT_TYPE_CORRUPT >> 2;
        }
        colour &= ElementTypeMaskColour[tileElementType];
        colour |= ElementTypeAddColour[tileElementType];
    }

    return colour;
}

static uint16_t map_window_get_pixel_colour_ride(CoordsXY c)
{
    Ride* ride;
    uint16_t colourA = 0;                            // highlight colour
    uint16_t colourB = MAP_COLOUR(PALETTE_INDEX_13); // surface colour (dark grey)

    // as an improvement we could use first_element to show underground stuff?
    TileElement* tileElement = reinterpret_cast<TileElement*>(map_get_surface_element_at(c));
    do
    {
        if (tileElement->IsGhost())
        {
            colourA = MAP_COLOUR(PALETTE_INDEX_21);
            break;
        }

        switch (tileElement->GetType())
        {
            case TILE_ELEMENT_TYPE_SURFACE:
                if (tileElement->AsSurface()->GetWaterHeight() > 0)
                    // Why is this a different water colour as above (195)?
                    colourB = MAP_COLOUR(PALETTE_INDEX_194);
                if (!(tileElement->AsSurface()->GetOwnership() & OWNERSHIP_OWNED))
                    colourB = MAP_COLOUR_UNOWNED(colourB);
                break;
            case TILE_ELEMENT_TYPE_PATH:
                colourA = MAP_COLOUR(PALETTE_INDEX_14); // lighter grey
                break;
            case TILE_ELEMENT_TYPE_ENTRANCE:
                if (tileElement->AsEntrance()->GetEntranceType() == ENTRANCE_TYPE_PARK_ENTRANCE)
                    break;
                ride = get_ride(tileElement->AsEntrance()->GetRideIndex());
                if (ride != nullptr)
                    colourA = RideKeyColours[RideColourKey[ride->type]];
                break;
            case TILE_ELEMENT_TYPE_TRACK:
                ride = get_ride(tileElement->AsTrack()->GetRideIndex());
                if (ride != nullptr)
                    colourA = RideKeyColours[RideColourKey[ride->type]];
                break;
        }
    } while (!(tileElement++)->IsLastForTile());

    if (colourA != 0)
        return colourA;

    return colourB;
}

static void map_window_set_pixels(rct_window* w)
{
    uint16_t colour = 0;
    int32_t x = 0, y = 0, dx = 0, dy = 0;

    int32_t pos = (_currentLine * (MAP_WINDOW_MAP_SIZE - 1)) + MAXIMUM_MAP_SIZE_TECHNICAL - 1;
    LocationXY16 destinationPosition = MakeXY16(pos % MAP_WINDOW_MAP_SIZE, pos / MAP_WINDOW_MAP_SIZE);
    auto destination = _mapImageData.data() + (destinationPosition.y * MAP_WINDOW_MAP_SIZE) + destinationPosition.x;
    switch (get_current_rotation())
    {
        case 0:
            x = _currentLine * 32;
            y = 0;
            dx = 0;
            dy = 32;
            break;
        case 1:
            x = (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32;
            y = _currentLine * 32;
            dx = -32;
            dy = 0;
            break;
        case 2:
            x = ((MAXIMUM_MAP_SIZE_TECHNICAL - 1) - _currentLine) * 32;
            y = (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32;
            dx = 0;
            dy = -32;
            break;
        case 3:
            x = 0;
            y = ((MAXIMUM_MAP_SIZE_TECHNICAL - 1) - _currentLine) * 32;
            dx = 32;
            dy = 0;
            break;
    }

    for (int32_t i = 0; i < MAXIMUM_MAP_SIZE_TECHNICAL; i++)
    {
        if (x > 0 && y > 0 && x < gMapSizeUnits && y < gMapSizeUnits)
        {
            switch (w->selected_tab)
            {
                case PAGE_PEEPS:
                    colour = map_window_get_pixel_colour_peep({ x, y });
                    break;
                case PAGE_RIDES:
                    colour = map_window_get_pixel_colour_ride({ x, y });
                    break;
            }
            destination[0] = (colour >> 8) & 0xFF;
            destination[1] = colour;
        }
        x += dx;
        y += dy;

        destinationPosition.x++;
        destinationPosition.y++;
        destination = _mapImageData.data() + (destinationPosition.y * MAP_WINDOW_MAP_SIZE) + destinationPosition.x;
    }
    _currentLine++;
    if (_currentLine >= MAXIMUM_MAP_SIZE_TECHNICAL)
        _currentLine = 0;
}

static CoordsXY map_window_screen_to_map(int32_t screenX, int32_t screenY)
{
    screenX = ((screenX + 8) - MAXIMUM_MAP_SIZE_TECHNICAL) / 2;
    screenY = ((screenY + 8)) / 2;
    int32_t x = (screenY - screenX) * 32;
    int32_t y = (screenX + screenY) * 32;

    switch (get_current_rotation())
    {
        case 0:
            return { x, y };
        case 1:
            return { MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - y, x };
        case 2:
            return { MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - x, MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - y };
        case 3:
            return { y, MAXIMUM_MAP_SIZE_TECHNICAL * 32 - 1 - x };
    }

    return { 0, 0 }; // unreachable
}
