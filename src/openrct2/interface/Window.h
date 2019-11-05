/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "../common.h"
#include "../ride/RideTypes.h"
#include "../world/Location.hpp"

#include <functional>
#include <limits>
#include <list>
#include <memory>

struct rct_drawpixelinfo;
struct rct_window;
union rct_window_event;
struct track_design_file_ref;
struct TitleSequence;
struct TextInputSession;
struct scenario_index_entry;

#define SCROLLABLE_ROW_HEIGHT 12
#define LIST_ROW_HEIGHT 12
#define TABLE_CELL_HEIGHT 12
#define BUTTON_FACE_HEIGHT 12

#define TEXT_INPUT_SIZE 1024
#define TOP_TOOLBAR_HEIGHT 27

extern uint16_t TextInputDescriptionArgs[4];
extern char gTextBoxInput[TEXT_INPUT_SIZE];
extern int32_t gMaxTextBoxInputLength;
extern int32_t gTextBoxFrameNo;
extern bool gUsingWidgetTextBox;
extern struct TextInputSession* gTextInput;

using wndproc = void(struct rct_window*, union rct_window_event*);

using rct_windowclass = uint8_t;
using rct_windownumber = uint16_t;
using rct_widgetindex = int16_t;

struct window_identifier
{
    rct_windowclass classification;
    rct_windownumber number;
};

struct widget_identifier
{
    window_identifier window;
    rct_widgetindex widget_index;
};

extern widget_identifier gCurrentTextBox;

/**
 * Widget structure
 * size: 0x10
 */
struct rct_widget
{
    uint8_t type;   // 0x00
    uint8_t colour; // 0x01
    int16_t left;   // 0x02
    int16_t right;  // 0x04
    int16_t top;    // 0x06
    int16_t bottom; // 0x08
    union
    { // 0x0A
        uint32_t image;
        rct_string_id text;
        uint32_t content;
        utf8* string;
    };
    rct_string_id tooltip; // 0x0E
};

/**
 * Viewport structure
 */
struct rct_viewport
{
    int16_t width;       // 0x00
    int16_t height;      // 0x02
    int16_t x;           // 0x04
    int16_t y;           // 0x06
    int16_t view_x;      // 0x08
    int16_t view_y;      // 0x0A
    int16_t view_width;  // 0x0C
    int16_t view_height; // 0x0E
    uint32_t flags;      // 0x12
    uint8_t zoom;        // 0x10
    uint8_t var_11;
    uint8_t visibility; // VISIBILITY_CACHE
};

/**
 * Scroll structure
 * size: 0x12
 */
struct rct_scroll
{
    uint16_t flags;          // 0x00
    uint16_t h_left;         // 0x02
    uint16_t h_right;        // 0x04
    uint16_t h_thumb_left;   // 0x06
    uint16_t h_thumb_right;  // 0x08
    uint16_t v_top;          // 0x0A
    uint16_t v_bottom;       // 0x0C
    uint16_t v_thumb_top;    // 0x0E
    uint16_t v_thumb_bottom; // 0x10
};

constexpr auto WINDOW_SCROLL_UNDEFINED = std::numeric_limits<uint16_t>::max();

/**
 * Viewport focus structure.
 * size: 0xA
 * Use sprite.type to work out type.
 */
struct coordinate_focus
{
    int16_t var_480;
    int16_t x;        // 0x482
    int16_t y;        // 0x484 & VIEWPORT_FOCUS_Y_MASK
    int16_t z;        // 0x486
    uint8_t rotation; // 0x488
    uint8_t zoom;     // 0x489
    int16_t width;
    int16_t height;
};

// Type is viewport_target_sprite_id & 0x80000000 != 0
struct sprite_focus
{
    int16_t var_480;
    uint16_t sprite_id; // 0x482
    uint8_t pad_484;
    uint8_t type; // 0x485 & VIEWPORT_FOCUS_TYPE_MASK
    uint16_t pad_486;
    uint8_t rotation; // 0x488
    uint8_t zoom;     // 0x489
};

#define VIEWPORT_FOCUS_TYPE_MASK 0xC0
enum VIEWPORT_FOCUS_TYPE : uint8_t
{
    VIEWPORT_FOCUS_TYPE_COORDINATE = (1 << 6),
    VIEWPORT_FOCUS_TYPE_SPRITE = (1 << 7)
};
#define VIEWPORT_FOCUS_Y_MASK 0x3FFF

struct viewport_focus
{
    VIEWPORT_FOCUS_TYPE type{};
    union
    {
        sprite_focus sprite;
        coordinate_focus coordinate;
    };
};

struct rct_window_event_list
{
    void (*close)(struct rct_window*);
    void (*mouse_up)(struct rct_window*, rct_widgetindex);
    void (*resize)(struct rct_window*);
    void (*mouse_down)(struct rct_window*, rct_widgetindex, rct_widget*);
    void (*dropdown)(struct rct_window*, rct_widgetindex, int32_t);
    void (*unknown_05)(struct rct_window*);
    void (*update)(struct rct_window*);
    void (*periodic_update)(struct rct_window*);
    void (*unknown_08)(struct rct_window*);
    void (*tool_update)(struct rct_window*, rct_widgetindex, ScreenCoordsXY);
    void (*tool_down)(struct rct_window*, rct_widgetindex, ScreenCoordsXY);
    void (*tool_drag)(struct rct_window*, rct_widgetindex, ScreenCoordsXY);
    void (*tool_up)(struct rct_window*, rct_widgetindex, ScreenCoordsXY);
    void (*tool_abort)(struct rct_window*, rct_widgetindex);
    void (*unknown_0E)(struct rct_window*);
    void (*get_scroll_size)(struct rct_window*, int32_t, int32_t*, int32_t*);
    void (*scroll_mousedown)(struct rct_window*, int32_t, int32_t, int32_t);
    void (*scroll_mousedrag)(struct rct_window*, int32_t, int32_t, int32_t);
    void (*scroll_mouseover)(struct rct_window*, int32_t, int32_t, int32_t);
    void (*text_input)(struct rct_window*, rct_widgetindex, char*);
    void (*viewport_rotate)(struct rct_window*);
    void (*unknown_15)(struct rct_window*, int32_t, int32_t);
    void (*tooltip)(struct rct_window*, rct_widgetindex, rct_string_id*);
    void (*cursor)(struct rct_window*, rct_widgetindex, int32_t, int32_t, int32_t*);
    void (*moved)(struct rct_window*, int32_t, int32_t);
    void (*invalidate)(struct rct_window*);
    void (*paint)(struct rct_window*, rct_drawpixelinfo*);
    void (*scroll_paint)(struct rct_window*, rct_drawpixelinfo*, int32_t);
};

struct campaign_variables
{
    int16_t campaign_type;
    int16_t no_weeks; // 0x482
    uint16_t ride_id; // 0x484
    uint32_t pad_486;
};

struct new_ride_variables
{
    int16_t selected_ride_id;    // 0x480
    int16_t highlighted_ride_id; // 0x482
    uint16_t pad_484;
    uint16_t pad_486;
    uint16_t selected_ride_countdown; // 488
};

struct news_variables
{
    int16_t var_480;
    int16_t var_482;
    uint16_t var_484;
    uint16_t var_486;
    uint16_t var_488;
};

struct map_variables
{
    int16_t rotation;
    int16_t var_482;
    uint16_t var_484;
    uint16_t var_486;
    uint16_t var_488;
};

struct ride_variables
{
    int16_t view;
    int32_t var_482;
    int32_t var_486;
};

struct scenery_variables
{
    uint16_t selected_scenery_id;
    int16_t hover_counter;
};

struct track_list_variables
{
    bool track_list_being_updated;
    bool reload_track_designs;
};

struct error_variables
{
    uint16_t var_480;
};

struct rct_window;

#define RCT_WINDOW_RIGHT(w) ((w)->x + (w)->width)
#define RCT_WINDOW_BOTTOM(w) ((w)->y + (w)->height)

enum WINDOW_EVENTS
{
    WE_CLOSE = 0,
    WE_MOUSE_UP = 1,
    WE_RESIZE = 2,
    WE_MOUSE_DOWN = 3,
    WE_DROPDOWN = 4,
    WE_UNKNOWN_05 = 5,
    // Unknown 05: Used to update tabs that are not being animated
    // see window_peep. When the overview tab is not highlighted the
    // items being carried such as hats/balloons still need to be shown
    // and removed. Probably called after anything that affects items
    // being carried.
    WE_UPDATE = 6,
    WE_UNKNOWN_07 = 7,
    WE_UNKNOWN_08 = 8,
    WE_TOOL_UPDATE = 9,
    WE_TOOL_DOWN = 10,
    WE_TOOL_DRAG = 11,
    WE_TOOL_UP = 12,
    WE_TOOL_ABORT = 13,
    WE_UNKNOWN_0E = 14,
    WE_SCROLL_GETSIZE = 15,
    WE_SCROLL_MOUSEDOWN = 16,
    WE_SCROLL_MOUSEDRAG = 17,
    WE_SCROLL_MOUSEOVER = 18,
    WE_TEXT_INPUT = 19,
    WE_VIEWPORT_ROTATE = 20,
    WE_UNKNOWN_15 = 21, // scroll mouse move?
    WE_TOOLTIP = 22,
    WE_CURSOR = 23,
    WE_MOVED = 24,
    WE_INVALIDATE = 25,
    WE_PAINT = 26,
    WE_SCROLL_PAINT = 27,
};

enum WINDOW_FLAGS
{
    /*
    WF_TIMEOUT_SHL = 0,
    WF_TIMEOUT_MASK = 7,
    WF_DRAGGING = 1 << 3,
    WF_SCROLLER_UP = 1 << 4,
    WF_SCROLLER_DOWN = 1 << 5,
    WF_SCROLLER_MIDDLE = 1 << 6,
    WF_DISABLE_VP_SCROLL = 1 << 9,
    */

    WF_STICK_TO_BACK = (1 << 0),
    WF_STICK_TO_FRONT = (1 << 1),
    WF_NO_SCROLLING = (1 << 2), // User is unable to scroll this viewport
    WF_SCROLLING_TO_LOCATION = (1 << 3),
    WF_TRANSPARENT = (1 << 4),
    WF_NO_BACKGROUND = (1 << 5), // Instead of half transparency, completely remove the window background
    WF_7 = (1 << 7),
    WF_RESIZABLE = (1 << 8),
    WF_NO_AUTO_CLOSE = (1 << 9), // Don't auto close this window if too many windows are open
    WF_10 = (1 << 10),
    WF_WHITE_BORDER_ONE = (1 << 12),
    WF_WHITE_BORDER_MASK = (1 << 12) | (1 << 13),

    WF_NO_SNAPPING = (1 << 15)
};

enum SCROLL_FLAGS
{
    HSCROLLBAR_VISIBLE = (1 << 0),
    HSCROLLBAR_THUMB_PRESSED = (1 << 1),
    HSCROLLBAR_LEFT_PRESSED = (1 << 2),
    HSCROLLBAR_RIGHT_PRESSED = (1 << 3),
    VSCROLLBAR_VISIBLE = (1 << 4),
    VSCROLLBAR_THUMB_PRESSED = (1 << 5),
    VSCROLLBAR_UP_PRESSED = (1 << 6),
    VSCROLLBAR_DOWN_PRESSED = (1 << 7),
};

#define SCROLLBAR_SIZE 16

enum
{
    SCROLL_PART_NONE = -1,
    SCROLL_PART_VIEW = 0,
    SCROLL_PART_HSCROLLBAR_LEFT = 1,
    SCROLL_PART_HSCROLLBAR_RIGHT = 2,
    SCROLL_PART_HSCROLLBAR_LEFT_TROUGH = 3,
    SCROLL_PART_HSCROLLBAR_RIGHT_TROUGH = 4,
    SCROLL_PART_HSCROLLBAR_THUMB = 5,
    SCROLL_PART_VSCROLLBAR_TOP = 6,
    SCROLL_PART_VSCROLLBAR_BOTTOM = 7,
    SCROLL_PART_VSCROLLBAR_TOP_TROUGH = 8,
    SCROLL_PART_VSCROLLBAR_BOTTOM_TROUGH = 9,
    SCROLL_PART_VSCROLLBAR_THUMB = 10,
};

enum
{
    WC_MAIN_WINDOW = 0,
    WC_TOP_TOOLBAR = 1,
    WC_BOTTOM_TOOLBAR = 2,
    WC_TOOLTIP = 5,
    WC_DROPDOWN = 6,
    WC_ABOUT = 8,
    WC_PUBLISHER_CREDITS = 9,
    WC_MUSIC_CREDITS = 10,
    WC_ERROR = 11,
    WC_RIDE = 12,
    WC_RIDE_CONSTRUCTION = 13,
    WC_SAVE_PROMPT = 14,
    WC_RIDE_LIST = 15,
    WC_CONSTRUCT_RIDE = 16,
    WC_DEMOLISH_RIDE_PROMPT = 17,
    WC_SCENERY = 18,
    WC_OPTIONS = 19,
    WC_FOOTPATH = 20,
    WC_LAND = 21,
    WC_WATER = 22,
    WC_PEEP = 23,
    WC_GUEST_LIST = 24,
    WC_STAFF_LIST = 25,
    WC_FIRE_PROMPT = 26,
    WC_PARK_INFORMATION = 27,
    WC_FINANCES = 28,
    WC_TITLE_MENU = 29,
    WC_TITLE_EXIT = 30,
    WC_RECENT_NEWS = 31,
    WC_SCENARIO_SELECT = 32,
    WC_TRACK_DESIGN_LIST = 33,
    WC_TRACK_DESIGN_PLACE = 34,
    WC_NEW_CAMPAIGN = 35,
    WC_KEYBOARD_SHORTCUT_LIST = 36,
    WC_CHANGE_KEYBOARD_SHORTCUT = 37,
    WC_MAP = 38,
    WC_TITLE_LOGO = 39,
    WC_BANNER = 40,
    WC_MAP_TOOLTIP = 41,
    WC_EDITOR_OBJECT_SELECTION = 42,
    WC_EDITOR_INVENTION_LIST = 43,
    WC_EDITOR_INVENTION_LIST_DRAG = 44,
    WC_EDITOR_SCENARIO_OPTIONS = 45,
    WC_EDTIOR_OBJECTIVE_OPTIONS = 46,
    WC_MANAGE_TRACK_DESIGN = 47,
    WC_TRACK_DELETE_PROMPT = 48,
    WC_INSTALL_TRACK = 49,
    WC_CLEAR_SCENERY = 50,
    WC_NOTIFICATION_OPTIONS = 109,
    WC_CHEATS = 110,
    WC_RESEARCH = 111,
    WC_VIEWPORT = 112,
    WC_TEXTINPUT = 113,
    WC_MAPGEN = 114,
    WC_LOADSAVE = 115,
    WC_LOADSAVE_OVERWRITE_PROMPT = 116,
    WC_TITLE_OPTIONS = 117,
    WC_LAND_RIGHTS = 118,
    WC_THEMES = 119,
    WC_TILE_INSPECTOR = 120,
    WC_CHANGELOG = 121,
    WC_TITLE_EDITOR = 122,
    WC_TITLE_COMMAND_EDITOR = 123,
    WC_MULTIPLAYER = 124,
    WC_PLAYER = 125,
    WC_NETWORK_STATUS = 126,
    WC_SERVER_LIST = 127,
    WC_SERVER_START = 128,
    WC_CUSTOM_CURRENCY_CONFIG = 129,
    WC_DEBUG_PAINT = 130,
    WC_VIEW_CLIPPING = 131,
    WC_OBJECT_LOAD_ERROR = 132,
    WC_NETWORK = 133,

    // Only used for colour schemes
    WC_STAFF = 220,
    WC_EDITOR_TRACK_BOTTOM_TOOLBAR = 221,
    WC_EDITOR_SCENARIO_BOTTOM_TOOLBAR = 222,
    WC_CHAT = 223,
    WC_CONSOLE = 224,

    WC_NULL = 255,
};

enum
{
    WV_PARK_AWARDS,
    WV_PARK_RATING,
    WV_PARK_OBJECTIVE,
    WV_PARK_GUESTS,
    WV_FINANCES_RESEARCH,
    WV_RIDE_RESEARCH,
    WV_MAZE_CONSTRUCTION,
    WV_NETWORK_PASSWORD,
    WV_EDITOR_BOTTOM_TOOLBAR,
    WV_EDITOR_MAIN,
};

enum
{
    WD_BANNER,
    WD_NEW_CAMPAIGN,
    WD_DEMOLISH_RIDE,
    WD_REFURBISH_RIDE,
    WD_SIGN,
    WD_SIGN_SMALL,

    WD_PLAYER,

    WD_VEHICLE,
    WD_TRACK,
};

#define validate_global_widx(wc, widx)                                                                                         \
    static_assert(widx == wc##__##widx, "Global WIDX of " #widx " doesn't match actual value.")

#define WC_MAIN_WINDOW__0 0
#define WC_TOP_TOOLBAR__WIDX_PAUSE 0
#define WC_TOP_TOOLBAR__WIDX_LAND 8
#define WC_TOP_TOOLBAR__WIDX_WATER 9
#define WC_TOP_TOOLBAR__WIDX_SCENERY 10
#define WC_TOP_TOOLBAR__WIDX_PATH 11
#define WC_TOP_TOOLBAR__WIDX_CLEAR_SCENERY 17
#define WC_RIDE_CONSTRUCTION__WIDX_CONSTRUCT 23
#define WC_RIDE_CONSTRUCTION__WIDX_ENTRANCE 29
#define WC_RIDE_CONSTRUCTION__WIDX_EXIT 30
#define WC_RIDE_CONSTRUCTION__WIDX_ROTATE 32
#define WC_SCENERY__WIDX_SCENERY_TAB_1 4
#define WC_SCENERY__WIDX_SCENERY_ROTATE_OBJECTS_BUTTON 25
#define WC_SCENERY__WIDX_SCENERY_EYEDROPPER_BUTTON 30
#define WC_PEEP__WIDX_PATROL 11
#define WC_PEEP__WIDX_ACTION_LBL 13
#define WC_PEEP__WIDX_PICKUP 14
#define WC_TRACK_DESIGN_LIST__WIDX_ROTATE 8
#define WC_TRACK_DESIGN_PLACE__WIDX_ROTATE 3
#define WC_MAP__WIDX_ROTATE_90 20
#define WC_EDITOR_OBJECT_SELECTION__WIDX_TAB_1 21
#define WC_STAFF__WIDX_PICKUP 10

enum PROMPT_MODE
{
    PM_SAVE_BEFORE_LOAD = 0,
    PM_SAVE_BEFORE_QUIT,
    PM_SAVE_BEFORE_QUIT2,
    PM_QUIT
};

enum BTM_TOOLBAR_DIRTY_FLAGS
{
    BTM_TB_DIRTY_FLAG_MONEY = (1 << 0),
    BTM_TB_DIRTY_FLAG_DATE = (1 << 1),
    BTM_TB_DIRTY_FLAG_PEEP_COUNT = (1 << 2),
    BTM_TB_DIRTY_FLAG_CLIMATE = (1 << 3),
    BTM_TB_DIRTY_FLAG_PARK_RATING = (1 << 4)
};

// 000N_TTTL
enum
{
    LOADSAVETYPE_LOAD = 0 << 0,
    LOADSAVETYPE_SAVE = 1 << 0,

    LOADSAVETYPE_GAME = 0 << 1,
    LOADSAVETYPE_LANDSCAPE = 1 << 1,
    LOADSAVETYPE_SCENARIO = 2 << 1,
    LOADSAVETYPE_TRACK = 3 << 1,
    LOADSAVETYPE_HEIGHTMAP = 4 << 1,
};

enum
{
    MODAL_RESULT_FAIL = -1,
    MODAL_RESULT_CANCEL,
    MODAL_RESULT_OK
};

enum VISIBILITY_CACHE
{
    VC_UNKNOWN,
    VC_VISIBLE,
    VC_COVERED
};

enum GUEST_LIST_FILTER_TYPE
{
    GLFT_GUESTS_ON_RIDE,
    GLFT_GUESTS_IN_QUEUE,
    GLFT_GUESTS_THINKING_ABOUT_RIDE,
    GLFT_GUESTS_THINKING_X,
};

enum TOOL_IDX
{
    TOOL_ARROW = 0,
    TOOL_UP_ARROW = 2,
    TOOL_UP_DOWN_ARROW = 3,
    TOOL_PICKER = 7,
    TOOL_CROSSHAIR = 12,
    TOOL_PATH_DOWN = 17,
    TOOL_DIG_DOWN = 18,
    TOOL_WATER_DOWN = 19,
    TOOL_WALK_DOWN = 22,
    TOOL_PAINT_DOWN = 23,
    TOOL_ENTRANCE_DOWN = 24,
};

using modal_callback = void (*)(int32_t result);
using close_callback = void (*)();

#define WINDOW_LIMIT_MIN 4
#define WINDOW_LIMIT_MAX 64
#define WINDOW_LIMIT_RESERVED 4 // Used to reserve room for the main viewport, toolbars, etc.

extern rct_window* gWindowAudioExclusive;

extern uint16_t gWindowUpdateTicks;
extern uint16_t gWindowMapFlashingFlags;

extern colour_t gCurrentWindowColours[4];

extern bool gDisableErrorWindowSound;

std::list<std::shared_ptr<rct_window>>::iterator window_get_iterator(const rct_window* w);
void window_visit_each(std::function<void(rct_window*)> func);

void window_dispatch_update_all();
void window_update_all_viewports();
void window_update_all();

void window_set_window_limit(int32_t value);

rct_window* window_create(
    ScreenCoordsXY screenCoords, int32_t width, int32_t height, rct_window_event_list* event_handlers, rct_windowclass cls,
    uint16_t flags);
rct_window* window_create_auto_pos(
    int32_t width, int32_t height, rct_window_event_list* event_handlers, rct_windowclass cls, uint16_t flags);
rct_window* window_create_centred(
    int32_t width, int32_t height, rct_window_event_list* event_handlers, rct_windowclass cls, uint16_t flags);
void window_close(rct_window* window);
void window_close_by_class(rct_windowclass cls);
void window_close_by_number(rct_windowclass cls, rct_windownumber number);
void window_close_top();
void window_close_all();
void window_close_all_except_class(rct_windowclass cls);
void window_close_all_except_flags(uint16_t flags);
rct_window* window_find_by_class(rct_windowclass cls);
rct_window* window_find_by_number(rct_windowclass cls, rct_windownumber number);
rct_window* window_find_from_point(ScreenCoordsXY screenCoords);
rct_widgetindex window_find_widget_from_point(rct_window* w, ScreenCoordsXY screenCoords);
void window_invalidate_by_class(rct_windowclass cls);
void window_invalidate_by_number(rct_windowclass cls, rct_windownumber number);
void window_invalidate_all();
void widget_invalidate(rct_window* w, rct_widgetindex widgetIndex);
void widget_invalidate_by_class(rct_windowclass cls, rct_widgetindex widgetIndex);
void widget_invalidate_by_number(rct_windowclass cls, rct_windownumber number, rct_widgetindex widgetIndex);
void window_init_scroll_widgets(rct_window* w);
void window_update_scroll_widgets(rct_window* w);
int32_t window_get_scroll_data_index(rct_window* w, rct_widgetindex widget_index);

rct_window* window_bring_to_front(rct_window* w);
rct_window* window_bring_to_front_by_class(rct_windowclass cls);
rct_window* window_bring_to_front_by_class_with_flags(rct_windowclass cls, uint16_t flags);
rct_window* window_bring_to_front_by_number(rct_windowclass cls, rct_windownumber number);

void window_push_others_right(rct_window* w);
void window_push_others_below(rct_window* w1);

rct_window* window_get_main();

void window_scroll_to_location(rct_window* w, int32_t x, int32_t y, int32_t z);
void window_rotate_camera(rct_window* w, int32_t direction);
void window_viewport_get_map_coords_by_cursor(
    rct_window* w, int16_t* map_x, int16_t* map_y, int16_t* offset_x, int16_t* offset_y);
void window_viewport_centre_tile_around_cursor(rct_window* w, int16_t map_x, int16_t map_y, int16_t offset_x, int16_t offset_y);
void window_zoom_set(rct_window* w, int32_t zoomLevel, bool atCursor);
void window_zoom_in(rct_window* w, bool atCursor);
void window_zoom_out(rct_window* w, bool atCursor);
void main_window_zoom(bool zoomIn, bool atCursor);

void window_show_textinput(rct_window* w, rct_widgetindex widgetIndex, uint16_t title, uint16_t text, int32_t value);

void window_draw_all(rct_drawpixelinfo* dpi, int16_t left, int16_t top, int16_t right, int16_t bottom);
void window_draw(rct_drawpixelinfo* dpi, rct_window* w, int32_t left, int32_t top, int32_t right, int32_t bottom);
void window_draw_widgets(rct_window* w, rct_drawpixelinfo* dpi);
void window_draw_viewport(rct_drawpixelinfo* dpi, rct_window* w);

void window_set_position(rct_window* w, ScreenCoordsXY screenCoords);
void window_move_position(rct_window* w, ScreenCoordsXY screenCoords);
void window_resize(rct_window* w, int32_t dw, int32_t dh);
void window_set_resize(rct_window* w, int32_t minWidth, int32_t minHeight, int32_t maxWidth, int32_t maxHeight);

bool tool_set(rct_window* w, rct_widgetindex widgetIndex, TOOL_IDX tool);
void tool_cancel();

void window_close_construction_windows();

void window_update_viewport_ride_music();

rct_viewport* window_get_viewport(rct_window* window);

// Open window functions
void window_relocate_windows(int32_t width, int32_t height);
void window_resize_gui(int32_t width, int32_t height);
void window_resize_gui_scenario_editor(int32_t width, int32_t height);
void window_ride_construct(rct_window* w);
void ride_construction_toolupdate_entrance_exit(ScreenCoordsXY screenCoords);
void ride_construction_toolupdate_construct(ScreenCoordsXY screenCoords);
void ride_construction_tooldown_construct(ScreenCoordsXY screenCoords);

void window_bubble_list_item(rct_window* w, int32_t item_position);

void window_align_tabs(rct_window* w, rct_widgetindex start_tab_id, rct_widgetindex end_tab_id);

void window_staff_list_init_vars();

void window_event_close_call(rct_window* w);
void window_event_mouse_up_call(rct_window* w, rct_widgetindex widgetIndex);
void window_event_resize_call(rct_window* w);
void window_event_mouse_down_call(rct_window* w, rct_widgetindex widgetIndex);
void window_event_dropdown_call(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex);
void window_event_unknown_05_call(rct_window* w);
void window_event_update_call(rct_window* w);
void window_event_periodic_update_call(rct_window* w);
void window_event_unknown_08_call(rct_window* w);
void window_event_tool_update_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
void window_event_tool_down_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
void window_event_tool_drag_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
void window_event_tool_up_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
void window_event_tool_abort_call(rct_window* w, rct_widgetindex widgetIndex);
void window_event_unknown_0E_call(rct_window* w);
void window_get_scroll_size(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height);
void window_event_scroll_mousedown_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords);
void window_event_scroll_mousedrag_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords);
void window_event_scroll_mouseover_call(rct_window* w, int32_t scrollIndex, ScreenCoordsXY screenCoords);
void window_event_textinput_call(rct_window* w, rct_widgetindex widgetIndex, char* text);
void window_event_viewport_rotate_call(rct_window* w);
void window_event_unknown_15_call(rct_window* w, int32_t scrollIndex, int32_t scrollAreaType);
rct_string_id window_event_tooltip_call(rct_window* w, rct_widgetindex widgetIndex);
int32_t window_event_cursor_call(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
void window_event_moved_call(rct_window* w, ScreenCoordsXY screenCoords);
void window_event_invalidate_call(rct_window* w);
void window_event_paint_call(rct_window* w, rct_drawpixelinfo* dpi);
void window_event_scroll_paint_call(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex);

void invalidate_all_windows_after_input();
void textinput_cancel();

void window_move_and_snap(rct_window* w, ScreenCoordsXY newWindowCoords, int32_t snapProximity);
int32_t window_can_resize(rct_window* w);

void window_start_textbox(
    rct_window* call_w, rct_widgetindex call_widget, rct_string_id existing_text, char* existing_args, int32_t maxLength);
void window_cancel_textbox();
void window_update_textbox_caret();
void window_update_textbox();

bool window_is_visible(rct_window* w);

bool scenery_tool_is_active();

rct_viewport* window_get_previous_viewport(rct_viewport* current);
void window_reset_visibilities();
void window_init_all();

// Cheat: in-game land ownership editor
void toggle_ingame_land_ownership_editor();

void window_ride_construction_keyboard_shortcut_turn_left();
void window_ride_construction_keyboard_shortcut_turn_right();
void window_ride_construction_keyboard_shortcut_use_track_default();
void window_ride_construction_keyboard_shortcut_slope_down();
void window_ride_construction_keyboard_shortcut_slope_up();
void window_ride_construction_keyboard_shortcut_chain_lift_toggle();
void window_ride_construction_keyboard_shortcut_bank_left();
void window_ride_construction_keyboard_shortcut_bank_right();
void window_ride_construction_keyboard_shortcut_previous_track();
void window_ride_construction_keyboard_shortcut_next_track();
void window_ride_construction_keyboard_shortcut_build_current();
void window_ride_construction_keyboard_shortcut_demolish_current();

void window_follow_sprite(rct_window* w, size_t spriteIndex);
void window_unfollow_sprite(rct_window* w);

bool window_ride_construction_update_state(
    int32_t* trackType, int32_t* trackDirection, ride_id_t* rideIndex, int32_t* _liftHillAndAlternativeState, int32_t* x,
    int32_t* y, int32_t* z, int32_t* properties);
money32 place_provisional_track_piece(
    ride_id_t rideIndex, int32_t trackType, int32_t trackDirection, int32_t liftHillAndAlternativeState, int32_t x, int32_t y,
    int32_t z);

extern uint64_t _enabledRidePieces;
extern uint8_t _rideConstructionState2;
extern bool _stationConstructed;
extern bool _deferClose;

rct_window* window_get_listening();
rct_windowclass window_get_classification(rct_window* window);

#endif
