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
#include <openrct2-ui/interface/Dropdown.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/actions/TileModifyAction.hpp>
#include <openrct2/common.h>
#include <openrct2/core/Guard.hpp>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/localisation/StringIds.h>
#include <openrct2/ride/RideData.h>
#include <openrct2/ride/Track.h>
#include <openrct2/sprites.h>
#include <openrct2/windows/tile_inspector.h>
#include <openrct2/world/Banner.h>
#include <openrct2/world/Footpath.h>
#include <openrct2/world/LargeScenery.h>
#include <openrct2/world/Park.h>
#include <openrct2/world/Scenery.h>
#include <openrct2/world/SmallScenery.h>
#include <openrct2/world/Surface.h>

// clang-format off
static constexpr const rct_string_id TerrainTypeStringIds[] = {
    STR_TILE_INSPECTOR_TERRAIN_GRASS,
    STR_TILE_INSPECTOR_TERRAIN_SAND,
    STR_TILE_INSPECTOR_TERRAIN_DIRT,
    STR_TILE_INSPECTOR_TERRAIN_ROCK,
    STR_TILE_INSPECTOR_TERRAIN_MARTIAN,
    STR_TILE_INSPECTOR_TERRAIN_CHECKERBOARD,
    STR_TILE_INSPECTOR_TERRAIN_GRASS_CLUMPS,
    STR_TILE_INSPECTOR_TERRAIN_ICE,
    STR_TILE_INSPECTOR_TERRAIN_GRID_RED,
    STR_TILE_INSPECTOR_TERRAIN_GRID_YELLOW,
    STR_TILE_INSPECTOR_TERRAIN_GRID_BLUE,
    STR_TILE_INSPECTOR_TERRAIN_GRID_GREEN,
    STR_TILE_INSPECTOR_TERRAIN_SAND_DARK,
    STR_TILE_INSPECTOR_TERRAIN_SAND_LIGHT,
    STR_TILE_INSPECTOR_TERRAIN_CHECKERBOARD_INVERTED,
    STR_TILE_INSPECTOR_TERRAIN_UNDERGROUND_VIEW,
};

static constexpr const rct_string_id TerrainEdgeTypeStringIds[] = {
    STR_TILE_INSPECTOR_TERRAIN_EDGE_ROCK,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_WOOD_RED,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_WOOD_BLACK,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_ICE,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_BRICK,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_IRON,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_GREY,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_YELLOW,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_RED,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_PURPLE,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_GREEN,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_STONE_BROWN,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_STONE_GREY,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_SKYSCRAPER_A,
    STR_TILE_INSPECTOR_TERRAIN_EDGE_SKYSCRAPER_B,
};

static constexpr const rct_string_id EntranceTypeStringIds[] = {
    STR_TILE_INSPECTOR_ENTRANCE_TYPE_RIDE_ENTRANCE,
    STR_TILE_INSPECTOR_ENTRANCE_TYPE_RIDE_EXIT,
    STR_TILE_INSPECTOR_ENTRANCE_TYPE_PARK_ENTRANC,
};

static constexpr const rct_string_id ParkEntrancePartStringIds[] = {
    STR_TILE_INSPECTOR_ENTRANCE_MIDDLE,
    STR_TILE_INSPECTOR_ENTRANCE_LEFT,
    STR_TILE_INSPECTOR_ENTRANCE_RIGHT
};

static constexpr const rct_string_id WallSlopeStringIds[] = {
    STR_TILE_INSPECTOR_WALL_FLAT,
    STR_TILE_INSPECTOR_WALL_SLOPED_LEFT,
    STR_TILE_INSPECTOR_WALL_SLOPED_RIGHT
};

enum WINDOW_TILE_INSPECTOR_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_LIST,
    WIDX_SPINNER_X,
    WIDX_SPINNER_X_INCREASE,
    WIDX_SPINNER_X_DECREASE,
    WIDX_SPINNER_Y,
    WIDX_SPINNER_Y_INCREASE,
    WIDX_SPINNER_Y_DECREASE,
    WIDX_BUTTON_CORRUPT,
    WIDX_BUTTON_REMOVE,
    WIDX_BUTTON_MOVE_UP,
    WIDX_BUTTON_MOVE_DOWN,
    WIDX_BUTTON_ROTATE,
    WIDX_BUTTON_SORT,
    WIDX_BUTTON_COPY,
    WIDX_BUTTON_PASTE,
    WIDX_COLUMN_TYPE,
    WIDX_COLUMN_BASEHEIGHT,
    WIDX_COLUMN_CLEARANCEHEIGHT,
    WIDX_COLUMN_GHOSTFLAG,
    WIDX_COLUMN_LASTFLAG,
    WIDX_GROUPBOX_DETAILS,
    WIDX_GROUPBOX_PROPERTIES,

    PAGE_WIDGETS,

    // Surface
    WIDX_SURFACE_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_SURFACE_SPINNER_HEIGHT_INCREASE,
    WIDX_SURFACE_SPINNER_HEIGHT_DECREASE,
    WIDX_SURFACE_BUTTON_REMOVE_FENCES,
    WIDX_SURFACE_BUTTON_RESTORE_FENCES,
    WIDX_SURFACE_CHECK_CORNER_N,
    WIDX_SURFACE_CHECK_CORNER_E,
    WIDX_SURFACE_CHECK_CORNER_S,
    WIDX_SURFACE_CHECK_CORNER_W,
    WIDX_SURFACE_CHECK_DIAGONAL,

    // Path
    WIDX_PATH_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_PATH_SPINNER_HEIGHT_INCREASE,
    WIDX_PATH_SPINNER_HEIGHT_DECREASE,
    WIDX_PATH_CHECK_BROKEN,
    WIDX_PATH_CHECK_SLOPED,
    WIDX_PATH_CHECK_EDGE_NE, // Note: This is NOT named after the world orientation, but after the way
    WIDX_PATH_CHECK_EDGE_E,  // it looks in the window (top corner is north). Their order is important,
    WIDX_PATH_CHECK_EDGE_SE, // as this is the same order paths use for their corners / edges.
    WIDX_PATH_CHECK_EDGE_S,  //           N
    WIDX_PATH_CHECK_EDGE_SW, //      NW-------NE
    WIDX_PATH_CHECK_EDGE_W,  //   W ------------- E
    WIDX_PATH_CHECK_EDGE_NW, //      SW-------SE
    WIDX_PATH_CHECK_EDGE_N,  //           S

    // Track
    WIDX_TRACK_CHECK_APPLY_TO_ALL = PAGE_WIDGETS,
    WIDX_TRACK_SPINNER_HEIGHT,
    WIDX_TRACK_SPINNER_HEIGHT_INCREASE,
    WIDX_TRACK_SPINNER_HEIGHT_DECREASE,
    WIDX_TRACK_CHECK_CHAIN_LIFT,
    WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED,
    WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE,

    // Scenery
    WIDX_SCENERY_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_SCENERY_SPINNER_HEIGHT_INCREASE,
    WIDX_SCENERY_SPINNER_HEIGHT_DECREASE,
    WIDX_SCENERY_CHECK_QUARTER_N,
    WIDX_SCENERY_CHECK_QUARTER_E,
    WIDX_SCENERY_CHECK_QUARTER_S,
    WIDX_SCENERY_CHECK_QUARTER_W,
    WIDX_SCENERY_CHECK_COLLISION_N,
    WIDX_SCENERY_CHECK_COLLISION_E,
    WIDX_SCENERY_CHECK_COLLISION_S,
    WIDX_SCENERY_CHECK_COLLISION_W,

    // Entrance
    WIDX_ENTRANCE_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE,
    WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE,
    WIDX_ENTRANCE_BUTTON_MAKE_USABLE,

    // Wall
    WIDX_WALL_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_WALL_SPINNER_HEIGHT_INCREASE,
    WIDX_WALL_SPINNER_HEIGHT_DECREASE,
    WIDX_WALL_DROPDOWN_SLOPE,
    WIDX_WALL_DROPDOWN_SLOPE_BUTTON,

    // Large
    WIDX_LARGE_SCENERY_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE,
    WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE,

    // Banner
    WIDX_BANNER_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_BANNER_SPINNER_HEIGHT_INCREASE,
    WIDX_BANNER_SPINNER_HEIGHT_DECREASE,
    WIDX_BANNER_CHECK_BLOCK_NE,
    WIDX_BANNER_CHECK_BLOCK_SE,
    WIDX_BANNER_CHECK_BLOCK_SW,
    WIDX_BANNER_CHECK_BLOCK_NW,

    // Corrupt
    WIDX_CORRUPT_SPINNER_HEIGHT = PAGE_WIDGETS,
    WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE,
    WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE,
    WIDX_CORRUPT_BUTTON_CLAMP,
};

// Window sizes
constexpr int32_t WW = 400;
constexpr int32_t WH = 170;
constexpr int32_t MIN_WW = WW;
constexpr int32_t MAX_WW = WW;
constexpr int32_t MIN_WH = 130;
constexpr int32_t MAX_WH = 800;

// Button space for top buttons
constexpr int32_t BW = WW - 5;     // Button's right side
constexpr int32_t BX = BW - 23;    // Button's left side
constexpr int32_t BY = 17;         // Button's Top
constexpr int32_t BH = BY + 23;    // Button's Bottom
constexpr int32_t BS = 24;

// Column offsets for the table headers
constexpr int32_t COL_X_TYPE = 3;                // Type
constexpr int32_t COL_X_BH = COL_X_TYPE + 312;   // Base height
constexpr int32_t COL_X_CH = COL_X_BH + 20;      // Clearance height
constexpr int32_t COL_X_GF = COL_X_CH + 20;      // Ghost flag
constexpr int32_t COL_X_LF = COL_X_GF + 12;      // Last for tile flag

constexpr int32_t PADDING_BOTTOM = 15;
constexpr int32_t GROUPBOX_PADDING = 6;
constexpr int32_t HORIZONTAL_GROUPBOX_PADDING = 5;
constexpr int32_t VERTICAL_GROUPBOX_PADDING = 4;
constexpr int32_t BUTTONW = 130;
constexpr int32_t BUTTONH = 17;

// Calculates the .left, .right, .top and .bottom for buttons in a group box.
// Buttons are used as reference points for all other widgets in the group boxes.
#define GBBL(col)               (12 + col * (BUTTONW + HORIZONTAL_GROUPBOX_PADDING))
#define GBBR(col)               (GBBL(col) + BUTTONW)
#define GBBT(GROUPTOP, row)     ((GROUPTOP) + 14 + row * (BUTTONH + VERTICAL_GROUPBOX_PADDING))
#define GBBB(GROUPTOP, row)     (GBBT((GROUPTOP), row) + BUTTONH)
#define GBB(GROUPTOP, col, row) GBBL(col), GBBR(col), GBBT((GROUPTOP), row), GBBB((GROUPTOP), row)
#define GBBF(GROUPTOP, col, row)GBBL(col), WW - 10, GBBT((GROUPTOP), row), GBBB((GROUPTOP), row) // Full width

// Dropdowns use the buttons as a reference
#define GBD(GBT, col, row)  GBBL(col), GBBR(col), GBBT(GBT, row) + 3, GBBB(GBT, row) - 3 // Group box dropdown field
#define GBDB(GBT, col, row) GBBR(col) - 11, GBBR(col) - 1, GBBT(GBT, row) + 4, GBBB(GBT, row) - 4 // Group box dropdown button

// Checkbox - given topleft corner
#define CHK(x, y) x, x + 13, y + 2, y + 15

#define MAIN_TILE_INSPECTOR_WIDGETS \
    { WWT_FRAME,        0,  0,              WW - 1,             0,              WH - 1,     0xFFFFFFFF,                 STR_NONE },                 /* panel / background */    \
    { WWT_CAPTION,      0,  1,              WW - 2,             1,              14,         STR_TILE_INSPECTOR_TITLE,   STR_WINDOW_TITLE_TIP },     /* title bar */             \
    { WWT_CLOSEBOX,     0,  WW - 13,        WW - 3,             2,              13,         STR_CLOSE_X,                STR_CLOSE_WINDOW_TIP },     /* close x button */        \
    { WWT_SCROLL,       1,  3,              WW - 4,             57,             WH - PADDING_BOTTOM,    2,              STR_NONE },                 /* Element list */          \
      SPINNER_WIDGETS  (1,  20,             70,                 23,             34,         STR_NONE,                   STR_NONE),                  /* Spinner X (3 widgets) */ \
      SPINNER_WIDGETS  (1,  90,             140,                23,             34,         STR_NONE,                   STR_NONE),                  /* Spinner Y (3 widgets) */ \
    /* Buttons */                                                                                                                                                               \
    { WWT_FLATBTN,      1,  BX,             BW,                 BY,             BH,         SPR_MAP,          STR_INSERT_CORRUPT_TIP },             /* Insert corrupt button */ \
    { WWT_FLATBTN,      1,  BX - BS * 1,    BW - BS * 1,        BY,             BH,         SPR_DEMOLISH,     STR_REMOVE_SELECTED_ELEMENT_TIP },    /* Remove button */         \
    { WWT_BUTTON,       1,  BX - BS * 2,    BW - BS * 2,        BY,             BY + 11,    STR_UP,           STR_MOVE_SELECTED_ELEMENT_UP_TIP },   /* Move up */               \
    { WWT_BUTTON,       1,  BX - BS * 2,    BW - BS * 2,        BH - 11,        BH,         STR_DOWN,         STR_MOVE_SELECTED_ELEMENT_DOWN_TIP }, /* Move down */             \
    { WWT_FLATBTN,      1,  BX - BS * 3,    BW - BS * 3,        BY,             BH,         SPR_ROTATE_ARROW, STR_ROTATE_SELECTED_ELEMENT_TIP },    /* Rotate button */         \
    { WWT_FLATBTN,      1,  BX - BS * 4,    BW - BS * 4,        BY,             BH,         SPR_G2_SORT,      STR_TILE_INSPECTOR_SORT_TIP },        /* Sort button */           \
    { WWT_FLATBTN,      1,  BX - BS * 5,    BW - BS * 5,        BY,             BH,         SPR_G2_COPY,      STR_TILE_INSPECTOR_COPY_TIP },        /* Copy button */           \
    { WWT_FLATBTN,      1,  BX - BS * 6,    BW - BS * 6,        BY,             BH,         SPR_G2_PASTE,     STR_TILE_INSPECTOR_PASTE_TIP },       /* Paste button */          \
    /* Column headers */                                                                                                                                                        \
    { WWT_TABLE_HEADER,     1, COL_X_TYPE,  COL_X_BH - 1,   42,     42 + 13,    STR_NONE,   STR_NONE },                                             /* Type */                  \
    { WWT_TABLE_HEADER,     1, COL_X_BH,    COL_X_CH - 1,   42,     42 + 13,    STR_NONE,   STR_TILE_INSPECTOR_BASE_HEIGHT },                       /* Base height */           \
    { WWT_TABLE_HEADER,     1, COL_X_CH,    COL_X_GF - 1,   42,     42 + 13,    STR_NONE,   STR_TILE_INSPECTOR_CLEARANCE_HEIGHT },                  /* Clearance height */      \
    { WWT_TABLE_HEADER,     1, COL_X_GF,    COL_X_LF - 1,   42,     42 + 13,    STR_NONE,   STR_TILE_INSPECTOR_FLAG_GHOST },                        /* Ghost flag */         \
    { WWT_TABLE_HEADER,     1, COL_X_LF,    WW - 3,         42,     42 + 13,    STR_NONE,   STR_TILE_INSPECTOR_FLAG_LAST },                         /* Last of tile flag */     \
    { WWT_GROUPBOX,     1, 6,           WW - 6,         -1,     -1,         STR_NONE,                                   STR_NONE },                 /* Details group box */     \
    { WWT_GROUPBOX,     1, 6,           WW - 6,         -1,     -1,         STR_TILE_INSPECTOR_GROUPBOX_PROPERTIES,     STR_NONE }                  /* Properties group box */

static rct_widget DefaultWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
    { WIDGETS_END },
};

// Group boxes .top and .bottom for a given window height offsets from the bottom
constexpr int32_t SUR_GBPB = PADDING_BOTTOM;               // Surface group box properties bottom
constexpr int32_t SUR_GBPT = SUR_GBPB + 16 + 4 * 21;       // Surface group box properties top
constexpr int32_t SUR_GBDB = SUR_GBPT + GROUPBOX_PADDING;  // Surface group box details bottom
constexpr int32_t SUR_GBDT = SUR_GBDB + 20 + 4 * 11;       // Surface group box details top
static rct_widget SurfaceWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - SUR_GBPT, 0) + 3, GBBB(WH - SUR_GBPT, 0) - 3,   STR_NONE, STR_NONE),  // WIDX_SURFACE_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_BUTTON,           1,  GBB(WH - SUR_GBPT, 0, 1),   STR_TILE_INSPECTOR_SURFACE_REMOVE_FENCES,   STR_NONE }, // WIDX_SURFACE_BUTTON_REMOVE_FENCES
    { WWT_BUTTON,           1,  GBB(WH - SUR_GBPT, 1, 1),   STR_TILE_INSPECTOR_SURFACE_RESTORE_FENCES,  STR_NONE }, // WIDX_SURFACE_BUTTON_RESTORE_FENCES
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SUR_GBPT, 2) + 7 * 0),  STR_NONE,   STR_NONE }, // WIDX_SURFACE_CHECK_CORNER_N
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 2, GBBT(WH - SUR_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SURFACE_CHECK_CORNER_E
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SUR_GBPT, 2) + 7 * 2),  STR_NONE,   STR_NONE }, // WIDX_SURFACE_CHECK_CORNER_S
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 0, GBBT(WH - SUR_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SURFACE_CHECK_CORNER_W
    { WWT_CHECKBOX,         1,  GBBF(WH - SUR_GBPT, 0, 4),  STR_TILE_INSPECTOR_SURFACE_DIAGONAL,    STR_NONE }, // WIDX_SURFACE_CHECK_DIAGONAL
    { WIDGETS_END },
};

constexpr int32_t PAT_GBPB = PADDING_BOTTOM;               // Path group box properties bottom
constexpr int32_t PAT_GBPT = PAT_GBPB + 16 + 5 * 21;       // Path group box properties top
constexpr int32_t PAT_GBDB = PAT_GBPT + GROUPBOX_PADDING;  // Path group box info bottom
constexpr int32_t PAT_GBDT = PAT_GBDB + 20 + 2 * 11;       // Path group box info top
static rct_widget PathWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - PAT_GBPT, 0) + 3, GBBB(WH - PAT_GBPT, 0) - 3,  STR_NONE,  STR_NONE),  // WIDX_PATH_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_CHECKBOX,         1,  GBBF(WH - PAT_GBPT, 0, 1),              STR_TILE_INSPECTOR_PATH_BROKEN, STR_NONE }, // WIDX_PATH_CHECK_BROKEN
    { WWT_CHECKBOX,         1,  GBBF(WH - PAT_GBPT, 0, 2),              STR_TILE_INSPECTOR_PATH_SLOPED, STR_NONE }, // WIDX_PATH_CHECK_SLOPED
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 3, GBBT(WH - PAT_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_NE
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 4, GBBT(WH - PAT_GBPT, 2) + 7 * 2),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_E
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 3, GBBT(WH - PAT_GBPT, 2) + 7 * 3),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_SE
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 2, GBBT(WH - PAT_GBPT, 2) + 7 * 4),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_S
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - PAT_GBPT, 2) + 7 * 3),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_SW
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 0, GBBT(WH - PAT_GBPT, 2) + 7 * 2),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_W
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - PAT_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_NW
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 2, GBBT(WH - PAT_GBPT, 2) + 7 * 0),  STR_NONE,   STR_NONE }, // WIDX_PATH_CHECK_EDGE_N
    { WIDGETS_END },
};

constexpr int32_t TRA_GBPB = PADDING_BOTTOM;               // Track group box properties bottom
constexpr int32_t TRA_GBPT = TRA_GBPB + 16 + 5 * 21;       // Track group box properties top
constexpr int32_t TRA_GBDB = TRA_GBPT + GROUPBOX_PADDING;  // Track group box info bottom
constexpr int32_t TRA_GBDT = TRA_GBDB + 20 + 7 * 11;       // Track group box info top
static rct_widget TrackWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
    { WWT_CHECKBOX,         1,  GBBF(WH - TRA_GBPT, 0, 0),  STR_TILE_INSPECTOR_TRACK_ENTIRE_TRACK_PIECE,    STR_NONE }, // WIDX_TRACK_CHECK_APPLY_TO_ALL
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - TRA_GBPT, 1) + 3, GBBB(WH - TRA_GBPT, 1) - 3,   STR_NONE, STR_NONE),  // WIDX_TRACK_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_CHECKBOX,         1,  GBBF(WH - TRA_GBPT, 0, 2),  STR_TILE_INSPECTOR_TRACK_CHAIN_LIFT,            STR_NONE }, // WIDX_TRACK_CHECK_CHAIN_LIFT
    { WWT_CHECKBOX,         1,  GBBF(WH - TRA_GBPT, 0, 3),  STR_TILE_INSPECTOR_TRACK_BLOCK_BRAKE,           STR_NONE }, // WIDX_PATH_CHECK_BLOCK_BRAKE_CLOSED
    { WWT_CHECKBOX,         1,  GBBF(WH - TRA_GBPT, 0, 4),  STR_TILE_INSPECTOR_TRACK_IS_INDESTRUCTIBLE,     STR_NONE }, // WIDX_PATH_CHECK_IS_INDESTRUCTIBLE
    { WIDGETS_END },
};

constexpr int32_t SCE_GBPB = PADDING_BOTTOM;               // Scenery group box properties bottom
constexpr int32_t SCE_GBPT = SCE_GBPB + 16 + 4 * 21;       // Scenery group box properties top
constexpr int32_t SCE_GBDB = SCE_GBPT + GROUPBOX_PADDING;  // Scenery group box info bottom
constexpr int32_t SCE_GBDT = SCE_GBDB + 20 + 3 * 11;       // Scenery group box info top
static rct_widget SceneryWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - SCE_GBPT, 0) + 3, GBBB(WH - SCE_GBPT, 0) - 3,  STR_NONE,   STR_NONE),  // WIDX_SCENERY_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SCE_GBPT, 1) + 7 * 0),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_QUARTER_N
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 2, GBBT(WH - SCE_GBPT, 1) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_QUARTER_E
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SCE_GBPT, 1) + 7 * 2),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_QUARTER_S
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 0, GBBT(WH - SCE_GBPT, 1) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_QUARTER_W
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SCE_GBPT, 2) + 7 * 0),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_COLLISION_N
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 2, GBBT(WH - SCE_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_COLLISION_E
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1, GBBT(WH - SCE_GBPT, 2) + 7 * 2),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_COLLISION_S
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 0, GBBT(WH - SCE_GBPT, 2) + 7 * 1),  STR_NONE,   STR_NONE }, // WIDX_SCENERY_CHECK_COLLISION_W
    { WIDGETS_END },
};


constexpr int32_t ENT_GBPB = PADDING_BOTTOM;               // Entrance group box properties bottom
constexpr int32_t ENT_GBPT = ENT_GBPB + 16 + 2 * 21;       // Entrance group box properties top
constexpr int32_t ENT_GBDB = ENT_GBPT + GROUPBOX_PADDING;  // Entrance group box info bottom
constexpr int32_t ENT_GBDT = ENT_GBDB + 20 + 4 * 11;       // Entrance group box info top
static rct_widget EntranceWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - ENT_GBPT, 0) + 3, GBBB(WH - ENT_GBPT, 0) - 3,   STR_NONE,  STR_NONE),  // WIDX_ENTRANCE_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_BUTTON,           1,  GBB(WH - ENT_GBPT, 0, 1),   STR_TILE_INSPECTOR_ENTRANCE_MAKE_USABLE,        STR_TILE_INSPECTOR_ENTRANCE_MAKE_USABLE_TIP }, // WIDX_ENTRANCE_BUTTON_MAKE_USABLE
    { WIDGETS_END },
};

constexpr int32_t WALL_GBPB = PADDING_BOTTOM;                  // Wall group box properties bottom
constexpr int32_t WALL_GBPT = WALL_GBPB + 16 + 2 * 21;         // Wall group box properties top
constexpr int32_t WALL_GBDB = WALL_GBPT + GROUPBOX_PADDING;    // Wall group box info bottom
constexpr int32_t WALL_GBDT = WALL_GBDB + 20 + 2 * 11;         // Wall group box info top
static rct_widget WallWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - WALL_GBPT, 0) + 3, GBBB(WH - WALL_GBPT, 0) - 3, STR_NONE,  STR_NONE),  // WIDX_WALL_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_DROPDOWN,         1,  GBD(WH - WALL_GBPT, 1, 1),  STR_NONE,                                       STR_NONE }, // WIDX_WALL_DROPDOWN_SLOPE
    { WWT_BUTTON,           1,  GBDB(WH - WALL_GBPT, 1, 1), STR_DROPDOWN_GLYPH,                             STR_NONE }, // WIDX_WALL_DROPDOWN_SLOPE_BUTTON
    { WIDGETS_END },
};

constexpr int32_t LAR_GBPB = PADDING_BOTTOM;               // Large scenery group box properties bottom
constexpr int32_t LAR_GBPT = LAR_GBPB + 16 + 1 * 21;       // Large scenery group box properties top
constexpr int32_t LAR_GBDB = LAR_GBPT + GROUPBOX_PADDING;  // Large scenery group box info bottom
constexpr int32_t LAR_GBDT = LAR_GBDB + 20 + 3 * 11;       // Large scenery group box info top
static rct_widget LargeSceneryWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - LAR_GBPT, 0) + 3, GBBB(WH - LAR_GBPT, 0) - 3,   STR_NONE,  STR_NONE),  // WIDX_LARGE_SCENERY_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WIDGETS_END },
};

constexpr int32_t BAN_GBPB = PADDING_BOTTOM;               // Banner group box properties bottom
constexpr int32_t BAN_GBPT = BAN_GBPB + 16 + 3 * 21;       // Banner group box properties top
constexpr int32_t BAN_GBDB = BAN_GBPT + GROUPBOX_PADDING;  // Banner group box info bottom
constexpr int32_t BAN_GBDT = BAN_GBDB + 20 + 1 * 11;       // Banner group box info top
static rct_widget BannerWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - BAN_GBPT, 0) + 3, GBBB(WH - BAN_GBPT, 0) - 3,   STR_NONE,  STR_NONE),  // WIDX_BANNER_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 3,   GBBT(WH - BAN_GBPT, 1) + 7 * 1),    STR_NONE,       STR_NONE }, // WIDX_BANNER_CHECK_BLOCK_NE
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 3,   GBBT(WH - BAN_GBPT, 1) + 7 * 3),    STR_NONE,       STR_NONE }, // WIDX_BANNER_CHECK_BLOCK_SE
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1,   GBBT(WH - BAN_GBPT, 1) + 7 * 3),    STR_NONE,       STR_NONE }, // WIDX_BANNER_CHECK_BLOCK_SW
    { WWT_CHECKBOX,         1,  CHK(GBBL(1) + 14 * 1,   GBBT(WH - BAN_GBPT, 1) + 7 * 1),    STR_NONE,       STR_NONE }, // WIDX_BANNER_CHECK_BLOCK_NW

    { WIDGETS_END },
};

constexpr int32_t COR_GBPB = PADDING_BOTTOM;               // Corrupt element group box properties bottom
constexpr int32_t COR_GBPT = COR_GBPB + 16 + 2 * 21;       // Corrupt element group box properties top
constexpr int32_t COR_GBDB = COR_GBPT + GROUPBOX_PADDING;  // Corrupt element group box info bottom
constexpr int32_t COR_GBDT = COR_GBDB + 20 + 0 * 11;       // Corrupt element group box info top
static rct_widget CorruptWidgets[] = {
    MAIN_TILE_INSPECTOR_WIDGETS,
      SPINNER_WIDGETS      (1,  GBBL(1), GBBR(1), GBBT(WH - COR_GBPT, 0) + 3, GBBB(WH - COR_GBPT, 0) - 3,   STR_NONE,  STR_NONE),  // WIDX_CORRUPT_SPINNER_HEIGHT{,_INCREASE,_DECREASE}
    { WWT_BUTTON,           1,  GBB(WH - SUR_GBPT, 0, 1),   STR_TILE_INSPECTOR_CLAMP_TO_NEXT, STR_TILE_INSPECTOR_CLAMP_TO_NEXT_TIP }, // WIDX_CORRUPT_BUTTON_CLAMP
    { WIDGETS_END },
};

static rct_widget *PageWidgets[] = {
    DefaultWidgets,
    SurfaceWidgets,
    PathWidgets,
    TrackWidgets,
    SceneryWidgets,
    EntranceWidgets,
    WallWidgets,
    LargeSceneryWidgets,
    BannerWidgets,
    CorruptWidgets,
};

static struct {
    // Offsets from the bottom of the window
    int16_t details_top_offset, details_bottom_offset;
    int16_t properties_top_offset, properties_bottom_offset;
    // String to be displayed in the details groupbox
    rct_string_id string_id;
} PageGroupBoxSettings[] = {
    { SUR_GBDT, SUR_GBDB, SUR_GBPT, SUR_GBPB, STR_TILE_INSPECTOR_GROUPBOX_SURFACE_INFO },
    { PAT_GBDT, PAT_GBDB, PAT_GBPT, PAT_GBPB, STR_TILE_INSPECTOR_GROUPBOX_PATH_INFO },
    { TRA_GBDT, TRA_GBDB, TRA_GBPT, TRA_GBPB, STR_TILE_INSPECTOR_GROUPBOX_TRACK_INFO },
    { SCE_GBDT, SCE_GBDB, SCE_GBPT, SCE_GBPB, STR_TILE_INSPECTOR_GROUPBOX_SCENERY_INFO },
    { ENT_GBDT, ENT_GBDB, ENT_GBPT, ENT_GBPB, STR_TILE_INSPECTOR_GROUPBOX_ENTRANCE_INFO },
    { WALL_GBDT, WALL_GBDB, WALL_GBPT, WALL_GBPB, STR_TILE_INSPECTOR_GROUPBOX_WALL_INFO },
    { LAR_GBDT, LAR_GBDB, LAR_GBPT, LAR_GBPB, STR_TILE_INSPECTOR_GROUPBOX_LARGE_SCENERY_INFO },
    { BAN_GBDT, BAN_GBDB, BAN_GBPT, BAN_GBPB, STR_TILE_INSPECTOR_GROUPBOX_BANNER_INFO },
    { COR_GBDT, COR_GBDB, COR_GBPT, COR_GBPB, STR_TILE_INSPECTOR_GROUPBOX_CORRUPT_INFO }
};

// clang-format on
static constexpr int32_t ViewportInteractionFlags = VIEWPORT_INTERACTION_MASK_TERRAIN & VIEWPORT_INTERACTION_MASK_SPRITE
    & VIEWPORT_INTERACTION_MASK_RIDE & VIEWPORT_INTERACTION_MASK_SCENERY & VIEWPORT_INTERACTION_MASK_FOOTPATH
    & VIEWPORT_INTERACTION_MASK_FOOTPATH_ITEM & VIEWPORT_INTERACTION_MASK_PARK & VIEWPORT_INTERACTION_MASK_WALL
    & VIEWPORT_INTERACTION_MASK_LARGE_SCENERY & VIEWPORT_INTERACTION_MASK_BANNER;

static int16_t windowTileInspectorHighlightedIndex = -1;
static bool windowTileInspectorTileSelected = false;
static int32_t windowTileInspectorToolMouseX = 0;
static int32_t windowTileInspectorToolMouseY = 0;
static bool windowTileInspectorToolCtrlDown = false;
static CoordsXY windowTileInspectorToolMap = {};
static bool windowTileInspectorApplyToAll = false;
static bool windowTileInspectorElementCopied = false;
static TileElement tileInspectorCopiedElement;

static void window_tile_inspector_mouseup(rct_window* w, rct_widgetindex widgetIndex);
static void window_tile_inspector_resize(rct_window* w);
static void window_tile_inspector_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_tile_inspector_update(rct_window* w);
static void window_tile_inspector_dropdown(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex);
static void window_tile_inspector_tool_update(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_tile_inspector_update_selected_tile(rct_window* w, int32_t x, int32_t y);
static void window_tile_inspector_tool_down(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_tile_inspector_tool_drag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords);
static void window_tile_inspector_scrollgetsize(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height);
static void window_tile_inspector_scrollmousedown(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y);
static void window_tile_inspector_scrollmouseover(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y);
static void window_tile_inspector_invalidate(rct_window* w);
static void window_tile_inspector_paint(rct_window* w, rct_drawpixelinfo* dpi);
static void window_tile_inspector_scrollpaint(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex);
static void window_tile_inspector_set_page(rct_window* w, const TILE_INSPECTOR_PAGE page);

// clang-format off
static rct_window_event_list TileInspectorWindowEvents = {
    nullptr,
    window_tile_inspector_mouseup,
    window_tile_inspector_resize,
    window_tile_inspector_mousedown,
    window_tile_inspector_dropdown,
    nullptr,
    window_tile_inspector_update,
    nullptr,
    nullptr,
    window_tile_inspector_tool_update,
    window_tile_inspector_tool_down,
    window_tile_inspector_tool_drag,
    nullptr,
    nullptr,
    nullptr,
    window_tile_inspector_scrollgetsize,
    window_tile_inspector_scrollmousedown,
    nullptr,
    window_tile_inspector_scrollmouseover,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_tile_inspector_invalidate,
    window_tile_inspector_paint,
    window_tile_inspector_scrollpaint
};

static uint64_t PageEnabledWidgets[] = {
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_SURFACE_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_SURFACE_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_SURFACE_BUTTON_REMOVE_FENCES) | (1ULL << WIDX_SURFACE_BUTTON_RESTORE_FENCES) | (1ULL << WIDX_SURFACE_CHECK_CORNER_N) | (1ULL << WIDX_SURFACE_CHECK_CORNER_E) | (1ULL << WIDX_SURFACE_CHECK_CORNER_S) | (1ULL << WIDX_SURFACE_CHECK_CORNER_W) | (1ULL << WIDX_SURFACE_CHECK_DIAGONAL),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_PATH_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_PATH_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_PATH_CHECK_SLOPED) | (1ULL << WIDX_PATH_CHECK_BROKEN) | (1ULL << WIDX_PATH_CHECK_EDGE_N) | (1ULL << WIDX_PATH_CHECK_EDGE_NE) | (1ULL << WIDX_PATH_CHECK_EDGE_E) | (1ULL << WIDX_PATH_CHECK_EDGE_SE) | (1ULL << WIDX_PATH_CHECK_EDGE_S) | (1ULL << WIDX_PATH_CHECK_EDGE_SW) | (1ULL << WIDX_PATH_CHECK_EDGE_W) | (1ULL << WIDX_PATH_CHECK_EDGE_NW),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_TRACK_CHECK_APPLY_TO_ALL) | (1ULL << WIDX_TRACK_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_TRACK_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_TRACK_CHECK_CHAIN_LIFT) | (1ULL << WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED) | (1ULL << WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_SCENERY_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_SCENERY_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_SCENERY_CHECK_QUARTER_N) | (1ULL << WIDX_SCENERY_CHECK_QUARTER_E) | (1ULL << WIDX_SCENERY_CHECK_QUARTER_S) | (1ULL << WIDX_SCENERY_CHECK_QUARTER_W) | (1ULL << WIDX_SCENERY_CHECK_COLLISION_N) | (1ULL << WIDX_SCENERY_CHECK_COLLISION_E) | (1ULL << WIDX_SCENERY_CHECK_COLLISION_S) | (1ULL << WIDX_SCENERY_CHECK_COLLISION_W),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_ENTRANCE_BUTTON_MAKE_USABLE),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_WALL_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_WALL_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_WALL_DROPDOWN_SLOPE) | (1ULL << WIDX_WALL_DROPDOWN_SLOPE_BUTTON),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_BANNER_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_BANNER_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_BANNER_CHECK_BLOCK_NE) | (1ULL << WIDX_BANNER_CHECK_BLOCK_SE) | (1ULL << WIDX_BANNER_CHECK_BLOCK_SW) | (1ULL << WIDX_BANNER_CHECK_BLOCK_NW),
    (1ULL << WIDX_CLOSE) | (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_COPY) | (1ULL << WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE) | (1ULL << WIDX_CORRUPT_BUTTON_CLAMP),
};

static uint64_t PageHoldDownWidgets[] = {
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_SURFACE_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_SURFACE_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_PATH_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_PATH_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_TRACK_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_TRACK_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_SCENERY_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_SCENERY_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_WALL_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_WALL_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_BANNER_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_BANNER_SPINNER_HEIGHT_DECREASE),
    (1ULL << WIDX_SPINNER_X_INCREASE) | (1ULL << WIDX_SPINNER_X_DECREASE) | (1ULL << WIDX_SPINNER_Y_INCREASE) | (1ULL << WIDX_SPINNER_Y_DECREASE) | (1ULL << WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE) | (1ULL << WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE),
};

static uint64_t PageDisabledWidgets[] = {
    (1ULL << WIDX_BUTTON_CORRUPT) | (1ULL << WIDX_BUTTON_MOVE_UP) | (1ULL << WIDX_BUTTON_MOVE_DOWN) | (1ULL << WIDX_BUTTON_REMOVE) | (1ULL << WIDX_BUTTON_ROTATE) | (1ULL << WIDX_BUTTON_COPY),
    0,
    0,
    0,
    0,
    0,
    0,
    (1ULL << WIDX_BUTTON_ROTATE),
    0,
    (1ULL << WIDX_BUTTON_ROTATE),
};
// clang-format on

rct_window* window_tile_inspector_open()
{
    rct_window* window;

    // Check if window is already open
    window = window_bring_to_front_by_class(WC_TILE_INSPECTOR);
    if (window != nullptr)
        return window;

    window = window_create(ScreenCoordsXY(0, 29), WW, WH, &TileInspectorWindowEvents, WC_TILE_INSPECTOR, WF_RESIZABLE);

    window_tile_inspector_set_page(window, TILE_INSPECTOR_PAGE_DEFAULT);
    window->min_width = MIN_WW;
    window->min_height = MIN_WH;
    window->max_width = MAX_WW;
    window->max_height = MAX_WH;
    windowTileInspectorSelectedIndex = -1;
    window_init_scroll_widgets(window);

    windowTileInspectorTileSelected = false;

    tool_set(window, WIDX_BACKGROUND, TOOL_CROSSHAIR);

    return window;
}

void window_tile_inspector_clear_clipboard()
{
    windowTileInspectorElementCopied = false;
}

static TileElement* window_tile_inspector_get_selected_element(rct_window* w)
{
    openrct2_assert(
        windowTileInspectorSelectedIndex >= 0 && windowTileInspectorSelectedIndex < windowTileInspectorElementCount,
        "Selected list item out of range");
    return map_get_first_element_at(windowTileInspectorToolMap.x / 32, windowTileInspectorToolMap.y / 32)
        + windowTileInspectorSelectedIndex;
}

static void window_tile_inspector_select_element_from_list(rct_window* w, int32_t index)
{
    if (index < 0 || index >= windowTileInspectorElementCount)
    {
        windowTileInspectorSelectedIndex = -1;
    }
    else
    {
        windowTileInspectorSelectedIndex = index;
    }

    w->Invalidate();
}

static void window_tile_inspector_load_tile(rct_window* w, TileElement* elementToSelect)
{
    windowTileInspectorSelectedIndex = -1;
    w->scrolls[0].v_top = 0;

    TileElement* element = map_get_first_element_at(windowTileInspectorToolMap.x / 32, windowTileInspectorToolMap.y / 32);
    int16_t numItems = 0;
    do
    {
        if (element == nullptr)
            break;
        if (element == elementToSelect)
        {
            windowTileInspectorSelectedIndex = numItems;
        }

        numItems++;
    } while (!(element++)->IsLastForTile());

    windowTileInspectorElementCount = numItems;

    w->Invalidate();
}

static void window_tile_inspector_insert_corrupt_element(int32_t elementIndex)
{
    openrct2_assert(elementIndex >= 0 && elementIndex < windowTileInspectorElementCount, "elementIndex out of range");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnyInsertCorrupt, elementIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_remove_element(int32_t elementIndex)
{
    openrct2_assert(elementIndex >= 0 && elementIndex < windowTileInspectorElementCount, "elementIndex out of range");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnyRemove, elementIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_rotate_element(int32_t elementIndex)
{
    openrct2_assert(elementIndex >= 0 && elementIndex < windowTileInspectorElementCount, "elementIndex out of range");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnyRotate, elementIndex);
    GameActions::Execute(&modifyTile);
}

// Swap element with its parent
static void window_tile_inspector_swap_elements(int16_t first, int16_t second)
{
    openrct2_assert(first >= 0 && first < windowTileInspectorElementCount, "first out of range");
    openrct2_assert(second >= 0 && second < windowTileInspectorElementCount, "second out of range");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnySwap, first, second);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_sort_elements()
{
    openrct2_assert(windowTileInspectorTileSelected, "No tile selected");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnySort);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_copy_element(rct_window* w)
{
    // Copy value, in case the element gets moved
    tileInspectorCopiedElement = *window_tile_inspector_get_selected_element(w);
    windowTileInspectorElementCopied = true;
    w->Invalidate();
}

static void window_tile_inspector_paste_element(rct_window* w)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::AnyPaste, 0, 0, tileInspectorCopiedElement);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_base_height_offset(int16_t elementIndex, int8_t heightOffset)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::AnyBaseHeightOffset, elementIndex, heightOffset);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_surface_show_park_fences(bool showFences)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::SurfaceShowParkFences, showFences);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_surface_toggle_corner(int32_t cornerIndex)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::SurfaceToggleCorner, cornerIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_surface_toggle_diagonal()
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::SurfaceToggleDiagonal);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_path_set_sloped(int32_t elementIndex, bool sloped)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::PathSetSlope, elementIndex, sloped);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_path_set_broken(int32_t elementIndex, bool broken)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::PathSetBroken, elementIndex, broken);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_path_toggle_edge(int32_t elementIndex, int32_t cornerIndex)
{
    openrct2_assert(elementIndex >= 0 && elementIndex < windowTileInspectorElementCount, "elementIndex out of range");
    openrct2_assert(cornerIndex >= 0 && cornerIndex < 8, "cornerIndex out of range");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::PathToggleEdge, elementIndex, cornerIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_entrance_make_usable(int32_t elementIndex)
{
    Guard::ArgumentInRange(elementIndex, 0, windowTileInspectorElementCount - 1);
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::EntranceMakeUsable, elementIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_wall_set_slope(int32_t elementIndex, int32_t slopeValue)
{
    // Make sure only the correct bits are set
    openrct2_assert((slopeValue & 3) == slopeValue, "slopeValue doesn't match its mask");
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::WallSetSlope, elementIndex, slopeValue);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_track_block_height_offset(int32_t elementIndex, int8_t heightOffset)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::TrackBaseHeightOffset, elementIndex, heightOffset);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_track_block_set_lift(int32_t elementIndex, bool entireTrackBlock, bool chain)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, entireTrackBlock ? TileModifyType::TrackSetChainBlock : TileModifyType::TrackSetChain,
        elementIndex, chain);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_track_set_block_brake(int32_t elementIndex, bool blockBrake)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::TrackSetBlockBrake, elementIndex, blockBrake);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_track_set_indestructible(int32_t elementIndex, bool isIndestructible)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::TrackSetIndestructible, elementIndex, isIndestructible);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_quarter_tile_set(int32_t elementIndex, const int32_t quarterIndex)
{
    // quarterIndex is widget index relative to WIDX_SCENERY_CHECK_QUARTER_N, so a value from 0-3
    openrct2_assert(quarterIndex >= 0 && quarterIndex < 4, "quarterIndex out of range");
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::ScenerySetQuarterLocation, elementIndex,
        (quarterIndex - get_current_rotation()) & 3);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_toggle_quadrant_collosion(int32_t elementIndex, const int32_t quadrantIndex)
{
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::ScenerySetQuarterCollision, elementIndex,
        (quadrantIndex + 2 - get_current_rotation()) & 3);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_banner_toggle_block(int32_t elementIndex, int32_t edgeIndex)
{
    openrct2_assert(edgeIndex >= 0 && edgeIndex < 4, "edgeIndex out of range");

    // Make edgeIndex abstract
    edgeIndex = (edgeIndex - get_current_rotation()) & 3;
    auto modifyTile = TileModifyAction(
        windowTileInspectorToolMap, TileModifyType::BannerToggleBlockingEdge, elementIndex, edgeIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_clamp_corrupt(int32_t elementIndex)
{
    auto modifyTile = TileModifyAction(windowTileInspectorToolMap, TileModifyType::CorruptClamp, elementIndex);
    GameActions::Execute(&modifyTile);
}

static void window_tile_inspector_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_CLOSE:
            tool_cancel();
            window_close(w);
            return;
        case WIDX_BUTTON_CORRUPT:
            window_tile_inspector_insert_corrupt_element(windowTileInspectorSelectedIndex);
            break;
        case WIDX_BUTTON_REMOVE:
        {
            int32_t nextItemToSelect = windowTileInspectorSelectedIndex - 1;
            window_tile_inspector_remove_element(windowTileInspectorSelectedIndex);
            window_tile_inspector_select_element_from_list(w, nextItemToSelect);
            break;
        }
        case WIDX_BUTTON_ROTATE:
            window_tile_inspector_rotate_element(windowTileInspectorSelectedIndex);
            break;
        case WIDX_BUTTON_SORT:
            window_tile_inspector_sort_elements();
            break;
        case WIDX_BUTTON_COPY:
            window_tile_inspector_copy_element(w);
            break;
        case WIDX_BUTTON_PASTE:
            window_tile_inspector_paste_element(w);
            break;
        case WIDX_BUTTON_MOVE_UP:
            window_tile_inspector_swap_elements(windowTileInspectorSelectedIndex, windowTileInspectorSelectedIndex + 1);
            break;
        case WIDX_BUTTON_MOVE_DOWN:
            window_tile_inspector_swap_elements(windowTileInspectorSelectedIndex - 1, windowTileInspectorSelectedIndex);
            break;
    }

    // Only element-specific widgets from now on
    if (w->page == TILE_INSPECTOR_PAGE_DEFAULT || windowTileInspectorSelectedIndex == -1)
    {
        return;
    }

    // Get the selected map element
    TileElement* const tileElement = window_tile_inspector_get_selected_element(w);

    // Page widgets
    switch (w->page)
    {
        case TILE_INSPECTOR_PAGE_SURFACE:
            switch (widgetIndex)
            {
                case WIDX_SURFACE_BUTTON_REMOVE_FENCES:
                    window_tile_inspector_surface_show_park_fences(false);
                    break;
                case WIDX_SURFACE_BUTTON_RESTORE_FENCES:
                    window_tile_inspector_surface_show_park_fences(true);
                    break;
                case WIDX_SURFACE_CHECK_CORNER_N:
                case WIDX_SURFACE_CHECK_CORNER_E:
                case WIDX_SURFACE_CHECK_CORNER_S:
                case WIDX_SURFACE_CHECK_CORNER_W:
                    window_tile_inspector_surface_toggle_corner(
                        ((widgetIndex - WIDX_SURFACE_CHECK_CORNER_N) + 2 - get_current_rotation()) & 3);
                    break;
                case WIDX_SURFACE_CHECK_DIAGONAL:
                    window_tile_inspector_surface_toggle_diagonal();
                    break;
            } // switch widgetindex
            break;

        case TILE_INSPECTOR_PAGE_PATH:
            switch (widgetIndex)
            {
                case WIDX_PATH_CHECK_SLOPED:
                    window_tile_inspector_path_set_sloped(windowTileInspectorSelectedIndex, !tileElement->AsPath()->IsSloped());
                    break;
                case WIDX_PATH_CHECK_BROKEN:
                    window_tile_inspector_path_set_broken(windowTileInspectorSelectedIndex, !tileElement->AsPath()->IsBroken());
                    break;
                case WIDX_PATH_CHECK_EDGE_E:
                case WIDX_PATH_CHECK_EDGE_S:
                case WIDX_PATH_CHECK_EDGE_W:
                case WIDX_PATH_CHECK_EDGE_N:
                {
                    // 0 = east/right, 1 = south/bottom, 2 = west/left, 3 = north/top
                    const int32_t eswn = (widgetIndex - WIDX_PATH_CHECK_EDGE_E) / 2;
                    // Transform to world orientation
                    const int32_t index = (eswn - get_current_rotation()) & 3;
                    window_tile_inspector_path_toggle_edge(
                        windowTileInspectorSelectedIndex,
                        index + 4); // The corners are stored in the 4 most significant bits, hence the + 4
                    break;
                }
                case WIDX_PATH_CHECK_EDGE_NE:
                case WIDX_PATH_CHECK_EDGE_SE:
                case WIDX_PATH_CHECK_EDGE_SW:
                case WIDX_PATH_CHECK_EDGE_NW:
                {
                    // 0 = NE, 1 = SE, 2 = SW, 3 = NW
                    const int32_t neseswnw = (widgetIndex - WIDX_PATH_CHECK_EDGE_NE) / 2;
                    // Transform to world orientation
                    const int32_t index = (neseswnw - get_current_rotation()) & 3;
                    window_tile_inspector_path_toggle_edge(windowTileInspectorSelectedIndex, index);
                    break;
                }
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_TRACK:
            switch (widgetIndex)
            {
                case WIDX_TRACK_CHECK_APPLY_TO_ALL:
                    windowTileInspectorApplyToAll ^= 1;
                    widget_invalidate(w, widgetIndex);
                    break;
                case WIDX_TRACK_CHECK_CHAIN_LIFT:
                {
                    bool entireTrackBlock = widget_is_pressed(w, WIDX_TRACK_CHECK_APPLY_TO_ALL);
                    bool newLift = !tileElement->AsTrack()->HasChain();
                    window_tile_inspector_track_block_set_lift(windowTileInspectorSelectedIndex, entireTrackBlock, newLift);
                    break;
                }
                case WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED:
                    window_tile_inspector_track_set_block_brake(
                        windowTileInspectorSelectedIndex, !tileElement->AsTrack()->BlockBrakeClosed());
                    break;
                case WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE:
                    window_tile_inspector_track_set_indestructible(
                        windowTileInspectorSelectedIndex, !tileElement->AsTrack()->IsIndestructible());
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_SCENERY:
            switch (widgetIndex)
            {
                case WIDX_SCENERY_CHECK_QUARTER_N:
                case WIDX_SCENERY_CHECK_QUARTER_E:
                case WIDX_SCENERY_CHECK_QUARTER_S:
                case WIDX_SCENERY_CHECK_QUARTER_W:
                    window_tile_inspector_quarter_tile_set(
                        windowTileInspectorSelectedIndex, widgetIndex - WIDX_SCENERY_CHECK_QUARTER_N);
                    break;
                case WIDX_SCENERY_CHECK_COLLISION_N:
                case WIDX_SCENERY_CHECK_COLLISION_E:
                case WIDX_SCENERY_CHECK_COLLISION_S:
                case WIDX_SCENERY_CHECK_COLLISION_W:
                    window_tile_inspector_toggle_quadrant_collosion(
                        windowTileInspectorSelectedIndex, widgetIndex - WIDX_SCENERY_CHECK_COLLISION_N);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_ENTRANCE:
            switch (widgetIndex)
            {
                case WIDX_ENTRANCE_BUTTON_MAKE_USABLE:
                    window_tile_inspector_entrance_make_usable(windowTileInspectorSelectedIndex);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_BANNER:
            switch (widgetIndex)
            {
                case WIDX_BANNER_CHECK_BLOCK_NE:
                case WIDX_BANNER_CHECK_BLOCK_SE:
                case WIDX_BANNER_CHECK_BLOCK_SW:
                case WIDX_BANNER_CHECK_BLOCK_NW:
                    window_tile_inspector_banner_toggle_block(
                        windowTileInspectorSelectedIndex, widgetIndex - WIDX_BANNER_CHECK_BLOCK_NE);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_CORRUPT:
            switch (widgetIndex)
            {
                case WIDX_CORRUPT_BUTTON_CLAMP:
                    window_tile_inspector_clamp_corrupt(windowTileInspectorSelectedIndex);
                    break;
            } // switch widget index
            break;
    } // switch page
}

static void window_tile_inspector_resize(rct_window* w)
{
    w->min_width = WW;
    w->min_height = MIN_WH;
    if (w->width < w->min_width)
    {
        w->Invalidate();
        w->width = w->min_width;
    }
    if (w->height < w->min_height)
    {
        w->Invalidate();
        w->height = w->min_height;
    }
}

static void window_tile_inspector_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex)
    {
        case WIDX_SPINNER_X_INCREASE:
            windowTileInspectorTileX = std::min<uint32_t>(windowTileInspectorTileX + 1, MAXIMUM_MAP_SIZE_TECHNICAL - 1);
            windowTileInspectorToolMap.x = std::min<int32_t>(
                windowTileInspectorToolMap.x + 32, (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32);
            window_tile_inspector_load_tile(w, nullptr);
            break;
        case WIDX_SPINNER_X_DECREASE:
            windowTileInspectorTileX = std::max<uint32_t>(windowTileInspectorTileX - 1, 0);
            windowTileInspectorToolMap.x = std::max<int32_t>(windowTileInspectorToolMap.x - 32, 0);
            window_tile_inspector_load_tile(w, nullptr);
            break;
        case WIDX_SPINNER_Y_INCREASE:
            windowTileInspectorTileY = std::min<uint32_t>(windowTileInspectorTileY + 1, MAXIMUM_MAP_SIZE_TECHNICAL - 1);
            windowTileInspectorToolMap.y = std::min<int32_t>(
                windowTileInspectorToolMap.y + 32, (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32);
            window_tile_inspector_load_tile(w, nullptr);
            break;
        case WIDX_SPINNER_Y_DECREASE:
            windowTileInspectorTileY = std::max<uint32_t>(windowTileInspectorTileY - 1, 0);
            windowTileInspectorToolMap.y = std::max<int32_t>(windowTileInspectorToolMap.y - 32, 0);
            window_tile_inspector_load_tile(w, nullptr);
            break;
    } // switch widget index

    // Only element-specific widgets from now on
    if (w->page == TILE_INSPECTOR_PAGE_DEFAULT || windowTileInspectorSelectedIndex == -1)
    {
        return;
    }

    switch (w->page)
    {
        case TILE_INSPECTOR_PAGE_SURFACE:
            switch (widgetIndex)
            {
                case WIDX_SURFACE_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_SURFACE_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_PATH:
            switch (widgetIndex)
            {
                case WIDX_PATH_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_PATH_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_TRACK:
            switch (widgetIndex)
            {
                case WIDX_TRACK_SPINNER_HEIGHT_INCREASE:
                    if (widget_is_pressed(w, WIDX_TRACK_CHECK_APPLY_TO_ALL))
                    {
                        window_tile_inspector_track_block_height_offset(windowTileInspectorSelectedIndex, 1);
                    }
                    else
                    {
                        window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    }
                    break;
                case WIDX_TRACK_SPINNER_HEIGHT_DECREASE:
                    if (widget_is_pressed(w, WIDX_TRACK_CHECK_APPLY_TO_ALL))
                    {
                        window_tile_inspector_track_block_height_offset(windowTileInspectorSelectedIndex, -1);
                    }
                    else
                    {
                        window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    }
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_SCENERY:
            switch (widgetIndex)
            {
                case WIDX_SCENERY_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_SCENERY_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_ENTRANCE:
            switch (widgetIndex)
            {
                case WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
                case WIDX_ENTRANCE_BUTTON_MAKE_USABLE:
                    window_tile_inspector_entrance_make_usable(windowTileInspectorSelectedIndex);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_WALL:
            switch (widgetIndex)
            {
                case WIDX_WALL_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_WALL_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
                case WIDX_WALL_DROPDOWN_SLOPE_BUTTON:
                    // Use dropdown instead of dropdown button
                    widget--;

                    // Fill dropdown list
                    gDropdownItemsFormat[0] = STR_DROPDOWN_MENU_LABEL;
                    gDropdownItemsFormat[1] = STR_DROPDOWN_MENU_LABEL;
                    gDropdownItemsFormat[2] = STR_DROPDOWN_MENU_LABEL;
                    gDropdownItemsArgs[0] = STR_TILE_INSPECTOR_WALL_FLAT;
                    gDropdownItemsArgs[1] = STR_TILE_INSPECTOR_WALL_SLOPED_LEFT;
                    gDropdownItemsArgs[2] = STR_TILE_INSPECTOR_WALL_SLOPED_RIGHT;
                    window_dropdown_show_text_custom_width(
                        w->x + widget->left, w->y + widget->top, widget->bottom - widget->top + 1, w->colours[1], 0,
                        DROPDOWN_FLAG_STAY_OPEN, 3, widget->right - widget->left - 3);

                    // Set current value as checked
                    TileElement* const tileElement = window_tile_inspector_get_selected_element(w);
                    dropdown_set_checked(tileElement->AsWall()->GetSlope(), true);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_LARGE_SCENERY:
            switch (widgetIndex)
            {
                case WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_BANNER:
            switch (widgetIndex)
            {
                case WIDX_BANNER_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_BANNER_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
            break;

        case TILE_INSPECTOR_PAGE_CORRUPT:
            switch (widgetIndex)
            {
                case WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, 1);
                    break;
                case WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE:
                    window_tile_inspector_base_height_offset(windowTileInspectorSelectedIndex, -1);
                    break;
            } // switch widget index
    }         // switch page
}

static void window_tile_inspector_update(rct_window* w)
{
    // Check if the mouse is hovering over the list
    if (!widget_is_highlighted(w, WIDX_LIST))
    {
        windowTileInspectorHighlightedIndex = -1;
        widget_invalidate(w, WIDX_LIST);
    }

    if (gCurrentToolWidget.window_classification != WC_TILE_INSPECTOR)
        window_close(w);
}

static void window_tile_inspector_dropdown(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex)
{
    if (dropdownIndex == -1)
    {
        return;
    }

    // Get selected element
    TileElement* const tileElement = window_tile_inspector_get_selected_element(w);

    switch (w->page)
    {
        case TILE_INSPECTOR_PAGE_WALL:
            openrct2_assert(tileElement->GetType() == TILE_ELEMENT_TYPE_WALL, "Element is not a wall");

            switch (widgetIndex)
            {
                case WIDX_WALL_DROPDOWN_SLOPE_BUTTON:
                    window_tile_inspector_wall_set_slope(windowTileInspectorSelectedIndex, dropdownIndex);
                    break;
            }
            break;
    }
}

static void window_tile_inspector_tool_update(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    map_invalidate_selection_rect();

    gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;

    int16_t mapX = screenCoords.x;
    int16_t mapY = screenCoords.y;
    TileElement* clickedElement = nullptr;
    if (input_test_place_object_modifier(PLACE_OBJECT_MODIFIER_COPY_Z))
    {
        get_map_coordinates_from_pos(
            screenCoords.x, screenCoords.y, ViewportInteractionFlags, &mapX, &mapY, nullptr, &clickedElement, nullptr);
    }

    // Even if Ctrl was pressed, fall back to normal selection when there was nothing under the cursor
    if (clickedElement == nullptr)
    {
        CoordsXY mapCoords = screen_pos_to_map_pos(mapX, mapY, nullptr);
        mapX = mapCoords.x;
        mapY = mapCoords.y;
    }

    if (mapX != LOCATION_NULL)
    {
        gMapSelectPositionA.x = gMapSelectPositionB.x = mapX;
        gMapSelectPositionA.y = gMapSelectPositionB.y = mapY;
    }
    else if (windowTileInspectorTileSelected)
    {
        gMapSelectPositionA.x = gMapSelectPositionB.x = windowTileInspectorToolMap.x;
        gMapSelectPositionA.y = gMapSelectPositionB.y = windowTileInspectorToolMap.y;
    }
    else
    {
        gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    }

    gMapSelectType = MAP_SELECT_TYPE_FULL;
    map_invalidate_selection_rect();
}

static void window_tile_inspector_update_selected_tile(rct_window* w, int32_t x, int32_t y)
{
    const bool ctrlIsHeldDown = input_test_place_object_modifier(PLACE_OBJECT_MODIFIER_COPY_Z);

    // Mouse hasn't moved
    if (x == windowTileInspectorToolMouseX && y == windowTileInspectorToolMouseY
        && windowTileInspectorToolCtrlDown == ctrlIsHeldDown)
    {
        return;
    }

    windowTileInspectorToolMouseX = x;
    windowTileInspectorToolMouseY = y;
    windowTileInspectorToolCtrlDown = ctrlIsHeldDown;

    int16_t mapX = x;
    int16_t mapY = y;
    TileElement* clickedElement = nullptr;
    if (ctrlIsHeldDown)
    {
        get_map_coordinates_from_pos(x, y, ViewportInteractionFlags, &mapX, &mapY, nullptr, &clickedElement, nullptr);
    }

    // Even if Ctrl was pressed, fall back to normal selection when there was nothing under the cursor
    if (clickedElement == nullptr)
    {
        CoordsXY mapCoords = screen_pos_to_map_pos(mapX, mapY, nullptr);

        if (mapCoords.x == LOCATION_NULL)
        {
            return;
        }

        // Tile is already selected
        if (windowTileInspectorTileSelected && mapCoords.x == windowTileInspectorToolMap.x
            && mapCoords.y == windowTileInspectorToolMap.y)
        {
            return;
        }
        mapX = mapCoords.x;
        mapY = mapCoords.y;
    }

    windowTileInspectorTileSelected = true;
    windowTileInspectorToolMap.x = mapX;
    windowTileInspectorToolMap.y = mapY;
    windowTileInspectorTileX = mapX >> 5;
    windowTileInspectorTileY = mapY >> 5;

    window_tile_inspector_load_tile(w, clickedElement);
}

static void window_tile_inspector_tool_down(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    window_tile_inspector_update_selected_tile(w, screenCoords.x, screenCoords.y);
}

static void window_tile_inspector_tool_drag(rct_window* w, rct_widgetindex widgetIndex, ScreenCoordsXY screenCoords)
{
    window_tile_inspector_update_selected_tile(w, screenCoords.x, screenCoords.y);
}

static void window_tile_inspector_scrollgetsize(rct_window* w, int32_t scrollIndex, int32_t* width, int32_t* height)
{
    *width = WW - 30;
    *height = windowTileInspectorElementCount * SCROLLABLE_ROW_HEIGHT;
}

static void window_tile_inspector_set_page(rct_window* w, const TILE_INSPECTOR_PAGE page)
{
    // Invalidate the window already, because the size may change
    w->Invalidate();

    // subtract current page height, then add new page height
    if (w->page != TILE_INSPECTOR_PAGE_DEFAULT)
    {
        w->height -= PageGroupBoxSettings[w->page - 1].details_top_offset - GROUPBOX_PADDING - 3;
        w->min_height -= PageGroupBoxSettings[w->page - 1].details_top_offset - GROUPBOX_PADDING - 3;
    }
    if (page != TILE_INSPECTOR_PAGE_DEFAULT)
    {
        w->height += PageGroupBoxSettings[page - 1].details_top_offset - GROUPBOX_PADDING - 3;
        w->min_height += PageGroupBoxSettings[page - 1].details_top_offset - GROUPBOX_PADDING - 3;
    }
    w->page = page;
    w->widgets = PageWidgets[page];
    w->enabled_widgets = PageEnabledWidgets[page];
    w->hold_down_widgets = PageHoldDownWidgets[page];
    w->disabled_widgets = PageDisabledWidgets[page];
    w->pressed_widgets = 0;
}

static void window_tile_inspector_scrollmousedown(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y)
{
    // Because the list items are displayed in reverse order, subtract the calculated index from the amount of elements
    const int16_t index = windowTileInspectorElementCount - (y - 1) / SCROLLABLE_ROW_HEIGHT - 1;
    window_tile_inspector_select_element_from_list(w, index);
}

static void window_tile_inspector_scrollmouseover(rct_window* w, int32_t scrollIndex, int32_t x, int32_t y)
{
    int16_t index = windowTileInspectorElementCount - (y - 1) / SCROLLABLE_ROW_HEIGHT - 1;
    if (index < 0 || index >= windowTileInspectorElementCount)
        windowTileInspectorHighlightedIndex = -1;
    else
        windowTileInspectorHighlightedIndex = index;

    widget_invalidate(w, WIDX_LIST);
}

static void window_tile_inspector_invalidate(rct_window* w)
{
    // Set the correct page automatically
    TILE_INSPECTOR_PAGE page = TILE_INSPECTOR_PAGE_DEFAULT;
    if (windowTileInspectorSelectedIndex != -1)
    {
        const auto element = window_tile_inspector_get_selected_element(w);
        auto type = element->GetType();
        switch (type)
        {
            case TILE_ELEMENT_TYPE_SURFACE:
                page = TILE_INSPECTOR_PAGE_SURFACE;
                break;
            case TILE_ELEMENT_TYPE_PATH:
                page = TILE_INSPECTOR_PAGE_PATH;
                break;
            case TILE_ELEMENT_TYPE_TRACK:
                page = TILE_INSPECTOR_PAGE_TRACK;
                break;
            case TILE_ELEMENT_TYPE_SMALL_SCENERY:
                page = TILE_INSPECTOR_PAGE_SCENERY;
                break;
            case TILE_ELEMENT_TYPE_ENTRANCE:
                page = TILE_INSPECTOR_PAGE_ENTRANCE;
                break;
            case TILE_ELEMENT_TYPE_WALL:
                page = TILE_INSPECTOR_PAGE_WALL;
                break;
            case TILE_ELEMENT_TYPE_LARGE_SCENERY:
                page = TILE_INSPECTOR_PAGE_LARGE_SCENERY;
                break;
            case TILE_ELEMENT_TYPE_BANNER:
                page = TILE_INSPECTOR_PAGE_BANNER;
                break;
            case TILE_ELEMENT_TYPE_CORRUPT:
            default:
                page = TILE_INSPECTOR_PAGE_CORRUPT;
                break;
        }
    }

    if (w->page != page)
    {
        window_tile_inspector_set_page(w, page);
        w->Invalidate();
    }

    // X and Y spinners
    widget_set_enabled(
        w, WIDX_SPINNER_X_INCREASE,
        (windowTileInspectorTileSelected && ((windowTileInspectorToolMap.x / 32) < MAXIMUM_MAP_SIZE_TECHNICAL - 1)));
    widget_set_enabled(
        w, WIDX_SPINNER_X_DECREASE, (windowTileInspectorTileSelected && ((windowTileInspectorToolMap.x / 32) > 0)));
    widget_set_enabled(
        w, WIDX_SPINNER_Y_INCREASE,
        (windowTileInspectorTileSelected && ((windowTileInspectorToolMap.y / 32) < MAXIMUM_MAP_SIZE_TECHNICAL - 1)));
    widget_set_enabled(
        w, WIDX_SPINNER_Y_DECREASE, (windowTileInspectorTileSelected && ((windowTileInspectorToolMap.y / 32) > 0)));

    // Sort buttons
    widget_set_enabled(w, WIDX_BUTTON_SORT, (windowTileInspectorTileSelected && windowTileInspectorElementCount > 1));

    // Move Up button
    widget_set_enabled(
        w, WIDX_BUTTON_MOVE_UP,
        (windowTileInspectorSelectedIndex != -1 && windowTileInspectorSelectedIndex < windowTileInspectorElementCount - 1));
    widget_invalidate(w, WIDX_BUTTON_MOVE_UP);

    // Move Down button
    widget_set_enabled(w, WIDX_BUTTON_MOVE_DOWN, (windowTileInspectorSelectedIndex > 0));
    widget_invalidate(w, WIDX_BUTTON_MOVE_DOWN);

    // Copy button
    widget_set_enabled(w, WIDX_BUTTON_COPY, windowTileInspectorSelectedIndex >= 0);
    widget_invalidate(w, WIDX_BUTTON_COPY);

    // Paste button
    widget_set_enabled(w, WIDX_BUTTON_PASTE, windowTileInspectorTileSelected && windowTileInspectorElementCopied);
    widget_invalidate(w, WIDX_BUTTON_PASTE);

    w->widgets[WIDX_BACKGROUND].bottom = w->height - 1;

    if (w->page == TILE_INSPECTOR_PAGE_DEFAULT)
    {
        w->widgets[WIDX_GROUPBOX_DETAILS].type = WWT_EMPTY;
        w->widgets[WIDX_GROUPBOX_PROPERTIES].type = WWT_EMPTY;
        w->widgets[WIDX_LIST].bottom = w->height - PADDING_BOTTOM;
    }
    else
    {
        w->widgets[WIDX_GROUPBOX_DETAILS].type = WWT_GROUPBOX;
        w->widgets[WIDX_GROUPBOX_PROPERTIES].type = WWT_GROUPBOX;
        w->widgets[WIDX_GROUPBOX_DETAILS].text = PageGroupBoxSettings[w->page - 1].string_id;
        w->widgets[WIDX_GROUPBOX_DETAILS].top = w->height - PageGroupBoxSettings[w->page - 1].details_top_offset;
        w->widgets[WIDX_GROUPBOX_DETAILS].bottom = w->height - PageGroupBoxSettings[w->page - 1].details_bottom_offset;
        w->widgets[WIDX_GROUPBOX_PROPERTIES].top = w->height - PageGroupBoxSettings[w->page - 1].properties_top_offset;
        w->widgets[WIDX_GROUPBOX_PROPERTIES].bottom = w->height - PageGroupBoxSettings[w->page - 1].properties_bottom_offset;
        w->widgets[WIDX_LIST].bottom = w->widgets[WIDX_GROUPBOX_DETAILS].top - GROUPBOX_PADDING;
    }

    // The default page doesn't need further invalidation
    if (w->page == TILE_INSPECTOR_PAGE_DEFAULT)
    {
        return;
    }

    // Using a switch, because I don't think giving each page their own callbacks is
    // needed here, as only the mouseup and invalidate functions are different.
    const int32_t propertiesAnchor = w->widgets[WIDX_GROUPBOX_PROPERTIES].top;
    const TileElement* const tileElement = window_tile_inspector_get_selected_element(w);

    switch (w->page)
    {
        case TILE_INSPECTOR_PAGE_SURFACE:
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_SURFACE_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_SURFACE_BUTTON_REMOVE_FENCES].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_SURFACE_BUTTON_REMOVE_FENCES].bottom = GBBB(propertiesAnchor, 1);
            w->widgets[WIDX_SURFACE_BUTTON_RESTORE_FENCES].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_SURFACE_BUTTON_RESTORE_FENCES].bottom = GBBB(propertiesAnchor, 1);
            w->widgets[WIDX_SURFACE_CHECK_CORNER_N].top = GBBT(propertiesAnchor, 2) + 7 * 0;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_N].bottom = w->widgets[WIDX_SURFACE_CHECK_CORNER_N].top + 13;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_E].top = GBBT(propertiesAnchor, 2) + 7 * 1;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_E].bottom = w->widgets[WIDX_SURFACE_CHECK_CORNER_E].top + 13;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_S].top = GBBT(propertiesAnchor, 2) + 7 * 2;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_S].bottom = w->widgets[WIDX_SURFACE_CHECK_CORNER_S].top + 13;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_W].top = GBBT(propertiesAnchor, 2) + 7 * 1;
            w->widgets[WIDX_SURFACE_CHECK_CORNER_W].bottom = w->widgets[WIDX_SURFACE_CHECK_CORNER_W].top + 13;
            w->widgets[WIDX_SURFACE_CHECK_DIAGONAL].top = GBBT(propertiesAnchor, 3) + 7 * 1;
            w->widgets[WIDX_SURFACE_CHECK_DIAGONAL].bottom = w->widgets[WIDX_SURFACE_CHECK_DIAGONAL].top + 13;
            widget_set_checkbox_value(
                w, WIDX_SURFACE_CHECK_CORNER_N,
                tileElement->AsSurface()->GetSlope() & (1 << ((2 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_SURFACE_CHECK_CORNER_E,
                tileElement->AsSurface()->GetSlope() & (1 << ((3 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_SURFACE_CHECK_CORNER_S,
                tileElement->AsSurface()->GetSlope() & (1 << ((0 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_SURFACE_CHECK_CORNER_W,
                tileElement->AsSurface()->GetSlope() & (1 << ((1 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_SURFACE_CHECK_DIAGONAL, tileElement->AsSurface()->GetSlope() & TILE_ELEMENT_SLOPE_DOUBLE_HEIGHT);
            break;
        case TILE_INSPECTOR_PAGE_PATH:
            w->widgets[WIDX_PATH_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_PATH_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_PATH_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_PATH_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_PATH_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_PATH_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_PATH_CHECK_BROKEN].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_PATH_CHECK_BROKEN].bottom = GBBB(propertiesAnchor, 1);
            w->widgets[WIDX_PATH_CHECK_SLOPED].top = GBBT(propertiesAnchor, 2);
            w->widgets[WIDX_PATH_CHECK_SLOPED].bottom = GBBB(propertiesAnchor, 2);
            w->widgets[WIDX_PATH_CHECK_EDGE_N].top = GBBT(propertiesAnchor, 3) + 7 * 0;
            w->widgets[WIDX_PATH_CHECK_EDGE_N].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_N].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_NE].top = GBBT(propertiesAnchor, 3) + 7 * 1;
            w->widgets[WIDX_PATH_CHECK_EDGE_NE].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_NE].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_E].top = GBBT(propertiesAnchor, 3) + 7 * 2;
            w->widgets[WIDX_PATH_CHECK_EDGE_E].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_E].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_SE].top = GBBT(propertiesAnchor, 3) + 7 * 3;
            w->widgets[WIDX_PATH_CHECK_EDGE_SE].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_SE].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_S].top = GBBT(propertiesAnchor, 3) + 7 * 4;
            w->widgets[WIDX_PATH_CHECK_EDGE_S].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_S].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_SW].top = GBBT(propertiesAnchor, 3) + 7 * 3;
            w->widgets[WIDX_PATH_CHECK_EDGE_SW].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_SW].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_W].top = GBBT(propertiesAnchor, 3) + 7 * 2;
            w->widgets[WIDX_PATH_CHECK_EDGE_W].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_W].top + 13;
            w->widgets[WIDX_PATH_CHECK_EDGE_NW].top = GBBT(propertiesAnchor, 3) + 7 * 1;
            w->widgets[WIDX_PATH_CHECK_EDGE_NW].bottom = w->widgets[WIDX_PATH_CHECK_EDGE_NW].top + 13;
            widget_set_checkbox_value(w, WIDX_PATH_CHECK_SLOPED, tileElement->AsPath()->IsSloped());
            widget_set_checkbox_value(w, WIDX_PATH_CHECK_BROKEN, tileElement->AsPath()->IsBroken());
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_NE, tileElement->AsPath()->GetEdges() & (1 << ((0 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_SE, tileElement->AsPath()->GetEdges() & (1 << ((1 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_SW, tileElement->AsPath()->GetEdges() & (1 << ((2 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_NW, tileElement->AsPath()->GetEdges() & (1 << ((3 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_E, tileElement->AsPath()->GetCorners() & (1 << ((0 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_S, tileElement->AsPath()->GetCorners() & (1 << ((1 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_W, tileElement->AsPath()->GetCorners() & (1 << ((2 - get_current_rotation()) & 3)));
            widget_set_checkbox_value(
                w, WIDX_PATH_CHECK_EDGE_N, tileElement->AsPath()->GetCorners() & (1 << ((3 - get_current_rotation()) & 3)));
            break;
        case TILE_INSPECTOR_PAGE_TRACK:
            w->widgets[WIDX_TRACK_CHECK_APPLY_TO_ALL].top = GBBT(propertiesAnchor, 0);
            w->widgets[WIDX_TRACK_CHECK_APPLY_TO_ALL].bottom = GBBB(propertiesAnchor, 0);
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 1) + 3;
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 1) - 3;
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 1) + 4;
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 1) - 4;
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 1) + 4;
            w->widgets[WIDX_TRACK_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 1) - 4;
            w->widgets[WIDX_TRACK_CHECK_CHAIN_LIFT].top = GBBT(propertiesAnchor, 2);
            w->widgets[WIDX_TRACK_CHECK_CHAIN_LIFT].bottom = GBBB(propertiesAnchor, 2);
            w->widgets[WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED].top = GBBT(propertiesAnchor, 3);
            w->widgets[WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED].bottom = GBBB(propertiesAnchor, 3);
            w->widgets[WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE].top = GBBT(propertiesAnchor, 4);
            w->widgets[WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE].bottom = GBBB(propertiesAnchor, 4);
            widget_set_checkbox_value(w, WIDX_TRACK_CHECK_APPLY_TO_ALL, windowTileInspectorApplyToAll);
            widget_set_checkbox_value(w, WIDX_TRACK_CHECK_CHAIN_LIFT, tileElement->AsTrack()->HasChain());
            widget_set_checkbox_value(w, WIDX_TRACK_CHECK_BLOCK_BRAKE_CLOSED, tileElement->AsTrack()->BlockBrakeClosed());
            widget_set_checkbox_value(w, WIDX_TRACK_CHECK_IS_INDESTRUCTIBLE, tileElement->AsTrack()->IsIndestructible());
            break;
        case TILE_INSPECTOR_PAGE_SCENERY:
        {
            // Raise / Lower
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_SCENERY_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;

            // Quadrant checkboxes
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_N].top = GBBT(propertiesAnchor, 1) - 5 + 7 * 0;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_N].bottom = w->widgets[WIDX_SCENERY_CHECK_QUARTER_N].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_E].top = GBBT(propertiesAnchor, 1) - 5 + 7 * 1;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_E].bottom = w->widgets[WIDX_SCENERY_CHECK_QUARTER_E].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_S].top = GBBT(propertiesAnchor, 1) - 5 + 7 * 2;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_S].bottom = w->widgets[WIDX_SCENERY_CHECK_QUARTER_S].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_W].top = GBBT(propertiesAnchor, 1) - 5 + 7 * 1;
            w->widgets[WIDX_SCENERY_CHECK_QUARTER_W].bottom = w->widgets[WIDX_SCENERY_CHECK_QUARTER_W].top + 13;
            // This gets the relative rotation, by subtracting the camera's rotation, and wrapping it between 0-3 inclusive
            bool N = tileElement->AsSmallScenery()->GetSceneryQuadrant() == ((0 - get_current_rotation()) & 3);
            bool E = tileElement->AsSmallScenery()->GetSceneryQuadrant() == ((1 - get_current_rotation()) & 3);
            bool S = tileElement->AsSmallScenery()->GetSceneryQuadrant() == ((2 - get_current_rotation()) & 3);
            bool W = tileElement->AsSmallScenery()->GetSceneryQuadrant() == ((3 - get_current_rotation()) & 3);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_QUARTER_N, N);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_QUARTER_E, E);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_QUARTER_S, S);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_QUARTER_W, W);

            // Collision checkboxes
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_N].top = GBBT(propertiesAnchor, 2) + 5 + 7 * 0;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_N].bottom = w->widgets[WIDX_SCENERY_CHECK_COLLISION_N].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_E].top = GBBT(propertiesAnchor, 2) + 5 + 7 * 1;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_E].bottom = w->widgets[WIDX_SCENERY_CHECK_COLLISION_E].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_S].top = GBBT(propertiesAnchor, 2) + 5 + 7 * 2;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_S].bottom = w->widgets[WIDX_SCENERY_CHECK_COLLISION_S].top + 13;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_W].top = GBBT(propertiesAnchor, 2) + 5 + 7 * 1;
            w->widgets[WIDX_SCENERY_CHECK_COLLISION_W].bottom = w->widgets[WIDX_SCENERY_CHECK_COLLISION_W].top + 13;
            auto occupiedQuadrants = tileElement->GetOccupiedQuadrants();
            N = (occupiedQuadrants & (1 << ((2 - get_current_rotation()) & 3))) != 0;
            E = (occupiedQuadrants & (1 << ((3 - get_current_rotation()) & 3))) != 0;
            S = (occupiedQuadrants & (1 << ((0 - get_current_rotation()) & 3))) != 0;
            W = (occupiedQuadrants & (1 << ((1 - get_current_rotation()) & 3))) != 0;
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_COLLISION_N, N);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_COLLISION_E, E);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_COLLISION_S, S);
            widget_set_checkbox_value(w, WIDX_SCENERY_CHECK_COLLISION_W, W);
            break;
        }
        case TILE_INSPECTOR_PAGE_ENTRANCE:
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_ENTRANCE_BUTTON_MAKE_USABLE].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_ENTRANCE_BUTTON_MAKE_USABLE].bottom = GBBB(propertiesAnchor, 1);
            widget_set_enabled(
                w, WIDX_ENTRANCE_BUTTON_MAKE_USABLE,
                tileElement->AsEntrance()->GetEntranceType() != ENTRANCE_TYPE_PARK_ENTRANCE);
            break;
        case TILE_INSPECTOR_PAGE_WALL:
        {
            w->widgets[WIDX_WALL_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_WALL_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_WALL_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_WALL_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_WALL_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_WALL_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_WALL_DROPDOWN_SLOPE].top = GBBT(propertiesAnchor, 1) + 3;
            w->widgets[WIDX_WALL_DROPDOWN_SLOPE].bottom = GBBB(propertiesAnchor, 1) - 3;
            w->widgets[WIDX_WALL_DROPDOWN_SLOPE].text = WallSlopeStringIds[tileElement->AsWall()->GetSlope()];
            w->widgets[WIDX_WALL_DROPDOWN_SLOPE_BUTTON].top = GBBT(propertiesAnchor, 1) + 4;
            w->widgets[WIDX_WALL_DROPDOWN_SLOPE_BUTTON].bottom = GBBB(propertiesAnchor, 1) - 4;
            const uint8_t wallType = tileElement->AsWall()->GetEntryIndex();
            const rct_wall_scenery_entry wallEntry = get_wall_entry(wallType)->wall;
            const bool canBeSloped = !(wallEntry.flags & WALL_SCENERY_CANT_BUILD_ON_SLOPE);
            // Wall slope dropdown
            widget_set_enabled(w, WIDX_WALL_DROPDOWN_SLOPE, canBeSloped);
            widget_invalidate(w, WIDX_WALL_DROPDOWN_SLOPE);
            widget_set_enabled(w, WIDX_WALL_DROPDOWN_SLOPE_BUTTON, canBeSloped);
            widget_invalidate(w, WIDX_WALL_DROPDOWN_SLOPE_BUTTON);
            break;
        }
        case TILE_INSPECTOR_PAGE_LARGE_SCENERY:
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            break;
        case TILE_INSPECTOR_PAGE_BANNER:
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_BANNER_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_BANNER_CHECK_BLOCK_NE].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_NE].bottom = GBBB(propertiesAnchor, 1);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_SE].top = GBBT(propertiesAnchor, 2);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_SE].bottom = GBBB(propertiesAnchor, 2);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_SW].top = GBBT(propertiesAnchor, 2);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_SW].bottom = GBBB(propertiesAnchor, 2);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_NW].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_BANNER_CHECK_BLOCK_NW].bottom = GBBB(propertiesAnchor, 1);
            widget_set_checkbox_value(
                w, WIDX_BANNER_CHECK_BLOCK_NE,
                !(tileElement->AsBanner()->GetAllowedEdges() & (1 << ((0 - get_current_rotation()) & 3))));
            widget_set_checkbox_value(
                w, WIDX_BANNER_CHECK_BLOCK_SE,
                !(tileElement->AsBanner()->GetAllowedEdges() & (1 << ((1 - get_current_rotation()) & 3))));
            widget_set_checkbox_value(
                w, WIDX_BANNER_CHECK_BLOCK_SW,
                !(tileElement->AsBanner()->GetAllowedEdges() & (1 << ((2 - get_current_rotation()) & 3))));
            widget_set_checkbox_value(
                w, WIDX_BANNER_CHECK_BLOCK_NW,
                !(tileElement->AsBanner()->GetAllowedEdges() & (1 << ((3 - get_current_rotation()) & 3))));
            break;
        case TILE_INSPECTOR_PAGE_CORRUPT:
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT].top = GBBT(propertiesAnchor, 0) + 3;
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT].bottom = GBBB(propertiesAnchor, 0) - 3;
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT_INCREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE].top = GBBT(propertiesAnchor, 0) + 4;
            w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT_DECREASE].bottom = GBBB(propertiesAnchor, 0) - 4;
            w->widgets[WIDX_CORRUPT_BUTTON_CLAMP].top = GBBT(propertiesAnchor, 1);
            w->widgets[WIDX_CORRUPT_BUTTON_CLAMP].bottom = GBBB(propertiesAnchor, 1);
            break;
    }
}

static void window_tile_inspector_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    window_draw_widgets(w, dpi);

    // Set medium font size
    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;

    // Draw column headers
    rct_widget* widget;
    if ((widget = &w->widgets[WIDX_COLUMN_TYPE])->type != WWT_EMPTY)
    {
        gfx_draw_string_left_clipped(
            dpi, STR_TILE_INSPECTOR_ELEMENT_TYPE, gCommonFormatArgs, w->colours[1], w->x + widget->left + 1,
            w->y + widget->top + 1, widget->right - widget->left);
    }
    if ((widget = &w->widgets[WIDX_COLUMN_BASEHEIGHT])->type != WWT_EMPTY)
    {
        gfx_draw_string_left_clipped(
            dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_SHORT, gCommonFormatArgs, w->colours[1], w->x + widget->left + 1,
            w->y + widget->top + 1, widget->right - widget->left);
    }
    if ((widget = &w->widgets[WIDX_COLUMN_CLEARANCEHEIGHT])->type != WWT_EMPTY)
    {
        gfx_draw_string_left_clipped(
            dpi, STR_TILE_INSPECTOR_CLEARANGE_HEIGHT_SHORT, gCommonFormatArgs, w->colours[1], w->x + widget->left + 1,
            w->y + widget->top + 1, widget->right - widget->left);
    }
    if ((widget = &w->widgets[WIDX_COLUMN_GHOSTFLAG])->type != WWT_EMPTY)
    {
        gfx_draw_string_left_clipped(
            dpi, STR_TILE_INSPECTOR_FLAG_GHOST_SHORT, gCommonFormatArgs, w->colours[1], w->x + widget->left + 1,
            w->y + widget->top + 1, widget->right - widget->left);
    }
    if ((widget = &w->widgets[WIDX_COLUMN_LASTFLAG])->type != WWT_EMPTY)
    {
        gfx_draw_string_left_clipped(
            dpi, STR_TILE_INSPECTOR_FLAG_LAST_SHORT, gCommonFormatArgs, w->colours[1], w->x + widget->left + 1,
            w->y + widget->top + 1, widget->right - widget->left);
    }

    // Draw coordinates
    gfx_draw_string(dpi, (char*)"X:", COLOUR_WHITE, w->x + 5, w->y + 24);
    gfx_draw_string(dpi, (char*)"Y:", COLOUR_WHITE, w->x + 74, w->y + 24);
    if (windowTileInspectorTileSelected)
    {
        auto tileCoords = TileCoordsXY{ windowTileInspectorToolMap };
        gfx_draw_string_right(dpi, STR_FORMAT_INTEGER, &tileCoords.x, COLOUR_WHITE, w->x + 43, w->y + 24);
        gfx_draw_string_right(dpi, STR_FORMAT_INTEGER, &tileCoords.y, COLOUR_WHITE, w->x + 113, w->y + 24);
    }
    else
    {
        gfx_draw_string(dpi, (char*)"-", COLOUR_WHITE, w->x + 43 - 7, w->y + 24);
        gfx_draw_string(dpi, (char*)"-", COLOUR_WHITE, w->x + 113 - 7, w->y + 24);
    }

    if (windowTileInspectorSelectedIndex != -1)
    {
        // X and Y of first element in detail box
        int32_t x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
        int32_t y = w->y + w->widgets[WIDX_GROUPBOX_DETAILS].top + 14;

        // Get map element
        TileElement* const tileElement = window_tile_inspector_get_selected_element(w);

        switch (w->page)
        {
            case TILE_INSPECTOR_PAGE_SURFACE:
            {
                // Details
                // Terrain texture name
                rct_string_id terrainNameId = TerrainTypeStringIds[tileElement->AsSurface()->GetSurfaceStyle()];
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SURFACE_TERAIN, &terrainNameId, COLOUR_WHITE, x, y);

                // Edge texture name
                uint32_t idx = tileElement->AsSurface()->GetEdgeStyle();
                openrct2_assert(
                    (uint32_t)idx < std::size(TerrainEdgeTypeStringIds),
                    "Tried accessing invalid entry %d in terrainEdgeTypeStringIds", idx);
                rct_string_id terrainEdgeNameId = TerrainEdgeTypeStringIds[tileElement->AsSurface()->GetEdgeStyle()];
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SURFACE_EDGE, &terrainEdgeNameId, COLOUR_WHITE, x, y + 11);

                // Land ownership
                rct_string_id landOwnership;
                if (tileElement->AsSurface()->GetOwnership() & OWNERSHIP_OWNED)
                    landOwnership = STR_LAND_OWNED;
                else if (tileElement->AsSurface()->GetOwnership() & OWNERSHIP_AVAILABLE)
                    landOwnership = STR_LAND_SALE;
                else if (tileElement->AsSurface()->GetOwnership() & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED)
                    landOwnership = STR_CONSTRUCTION_RIGHTS_OWNED;
                else if (tileElement->AsSurface()->GetOwnership() & OWNERSHIP_CONSTRUCTION_RIGHTS_AVAILABLE)
                    landOwnership = STR_CONSTRUCTION_RIGHTS_SALE;
                else
                    landOwnership = STR_TILE_INSPECTOR_LAND_NOT_OWNED_AND_NOT_AVAILABLE;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SURFACE_OWNERSHIP, &landOwnership, COLOUR_WHITE, x, y + 22);

                // Water level
                uint32_t waterLevel = tileElement->AsSurface()->GetWaterHeight();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SURFACE_WATER_LEVEL, &waterLevel, COLOUR_WHITE, x, y + 33);

                // Properties
                // Raise / lower label
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_SURFACE_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_SURFACE_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);

                // Raised corners
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_SURFACE_CHECK_CORNER_E].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SURFACE_CORNERS, nullptr, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_PATH:
            {
                // Details
                // Path name
                rct_string_id pathNameId = tileElement->AsPath()->GetPathEntry()->string_idx;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_PATH_NAME, &pathNameId, COLOUR_WHITE, x, y);

                // Path addition
                if (tileElement->AsPath()->HasAddition())
                {
                    const uint8_t pathAdditionType = tileElement->AsPath()->GetAdditionEntryIndex();
                    const auto* sceneryElement = get_footpath_item_entry(pathAdditionType);
                    rct_string_id additionNameId = sceneryElement != nullptr ? sceneryElement->name
                                                                             : (rct_string_id)STR_UNKNOWN_OBJECT_TYPE;
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_PATH_ADDITIONS, &additionNameId, COLOUR_WHITE, x, y + 11);
                }
                else
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_PATH_ADDITIONS_NONE, nullptr, COLOUR_WHITE, x, y + 11);

                // Properties
                // Raise / lower label
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_PATH_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_PATH_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);

                // Path connections
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_PATH_CHECK_EDGE_W].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_PATH_CONNECTED_EDGES, nullptr, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_TRACK:
            {
                // Details
                // Ride
                auto trackElement = tileElement->AsTrack();
                int16_t rideId = trackElement->GetRideIndex();
                auto ride = get_ride(rideId);
                if (ride != nullptr)
                {
                    auto rideType = RideNaming[ride->type].name;
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_TRACK_RIDE_TYPE, &rideType, COLOUR_WHITE, x, y);
                }
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_TRACK_RIDE_ID, &rideId, COLOUR_WHITE, x, y + 11);
                if (ride != nullptr)
                {
                    ride->FormatNameTo(gCommonFormatArgs);
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_TRACK_RIDE_NAME, gCommonFormatArgs, COLOUR_WHITE, x, y + 22);
                }
                // Track
                int16_t trackType = trackElement->GetTrackType();
                int16_t sequenceNumber = trackElement->GetSequenceIndex();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_TRACK_PIECE_ID, &trackType, COLOUR_WHITE, x, y + 33);
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_TRACK_SEQUENCE, &sequenceNumber, COLOUR_WHITE, x, y + 44);
                if (track_element_is_station(tileElement))
                {
                    int16_t stationIndex = trackElement->GetStationIndex();
                    set_format_arg(0, rct_string_id, STR_COMMA16);
                    set_format_arg(2, int16_t, stationIndex);
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_STATION_INDEX, gCommonFormatArgs, COLOUR_WHITE, x, y + 55);
                }
                else
                {
                    const char* stationNone = "-";
                    set_format_arg(0, rct_string_id, STR_STRING);
                    set_format_arg(2, char*, stationNone);
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_STATION_INDEX, gCommonFormatArgs, COLOUR_WHITE, x, y + 55);
                }

                rct_string_id colourScheme = ColourSchemeNames[trackElement->GetColourScheme()];
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_COLOUR_SCHEME, &colourScheme, COLOUR_WHITE, x, y + 66);

                // Properties
                // Raise / lower label
                y = w->y + w->widgets[WIDX_TRACK_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_TRACK_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_SCENERY:
            {
                // Details
                // Age
                int16_t age = tileElement->AsSmallScenery()->GetAge();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SCENERY_AGE, &age, COLOUR_WHITE, x, y);

                // Quadrant value
                const rct_scenery_entry* sceneryEntry = get_small_scenery_entry(tileElement->AsSmallScenery()->GetEntryIndex());
                if (!(scenery_small_entry_has_flag(sceneryEntry, SMALL_SCENERY_FLAG_FULL_TILE)))
                {
                    int16_t quadrant = tileElement->AsSmallScenery()->GetSceneryQuadrant();
                    static rct_string_id quadrant_string_idx[] = { STR_TILE_INSPECTOR_SCENERY_QUADRANT_SW,
                                                                   STR_TILE_INSPECTOR_SCENERY_QUADRANT_NW,
                                                                   STR_TILE_INSPECTOR_SCENERY_QUADRANT_NE,
                                                                   STR_TILE_INSPECTOR_SCENERY_QUADRANT_SE };
                    gfx_draw_string_left(
                        dpi, STR_TILE_INSPECTOR_SCENERY_QUADRANT, &quadrant_string_idx[quadrant], COLOUR_WHITE, x, y + 11);
                }

                // Scenery ID
                int16_t idx = tileElement->AsSmallScenery()->GetEntryIndex();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SCENERY_ENTRY_IDX, &idx, COLOUR_WHITE, x, y + 22);

                // Properties
                // Raise / Lower
                y = w->y + w->widgets[WIDX_SCENERY_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_SCENERY_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);

                // Quarter tile
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_SCENERY_CHECK_QUARTER_E].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_SCENERY_QUADRANT_LABEL, nullptr, COLOUR_WHITE, x, y);

                // Collision
                y = w->y + w->widgets[WIDX_SCENERY_CHECK_COLLISION_E].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_COLLISSION, nullptr, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_ENTRANCE:
            {
                // Details
                // Entrance type
                rct_string_id entranceType = EntranceTypeStringIds[tileElement->AsEntrance()->GetEntranceType()];
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRANCE_TYPE, &entranceType, COLOUR_WHITE, x, y);

                if (tileElement->AsEntrance()->GetEntranceType() == ENTRANCE_TYPE_PARK_ENTRANCE)
                {
                    // TODO: Make this work with Left/Right park entrance parts
                    int16_t parkEntranceIndex = park_entrance_get_index(
                        windowTileInspectorToolMap.x, windowTileInspectorToolMap.y, tileElement->base_height * 8);
                    gfx_draw_string_left(
                        dpi, STR_TILE_INSPECTOR_ENTRANCE_ENTRANCE_ID, &parkEntranceIndex, COLOUR_WHITE, x, y + 11);
                }
                else
                {
                    int16_t rideEntranceIndex = tileElement->AsEntrance()->GetStationIndex();
                    if (tileElement->AsEntrance()->GetEntranceType() == ENTRANCE_TYPE_RIDE_ENTRANCE)
                    {
                        // Ride entrance ID
                        gfx_draw_string_left(
                            dpi, STR_TILE_INSPECTOR_ENTRANCE_ENTRANCE_ID, &rideEntranceIndex, COLOUR_WHITE, x, y + 11);
                    }
                    else
                    {
                        // Ride exit ID
                        gfx_draw_string_left(
                            dpi, STR_TILE_INSPECTOR_ENTRANCE_EXIT_ID, &rideEntranceIndex, COLOUR_WHITE, x, y + 11);
                    }
                }

                if (tileElement->AsEntrance()->GetEntranceType() == ENTRANCE_TYPE_PARK_ENTRANCE)
                {
                    // Entrance part
                    rct_string_id entrancePart = ParkEntrancePartStringIds[tileElement->AsEntrance()->GetSequenceIndex()];
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRANCE_PART, &entrancePart, COLOUR_WHITE, x, y + 22);
                }
                else
                {
                    // Ride ID
                    int16_t rideId = tileElement->AsEntrance()->GetRideIndex();
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRANCE_RIDE_ID, &rideId, COLOUR_WHITE, x, y + 22);
                    // Station index
                    int16_t stationIndex = tileElement->AsEntrance()->GetStationIndex();
                    set_format_arg(0, rct_string_id, STR_COMMA16);
                    set_format_arg(2, int16_t, stationIndex);
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_STATION_INDEX, gCommonFormatArgs, COLOUR_WHITE, x, y + 33);
                }

                // Properties
                // Raise / Lower
                y = w->y + w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_ENTRANCE_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_WALL:
            {
                // Details
                // Type
                int16_t wallType = tileElement->AsWall()->GetEntryIndex();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_WALL_TYPE, &wallType, COLOUR_WHITE, x, y);

                // Banner info
                rct_wall_scenery_entry wallEntry = get_wall_entry(wallType)->wall;
                if (wallEntry.flags & WALL_SCENERY_IS_BANNER)
                {
                    auto banner = tileElement->AsWall()->GetBanner();
                    if (banner != nullptr && !banner->IsNull())
                    {
                        uint8_t args[32]{};
                        banner->FormatTextTo(args);
                        gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRY_BANNER_TEXT, args, COLOUR_WHITE, x, y + 11);
                    }
                }
                else
                {
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRY_BANNER_NONE, nullptr, COLOUR_WHITE, x, y + 11);
                }

                // Properties
                // Raise / lower label
                y = w->y + w->widgets[WIDX_WALL_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_WALL_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);

                // Slope label
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                y = w->y + w->widgets[WIDX_WALL_DROPDOWN_SLOPE].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_WALL_SLOPE, nullptr, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_LARGE_SCENERY:
            {
                // Details
                // Type
                auto sceneryElement = tileElement->AsLargeScenery();
                int16_t largeSceneryType = sceneryElement->GetEntryIndex();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_LARGE_SCENERY_TYPE, &largeSceneryType, COLOUR_WHITE, x, y);

                // Part ID
                int16_t pieceID = sceneryElement->GetSequenceIndex();
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_LARGE_SCENERY_PIECE_ID, &pieceID, COLOUR_WHITE, x, y + 11);

                // Banner info
                rct_scenery_entry* largeSceneryEntry = get_large_scenery_entry(largeSceneryType);
                if (largeSceneryEntry->large_scenery.scrolling_mode != SCROLLING_MODE_NONE)
                {
                    auto banner = sceneryElement->GetBanner();
                    if (banner != nullptr && !banner->IsNull())
                    {
                        uint8_t args[32]{};
                        banner->FormatTextTo(args);
                        gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRY_BANNER_TEXT, args, COLOUR_WHITE, x, y + 22);
                    }
                }
                else
                {
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRY_BANNER_NONE, nullptr, COLOUR_WHITE, x, y + 22);
                }

                // Properties
                // Raise / lower label
                y = w->y + w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_LARGE_SCENERY_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_BANNER:
            {
                // Details
                // Banner info
                auto banner = tileElement->AsBanner()->GetBanner();
                if (banner != nullptr && !banner->IsNull())
                {
                    uint8_t args[32]{};
                    banner->FormatTextTo(args);
                    gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_ENTRY_BANNER_TEXT, args, COLOUR_WHITE, x, y + 22);
                }

                // Properties
                // Raise / lower label
                y = w->y + w->widgets[WIDX_BANNER_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_BANNER_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);

                // Blocked paths
                y += 28;
                x = w->x + w->widgets[WIDX_GROUPBOX_DETAILS].left + 7;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BANNER_BLOCKED_PATHS, nullptr, COLOUR_WHITE, x, y);
                break;
            }

            case TILE_INSPECTOR_PAGE_CORRUPT:
            {
                // Properties
                // Raise / lower label
                y = w->y + w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT].top;
                gfx_draw_string_left(dpi, STR_TILE_INSPECTOR_BASE_HEIGHT_FULL, nullptr, COLOUR_WHITE, x, y);

                // Current base height
                x = w->x + w->widgets[WIDX_CORRUPT_SPINNER_HEIGHT].left + 3;
                int32_t baseHeight = tileElement->base_height;
                gfx_draw_string_left(dpi, STR_FORMAT_INTEGER, &baseHeight, COLOUR_WHITE, x, y);
                break;
            }
        } // switch page
    }
}

static void window_tile_inspector_scrollpaint(rct_window* w, rct_drawpixelinfo* dpi, int32_t scrollIndex)
{
    const int32_t listWidth = w->widgets[WIDX_LIST].right - w->widgets[WIDX_LIST].left;
    gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ColourMapA[w->colours[1]].mid_light);

    int32_t y = SCROLLABLE_ROW_HEIGHT * (windowTileInspectorElementCount - 1);
    int32_t i = 0;
    char buffer[256];

    if (!windowTileInspectorTileSelected)
        return;

    const TileElement* tileElement = map_get_first_element_at(
        windowTileInspectorToolMap.x / 32, windowTileInspectorToolMap.y / 32);

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;
    do
    {
        if (tileElement == nullptr)
            break;
        const bool selectedRow = i == windowTileInspectorSelectedIndex;
        const bool hoveredRow = i == windowTileInspectorHighlightedIndex;
        int32_t type = tileElement->GetType();
        const char* typeName = "";

        if (selectedRow)
        {
            gfx_fill_rect(dpi, 0, y, listWidth, y + SCROLLABLE_ROW_HEIGHT - 1, ColourMapA[w->colours[1]].mid_dark);
        }
        else if (hoveredRow)
        {
            gfx_fill_rect(dpi, 0, y, listWidth, y + SCROLLABLE_ROW_HEIGHT - 1, ColourMapA[w->colours[1]].mid_dark | 0x1000000);
        }
        else if (((windowTileInspectorElementCount - i) & 1) == 0)
        {
            // Zebra stripes
            gfx_fill_rect(dpi, 0, y, listWidth, y + SCROLLABLE_ROW_HEIGHT - 1, ColourMapA[w->colours[1]].light | 0x1000000);
        }

        switch (type)
        {
            case TILE_ELEMENT_TYPE_SURFACE:
                typeName = language_get_string(STR_TILE_INSPECTOR_SURFACE);
                break;
            case TILE_ELEMENT_TYPE_PATH:
                typeName = tileElement->AsPath()->IsQueue() ? language_get_string(STR_QUEUE_LINE_MAP_TIP)
                                                            : language_get_string(STR_FOOTPATH_MAP_TIP);
                break;
            case TILE_ELEMENT_TYPE_TRACK:
                typeName = language_get_string(STR_RIDE_COMPONENT_TRACK_CAPITALISED);
                break;
            case TILE_ELEMENT_TYPE_SMALL_SCENERY:
                snprintf(
                    buffer, sizeof(buffer), "%s (%s)", language_get_string(STR_OBJECT_SELECTION_SMALL_SCENERY),
                    language_get_string(get_small_scenery_entry(tileElement->AsSmallScenery()->GetEntryIndex())->name));
                typeName = buffer;
                break;
            case TILE_ELEMENT_TYPE_ENTRANCE:
                typeName = language_get_string(STR_RIDE_CONSTRUCTION_ENTRANCE);
                break;
            case TILE_ELEMENT_TYPE_WALL:
                snprintf(
                    buffer, sizeof(buffer), "%s (%s)", language_get_string(STR_TILE_INSPECTOR_WALL),
                    language_get_string(tileElement->AsWall()->GetEntry()->name));
                typeName = buffer;
                break;
            case TILE_ELEMENT_TYPE_LARGE_SCENERY:
                typeName = language_get_string(STR_OBJECT_SELECTION_LARGE_SCENERY);
                break;
            case TILE_ELEMENT_TYPE_BANNER:
                snprintf(
                    buffer, sizeof(buffer), "%s (%d)", language_get_string(STR_BANNER_WINDOW_TITLE),
                    tileElement->AsBanner()->GetIndex());
                typeName = buffer;
                break;
            case TILE_ELEMENT_TYPE_CORRUPT:
                // fall-through
            default:
                snprintf(buffer, sizeof(buffer), "%s (%d)", language_get_string(STR_UNKNOWN_OBJECT_TYPE), type);
                typeName = buffer;
        }

        const int32_t baseHeight = tileElement->base_height;
        const int32_t clearanceHeight = tileElement->clearance_height;
        const bool ghost = tileElement->IsGhost();
        const bool last = tileElement->IsLastForTile();

        const rct_string_id stringFormat = (selectedRow || hoveredRow) ? STR_WHITE_STRING : STR_WINDOW_COLOUR_2_STRINGID;

        // Undo relative scroll offset, but keep the 3 pixel padding
        const int32_t x = -w->widgets[WIDX_LIST].left;
        set_format_arg(0, rct_string_id, STR_STRING);
        set_format_arg(2, char*, typeName);
        gfx_draw_string_left_clipped(
            dpi, stringFormat, gCommonFormatArgs, COLOUR_BLACK, x + COL_X_TYPE + 3, y, COL_X_BH); // 3px padding

        // Base height
        set_format_arg(0, rct_string_id, STR_FORMAT_INTEGER);
        set_format_arg(2, int32_t, baseHeight);
        gfx_draw_string_left(dpi, stringFormat, gCommonFormatArgs, COLOUR_BLACK, x + COL_X_BH, y);

        // Clearance height
        set_format_arg(0, rct_string_id, STR_FORMAT_INTEGER);
        set_format_arg(2, int32_t, clearanceHeight);
        gfx_draw_string_left(dpi, stringFormat, gCommonFormatArgs, COLOUR_BLACK, x + COL_X_CH, y);

        // Checkmarks for ghost and last for tile
        set_format_arg(0, rct_string_id, STR_STRING);
        set_format_arg(2, char*, CheckBoxMarkString);
        if (ghost)
        {
            gfx_draw_string_left(dpi, stringFormat, gCommonFormatArgs, COLOUR_BLACK, x + COL_X_GF, y);
        }
        if (last)
        {
            gfx_draw_string_left(dpi, stringFormat, gCommonFormatArgs, COLOUR_BLACK, x + COL_X_LF, y);
        }

        y -= SCROLLABLE_ROW_HEIGHT;
        i++;
    } while (!(tileElement++)->IsLastForTile());
}
